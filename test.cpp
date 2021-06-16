class ReadyWeaponHandlerEx : public ReadyWeaponHandler
{
public:
	typedef void(ReadyWeaponHandlerEx::*FnProcessButton)(ButtonEvent* evn, PlayerControls::Data14 *data);

	static FnProcessButton fnProcessButtonOld;

	void ProcessButton_Hook(ButtonEvent* evn, PlayerControls::Data14 *data)
	{
		LootMenu *lootMenu = LootMenu::GetSingleton();

		static bool bProcessLongTap = false;

		if (!evn->IsDown())
		{
			if (bProcessLongTap && evn->timer > 2.0f)
			{
				bProcessLongTap = false;

				if (lootMenu && !lootMenu->IsVisible())
				{
					bool disabled = !lootMenu->IsDisabled();
					lootMenu->SetDisabled(disabled);
					
					typedef void(*FnDebug_Notification)(const char *, bool, bool);
					const FnDebug_Notification fnDebug_Notification = (FnDebug_Notification)0x008997A0;

					fnDebug_Notification((disabled ? "QuickLoot has been disabled." : "QuickLoot has been enabled."), false, true);
				}
			}

			return;
		}

		bProcessLongTap = true;

		if (lootMenu && lootMenu->IsVisible())
		{
			g_thePlayer->StartActivation();
		}
		else
		{
			(this->*fnProcessButtonOld)(evn, data);
		}
	}

	static void InitHook()
	{
		fnProcessButtonOld = SafeWrite32(0x010D45C4 + 0x04 * 4, &ProcessButton_Hook);
	}
};

ReadyWeaponHandlerEx::FnProcessButton ReadyWeaponHandlerEx::fnProcessButtonOld;

class FavoritesMenu : public IMenu,
					  public MenuEventHandler
{
public:

	struct ItemData
	{
		TESForm*				form;
		InventoryEntryData*	    objDesc;
	};

	struct EquipManager
	{
		static EquipManager * GetSingleton(void)
		{
			return *((EquipManager **)0x012E5FAC);
		}
		DEFINE_MEMBER_FN(EquipSpell, void, 0x006EFAC0, Actor * actor, TESForm * item, BGSEquipSlot* slot);
		DEFINE_MEMBER_FN(EquipShout, void, 0x006F0110, Actor * actor, TESForm * item);
		DEFINE_MEMBER_FN(EquipItem, void, 0x006EF820, Actor * actor, InventoryEntryData* unk0, BGSEquipSlot* slot, bool unk1);
	};

	void EquipItem(BGSEquipSlot* slot)
	{
		GFxMovieView* view = this->GetMovieView();
		GFxValue result;
		view->Invoke("_root.GetSelectedIndex", &result, nullptr, 0);

		if (result.GetType() == GFxValue::ValueType::VT_Number && result.GetNumber() != -1.00f)
		{
			if (this->unk40 && itemDatas != nullptr)
			{
				UInt32 index = static_cast<UInt32>(result.GetNumber());
				TESForm* form = itemDatas[index].form;
				if (form != nullptr)
				{
					if (form->formType == FormType::Spell)
					{
						EquipManager::GetSingleton()->EquipSpell(g_thePlayer, form, slot);
					}
					else if (form->formType == FormType::Shout)
					{
						if (!g_thePlayer->IsEquipShoutDisabled())
						{
							EquipManager::GetSingleton()->EquipShout(g_thePlayer, form);
						}
					}
					else
					{
						InventoryEntryData* objDesc = itemDatas[index].objDesc;
						EquipManager::GetSingleton()->EquipItem(g_thePlayer, objDesc, slot, true);
						if (g_thePlayer->processManager != nullptr)
						{
							auto fn = [form]() 
							{
								g_thePlayer->processManager->UpdateEquipment(g_thePlayer);

								if (form->formType == FormType::Armor)
								{
									UInt32 slots = g_thePlayer->GetArmorSlots();//injuredHealthPct
									TESObjectARMO* armor = ((TESObjectARMO* (__cdecl*)(TESForm*))0x447CA0)(form);
									//_MESSAGE("%s", GetObjectClassName(armor));
									bool(__fastcall* IsSlotsSuitable)(TESObjectARMO*, void*, UInt32) = (bool(__fastcall*)(TESObjectARMO*, void*, UInt32))0x00447C20;
									if (IsSlotsSuitable(armor, nullptr, slots))
									{
										g_thePlayer->UpdateSlots();
									}
								}
							};
							CallbackDelegate::Register<CallbackDelegate::kType_Normal>(fn);
						}
					}
					((void(__cdecl*)(char*))0x00899620)("UIFavorite");
					this->unk45 = true;
					((void(__cdecl*)(Actor*, bool))0x00897F90)(g_thePlayer, false);
				}
			}
		}
		if ((result.GetType() >> 6) & 1)
		{
			result.pObjectInterface->ObjectRelease(&result, result.Value.pData);
		}
	}

	static void InitHook()
	{
		SafeWrite8(0x0085BE1D, 0x00);
		WriteRelJump(0x0085B5E0, GetFnAddr(&FavoritesMenu::EquipItem));
	}

	UInt32						unk28;
	UInt32						unk2C;
	UInt32						unk30;
	UInt32						unk34;
	ItemData*					itemDatas;
	UInt32						unk3C;
	UInt32						unk40;
	bool						unk44;
	bool						unk45;
	//
	// ....
};
static_assert(sizeof(FavoritesMenu) == 0x48, "sizeof(FavoritesMenu) != 0x48");

