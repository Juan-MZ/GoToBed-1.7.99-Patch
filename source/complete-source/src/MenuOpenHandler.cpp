#include "MenuOpenHandler.h"
#include "Offsets.h"
#include "UIFuncs.h"
#include "Settings.h"

namespace Gotobed
{
	namespace Hooks
	{
		stl::HookData ProcessButton{&MenuOpenHandler::ProcessButtonHook};
		stl::HookData CanProcess{&MenuOpenHandler::CanProcessHook};
	}

	namespace detail
	{
		bool IsKeyPressed(std::uint32_t a_key) {
			// CommonLibSSE-NG declares BSWin32KeyboardDevice's virtual overrides without
			// defining them - they are the game's own vtable slots, and the class is only ever
			// used through a pointer the game hands out. Calling its IsPressed() drags that
			// translation unit into the link, and MSVC then has to emit the whole vtable,
			// which cannot resolve. This is exactly the check IsPressed() performs.
			const auto& keys = RE::BSInputDeviceManager::GetSingleton()->GetKeyboard()->GetRuntimeData().curState;
			return a_key < sizeof(keys) && (keys[a_key] & 0x80) != 0;
		}

		bool IsSleepKey(RE::ButtonEvent* a_event) {
			auto& settings = Settings::Get();
			return a_event && (settings.keys.sleep != -1 ? a_event->idCode == settings.keys.sleep : a_event->userEvent == "Wait");
		}

		bool IsServeTimeKey(RE::ButtonEvent* a_event) {
			auto& settings = Settings::Get();
			return a_event && (settings.keys.serveTime != -1 ? a_event->idCode == settings.keys.serveTime : a_event->userEvent == "Wait");
		}
	}

	bool MenuOpenHandler::CanProcessHook(RE::InputEvent* a_event) {
		if (a_event && a_event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
			if (detail::IsSleepKey(a_event->AsButtonEvent()) || detail::IsServeTimeKey(a_event->AsButtonEvent())) {
				return true;
			}
		}

		return Hooks::CanProcess.call_orig(this, a_event);
	}

	bool MenuOpenHandler::ProcessButtonHook(RE::ButtonEvent* a_event) {
		auto& settings = Settings::Get();

		if (a_event && a_event->IsDown()) {
			if (detail::IsServeTimeKey(a_event) && (settings.keys.serveTimeMod == -1 || detail::IsKeyPressed(settings.keys.serveTimeMod)) && OnServeTimeButtonDown()) {
				return true;
			}

			if (detail::IsSleepKey(a_event) && (settings.keys.sleepMod == -1 || detail::IsKeyPressed(settings.keys.sleepMod)) && OnSleepButtonDown()) {
				return true;
			}
		}

		return Hooks::ProcessButton.call_orig(this, a_event);
	}

	bool MenuOpenHandler::OnSleepButtonDown() {
		auto ui = RE::UI::GetSingleton();

		if (ui->numPausesGame || ui->IsMenuOpen(RE::FaderMenu::MENU_NAME)) {
			return false;
		}

		auto player = RE::PlayerCharacter::GetSingleton();

		if (player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDying || player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDead) {
			return false;
		}

		if (player->actorState1.sitSleepState != RE::SIT_SLEEP_STATE::kIsSleeping) {
			return false;
		}

		UIFuncs::ShowSleepWaitMenu(true);

		return true;
	}

	bool MenuOpenHandler::OnServeTimeButtonDown() {
		auto ui = RE::UI::GetSingleton();

		if (ui->numPausesGame || ui->IsMenuOpen(RE::FaderMenu::MENU_NAME)) {
			return false;
		}

		auto player = RE::PlayerCharacter::GetSingleton();

		if (player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDying || player->actorState1.lifeState == RE::ACTOR_LIFE_STATE::kDead) {
			return false;
		}

		// Both now go through CommonLibSSE-NG accessors. That matters on 1.7.99: PlayerFlags
		// moved from offset 0xBD8 to 0xBE0, so the old direct read would have hit the wrong
		// bytes. Bit 0x40 of that first flags byte is `escaping`.
		if (player->GetPlayerRuntimeData().jailSentence <= 0 || player->GetPlayerFlags().escaping) {
			return false;
		}

		UIFuncs::ShowServeSentenceQuestion();

		return true;
	}

	void MenuOpenHandler::InstallHooks() {
		Hooks::CanProcess.write_detour(Offsets::MenuOpenHandler::CanProcess.address());
		Hooks::ProcessButton.write_detour(Offsets::MenuOpenHandler::ProcessButton.address());
	}
} 