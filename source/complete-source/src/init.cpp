#include "init.h"
#include "Settings.h"
#include "Offsets.h"
#include "MenuOpenHandler.h"
#include "Fixes.h"
#include "JCApi.h"
#include "AIProcess.h"
#include "UIUtil.h"
#include "DiagUtil.h"
#include "SettingsPapyrus.h"

namespace Gotobed
{
	void OnJCApiInit() {
		if (!jc::api::ready() || !jc::api::setDefaultDomain("GTB_JCDomain")) {
			return;
		}

		AIProcess::InstallHooks();
	}

	void OnPostLoad() {
		jc::api::init(OnJCApiInit);
	}

	void Init() {
		auto& settings = Settings::Get();
		settings.Read();

		// don't show sleep menu on bed activation
		REL::safe_write(Offsets::TESFurniture::Activate.address() + 0x0177, static_cast<std::uint16_t>(0x0CEB));
		// don't show serve time message on bed activation
		REL::safe_write(Offsets::TESFurniture::Activate.address() + 0x0113, static_cast<std::uint8_t>(0xEB));
		// get up from bed using movement keys
		// This byte is the third octet of the immediate in `and eax, 0x3C000`, which isolates the
		// sit/sleep state out of the actor flags. Writing 0x02 over the 0x03 turns the mask into
		// 0x2C000, clearing bit 0x10000 so Sleeping (state 7, 0x1C000) compares equal to Sitting
		// (state 3, 0xC000) in the `cmp eax, 0xC000` that follows - which is what lets movement
		// keys get the player out of a bed.
		//
		// That block moved 7 bytes in Skyrim 1.7.99. At the old offset 0x63E the byte is now the
		// 0x0F of the following `jne`, and writing 0x02 there would corrupt the branch into
		// `add al, [rbp+0x9B]`. Verified by disassembling the 1.7.99 binary.
		REL::safe_write(
			Offsets::PlayerCharacter::Update.address() +
				(REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? 0x0637 : 0x063E),
			static_cast<std::uint8_t>(0x02));
		// don't allow sleeping if bed is reserved
		REL::safe_write(Offsets::PlayerCharacter::CanSleepWait.address() + 0x01B0, static_cast<std::uint8_t>(0x00));
		// don't pause character animations while sleeping
		REL::safe_write(Offsets::AIProcess::OnSitSleepStateChange.address() + 0x033F, static_cast<std::uint8_t>(0xEB));
		REL::safe_write(Offsets::AIProcess::sub_674B60.address() + 0x013C, static_cast<std::uint8_t>(0xEB));
		REL::safe_write(Offsets::Actor::FinishLoadGame.address() + 0x01B3, static_cast<std::uint8_t>(0xEB));

		// handle buttons
		MenuOpenHandler::InstallHooks();

		// fixes
		if (settings.fixes.multipleMarkersReservation) {
			Fixes::MultipleMarkersReservation::Install();
		}

		// papyrus
		UIUtil::Register();
		DiagUtil::Register();
		SettingsPapyrus::Register();

		SKSE::GetMessagingInterface()->RegisterListener("SKSE", [](SKSE::MessagingInterface::Message* a_msg) {
			if (!a_msg) {
				return;
			}

			switch (a_msg->type) {
				case SKSE::MessagingInterface::kPostLoad: {
					OnPostLoad();
					break;
				}
			}
		});
	}
}