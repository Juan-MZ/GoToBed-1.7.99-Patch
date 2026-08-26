# Message to andrelo1 — permission request

Send this on Nexus (PM) or GitHub. Nothing gets published until they reply — their permissions
require it in writing.

Keep it in English.

---

**Subject:** Go to bed on Skyrim 1.7.99 — asking permission for a compatibility patch

Hi andrelo1,

Your mod page says modification requires permission first, so I'm asking before doing anything
public.

The 20 August 2026 Skyrim update (1.7.99) shipped a new Address Library format, and Go to bed now
fails on launch with:

    REL/Relocation.h(636): Unsupported address library format: 5

I've been rebuilding a few broken SKSE plugins against CommonLibSSE-NG 6.7.0 and putting the
recompiled DLL up as a DLL-only compatibility patch — no ESP, no scripts, no MCM, nothing of yours
redistributed, and your mod required as a dependency. It exists only so people can keep playing
until the author updates, and it comes down the moment you release your own build.

I have it building and verified. Two things I think are worth your time regardless of what you
decide about the patch.

**One of your byte patches is wrong on 1.7.99, and it fails quietly.**

In `init.cpp`:

    REL::safe_write(Offsets::PlayerCharacter::Update.address() + 0x063E, static_cast<std::uint8_t>(0x02));

That byte is the third octet of the immediate in `and eax, 0x3C000` — turning the mask into
`0x2C000` clears bit `0x10000`, so Sleeping (state 7) compares equal to Sitting (state 3) in the
`cmp eax, 0xC000` right after. That's what lets movement keys get the player out of a bed.

On 1.7.99 that block moved **7 bytes**. The layout is now:

    +0x634  and  eax, 0x3C000     <- the byte you want is at +0x637
    +0x639  cmp  eax, 0xC000
    +0x63E  jne  ...              <- where 0x63E now points

So writing `0x02` at `0x63E` lands on the `0F` of that `jne` and turns it into
`add al, [rbp+0x9B]` — the conditional is destroyed. I gated it on
`REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99)` and used `0x637`, keeping `0x63E` for AE 1.6.x.

I checked all eight of your patch sites by disassembling the real 1.7.99 binary and simulating each
write. **The other seven are all still exactly right**, including the `EB 0C` in
`TESFurniture::Activate` — that jump still lands precisely on the instruction after the
`ShowSleepWaitMenu` call. Only the one above moved.

**A struct offset moved too.** `MenuOpenHandler.cpp` reads `player->unkBD8 & 0x40`. On 1.7.99 the
player flags moved from `0xBD8` to `0xBE0`, so that read now hits the wrong bytes. CommonLibSSE-NG
handles it through `GetPlayerFlags()`, and bit `0x40` there turns out to be `escaping` — which
matches exactly what your check is doing.

Beyond those, the port was small: CommonLibSSE-NG renamed `Actor::currentProcess` behind
`GetActorRuntimeData()`, named `middleHigh->unk2E4` as `reservationSlot` (same offset, and the name
confirms your use of it), and `GetSlotMask()` now returns a `REX::EnumSet` needing `.underlying()`.
Plus C++20 → C++23, and `UsesAddressLibrary()` lost its bool parameter.

**On which source to build.** I built from commit `fba07fff`, which declares `VERSION 2.0.7.0` —
your Nexus release — rather than `main`. `main` drops JContainers, moves sleepwear out, and changes
the Papyrus API (`Keys_GetSleepKey` on `GTB_Settings` instead of `GetSleepKey`), so a DLL built from
it would load fine and then leave the shipped MCM calling functions that no longer exist. The 2.0.7
rebuild declares itself 2.0.7.0, the same as your shipped DLL, and a full string comparison shows
nothing of the mod missing.

**One licensing note.** CommonLibSSE-NG changed from MIT to GPL-3.0-or-later in August 2026. Because
the DLL statically links it, the compiled binary has to go out as GPL-3 with corresponding source
available. That doesn't touch your mod — Go to bed stays MIT, and I'd say so plainly. I mention it
because plenty of pages still credit CommonLibSSE as MIT and it's easy to miss.

So, concretely: are you OK with me publishing a DLL-only 1.7.99 compatibility patch for Go to bed,
crediting you, requiring your mod, and taking it down when you update?

If the answer is no, that's completely fine and I'll leave it alone. And if you'd rather just do the
1.7.99 build yourself, I'd honestly prefer that — I'm happy to send you the diff so you don't have
to re-derive the offset.

Thanks for the mod. It's been in my load order for years.

— Juan (Juan-MZ)

---

## If they say no

Drop it, and don't publish anything. Consider still sending them the offset finding — a wrong
`safe_write` is worth knowing about whether or not a patch ever ships.

## If they say yes

Then finish the Nexus page: the description hasn't been written yet, deliberately, so it can
describe whatever ends up actually being published. Permissions setup follows the same pattern as
the other patches in this series (GPL-3 in Author's instructions, permissive dropdowns).
