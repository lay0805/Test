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
