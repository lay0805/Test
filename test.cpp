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

const _KeepOffsetFromActor KeepOffsetFromActor = (_KeepOffsetFromActor)0x8DA850;
const _GetCurrentStageID GetCurrentStageID = (_GetCurrentStageID)0x009152D0;
const _GetOpenState GetOpenState = (_GetOpenState)0x009027A0;
const _SetInvulnerable SetInvulnerable = (_SetInvulnerable)0x008FFC20;
const _Notification Notification = (_Notification)0x008EE550;
const _ToggleAI ToggleAI = (_ToggleAI)0x008EEF10;
const _MoveTo MoveTo = (_MoveTo)0x00908B60;
const _EnableAI EnableAI = (_EnableAI)0x008DAEE0;
const _SetAlpha SetAlpha = (_SetAlpha)0x008DBB80;
const _AllowPCDialogue AllowPCDialogue = (_AllowPCDialogue)0x008DB3C0;
const _IsDisabled IsDisabled = (_IsDisabled)0x00902DC0;
const _ClearKeepOffsetFromActor ClearKeepOffsetFromActor = (_ClearKeepOffsetFromActor)0x008DAE90;
const _IgnoreFriendlyHits IgnoreFriendlyHits = (_IgnoreFriendlyHits)0x00902930;
const _SetRelationshipRank SetRelationshipRank = (_SetRelationshipRank)0x008DC7F0;
const _SetHeadTracking SetHeadTracking = (_SetHeadTracking)0x008DACB0;
const _GetCurrentLocation GetCurrentLocation = (_GetCurrentLocation)0x00902D20;
const _GetWorldSpace GetWorldSpace = (_GetWorldSpace)0x00902D90;
const _GetDistance GetDistance = (_GetDistance)0x009026E0;
const _IsRunning IsRunning = (_IsRunning)0x008DB070;
const _IsSprinting IsSprinting = (_IsSprinting)0x008DB080;
const _ForceActorValue ForceActorValue = (_ForceActorValue)0x008DD5A0;
const _GetEquippedSpell GetEquippedSpell = (_GetEquippedSpell)0x008DB500;
const _Cast Cast = (_Cast)0x008F90B0;
const _GetEquippedItemType GetEquippedItemType = (_GetEquippedItemType)0x008DB550;
const _DisableNoWait DisableNoWait = (_DisableNoWait)0x009087B0;
const _SetValue SetValue = (_SetValue)0x008FE2B0;
const _SetINIBool SetINIBool = (_SetINIBool)0x00919460;
const _KillSilent KillSilent = (_KillSilent)0x008DA9E0;
const _Resurrect Resurrect = (_Resurrect)0x008DD990;
const _ForceActive ForceActive = (_ForceActive)0x00917FE0;
const _IsAllowedToFly IsAllowedToFly = (_IsAllowedToFly)0x008DA800;
const _IsDead IsDead = (_IsDead)0x008DA830;
const _IsAlarmed IsAlarmed = (_IsAlarmed)0x008DA7D0;
const _SendAssaultAlarm SendAssaultAlarm = (_SendAssaultAlarm)0x008DB9E0;
const _SetAlert SetAlert = (_SetAlert)0x008DB0D0;
const _IsAlerted IsAlerted = (_IsAlerted)0x008DAF90;
const _IsUnconscious IsUnconscious = (_IsUnconscious)0x008DC040;
const _SetUnconscious SetUnconscious = (_SetUnconscious)0x008DBD90;
const _IsSneaking IsSneaking = (_IsSneaking)0x008DDFD0;
const _GetActorValue_Native GetActorValue_Native = (_GetActorValue_Native)0x008DB430;
const _IsInCombat IsInCombat = (_IsInCombat)0x008DB020;
const _StartCombat StartCombat = (_StartCombat)0x008DDC20;
const _StopCombat StopCombat = (_StopCombat)0x008DBF40;
const _Is3DLoaded Is3DLoaded = (_Is3DLoaded)0x00902950;
const _SetMotionType SetMotionType = (_SetMotionType)0x00909C90;
const _EnableNoWait EnableNoWait = (_EnableNoWait)0x00908AE0;
const _GetValue GetValue = (_GetValue)0x008FE2A0;
const _GetCurrentWeather GetCurrentWeather = (_GetCurrentWeather)0x00918090;
const _IsWeaponDrawn IsWeaponDrawn = (_IsWeaponDrawn)0x008DC060;
const _FindClosestReferenceOfType FindClosestReferenceOfType = (_FindClosestReferenceOfType)0x008F4530;
const _IsBleedingOut IsBleedingOut = (_IsBleedingOut)0x008DA810;
const _IsBribed IsBribed = (_IsBribed)0x008DAFB0;
const _IsPlayerTeammate IsPlayerTeammate = (_IsPlayerTeammate)0x008DC030;
const _GetEquippedShout GetEquippedShout = (_GetEquippedShout)0x008DAF10;
const _GetVoiceRecoveryTime GetVoiceRecoveryTime = (_GetVoiceRecoveryTime)0x008DAF80;
const _IsInMenuMode IsInMenuMode = (_IsInMenuMode)0x00918D90;
const _GetActorValuePercentage GetActorValuePercentage = (_GetActorValuePercentage)0x008DA6C0;
const _GetCombatTarget GetCombatTarget = (_GetCombatTarget)0x008E1010;
const _GetArrestedState GetArrestedState = (_GetArrestedState)0x00452300;
const _SetAngle SetAngle = (_SetAngle)0x00909880;
const _SetPosition SetPosition = (_SetPosition)0x00909E40;
const _SendAnimationEvent SendAnimationEvent_Native = (_SendAnimationEvent)0x008EE630;
const _AddToMap AddToMap = (_AddToMap)0x009024E0;
const _SetLookAt SetLookAt = (_SetLookAt)0x008DBC10;
const _GetEquippedWeapon GetEquippedWeapon = (_GetEquippedWeapon)0x008DAF20;
const _EquipSpell EquipSpell = (_EquipSpell)0x008DD4E0;
const _Fire Fire = (_Fire)0x009149E0;
const _Lock Lock = (_Lock)0x00902A00;
const _SetOpen SetOpen = (_SetOpen)0x009039B0;
const _GetEquippedShield GetEquippedShield = (_GetEquippedShield)0x008DAF30;
const _GetAngleX GetAngleX = (_GetAngleX)0x00903330;
const _GetAngleY GetAngleY = (_GetAngleY)0x00903350;
const _GetAngleZ GetAngleZ = (_GetAngleZ)0x00903370;
const _IsLocked IsLocked_Native = (_IsLocked)0x009036F0;
const _Activate Activate = (_Activate)0x00902460;
const _IsBeingRidden IsBeingRidden = (_IsBeingRidden)0x008DCAF0;
const _GetSitState GetSitState = (_GetSitState)0x008DA8E0;
const _GetPositionX GetPositionX = (_GetPositionX)0x00903670;
const _GetPositionY GetPositionY = (_GetPositionY)0x00903680;
const _GetPositionZ GetPositionZ = (_GetPositionZ)0x00903690;
const _IsOnMount IsOnMount = (_IsOnMount)0x008DCB40;
const _SetAnimationVariableBool SetAnimationVariableBool = (_SetAnimationVariableBool)0x00902AE0;
const _SetAnimationVariableInt SetAnimationVariableInt = (_SetAnimationVariableInt)0x00902B70;
const _SetAnimationVariableFloat SetAnimationVariableFloat = (_SetAnimationVariableFloat)0x00902C00;
const _GetAnimationVariableFloat GetAnimationVariableFloat_Native = (_GetAnimationVariableFloat)0x009034D0;
const _GetAnimationVariableBool GetAnimationVariableBool_Native = (_GetAnimationVariableBool)0x00903390;
const _SetDontMove SetDontMove = (_SetDontMove)0x008DAD20;
const _IsChildLocation IsChildLocation = (_IsChildLocation)0x008E9FC0;
const _IsInterior IsInterior = (_IsInterior)0x00900F90;
const _RemoveItem RemoveItem = (_RemoveItem)0x00909650;
const _UnequipSpell UnequipSpell = (_UnequipSpell)0x008DD540;
const _UnequipItemSlot UnequipItemSlot = (_UnequipItemSlot)0x008DAD70;
const _TranslateTo TranslateTo = (_TranslateTo)0x00911F10;
const _RemoveAllItems RemoveAllItems = (_RemoveAllItems)0x00903900;
const _StartQuest StartQuest = (_StartQuest)0x00915E00;
const _SetCurrentStageID SetCurrentStageID = (_SetCurrentStageID)0x00915CC0;
const _SetObjectiveDisplayed SetObjectiveDisplayed = (_SetObjectiveDisplayed)0x009156A0;
const _SetObjectiveCompleted SetObjectiveCompleted = (_SetObjectiveCompleted)0x00915590;
