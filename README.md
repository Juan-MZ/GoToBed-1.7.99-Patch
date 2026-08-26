# Go to bed — 1.7.99 Compatibility Patch

> ## ⚠️ Status: **do not publish until andrelo1 says yes**
>
> The mod's permissions read *"Modification permission: You must get permission from me before you
> are allowed to modify my files to improve it."* That is an explicit gate, and it has not been
> cleared. The build below is finished and verified, but it stays here until the author replies.
> The message to send is in `nexus/01-permission-request.md`.

**This is not a mod. It is only the recompiled DLL.**
**It REQUIRES the original mod installed** — [Go to bed](https://www.nexusmods.com/skyrimspecialedition/mods/4224) v2.0.7 by andrelo1.

**Upstream (andrelo1's original):** <https://github.com/andrelo1/gotobed-se>

It ships none of the author's assets — no ESP, no Papyrus scripts, no MCM, no meshes. All of that
still comes from the original mod. The only file installed here is a rebuilt `gotobed.dll`.

### Licensing

The mod is **MIT**; CommonLibSSE-NG 6.7.0, which the DLL statically links, is
**GPL-3.0-or-later** with the Modding and Linking Exceptions. MIT is GPL-compatible, so the code can
be combined — but the resulting binary is **GPL-3.0-or-later**. That does not relicense andrelo1's
mod, which stays MIT (`LICENSE-upstream-MIT.txt`); it only sets the terms of the binary here.

GPL-3 §6 requires the complete corresponding source; it is in `source/complete-source/`, and §6(d)
allows offering it from a designated place rather than bundling it with the binary.
Not affiliated with andrelo1 or Bethesda.

## Built from the 2.0.7 release commit, not from `main`

This matters, and it is the reason the patch is safe to use with the mod people actually have.

`main` is two years past the release. It carries unpublished work — *"drop the use of JContainers"*,
*"move sleepwear into separate module"*, *"remove UIUtil"* — and it changed the **Papyrus API**:
`main` registers `Keys_GetSleepKey` on script class `GTB_Settings`, while the released 2.0.7 scripts
call `GetSleepKey` on `GTB_UIUtil` / `GTB_JCDomain`. Building `main` would produce a DLL that loads
fine and then leaves the MCM calling functions that no longer exist, with the sleepwear system gone.

So this is built from commit **`fba07fff`**, which declares `VERSION 2.0.7.0` — the version on Nexus.

The check that settles it: the rebuilt DLL declares itself **2.0.0.112**, byte-for-byte the same
version the shipped `gotobed.dll` declares. Comparing every printable string between the two, the
only ones missing from the rebuild are C-runtime and CommonLibSSE internals — **not one mod string**.
JContainers, the sleepwear data, and all the Papyrus entry points are present and unchanged.

## Who needs this (once it is releasable)

You are on **Skyrim SE/AE 1.7.99** (the 20 August 2026 update) and Go to bed throws:

```
REL/Relocation.h(636): Unsupported address library format: 5
```

Note this is a *different* error from most broken plugins. The DLL finds and opens the new
`versionlib-1-7-99-0.bin` — it just refuses it, because it was built against a CommonLibSSE that
only understands formats 1 and 2.

If you are still on 1.6.x, you do **not** need this patch — keep using the official release.

## Requirements

- **Go to bed 2.0.7** (the original mod — required)
- SKSE64 **2.3.0** (the 1.7.99 build)
- Address Library for SKSE Plugins (updated — must include `versionlib-1-7-99-0.bin`)
- Whatever the original mod lists (JContainers, and SkyUI for the MCM)

## Installation

Use **`GoToBed-1.7.99-Compatibility-Patch.zip`** as-is, without extracting it.

### Vortex

1. Install the original mod 2.0.7 normally first.
2. Mods tab → *Install From File* → pick the zip → Enable.
3. Vortex will report a **file conflict** with the original mod (both ship the same DLL).
   When prompted, set this patch to *Load After* the original mod so **this patch wins**.

> If you pick the other option, Vortex keeps deploying the old DLL and it will look like the patch
> did nothing.

### Mod Organizer 2

*Install a new mod from an archive*, then place this patch **below** the original mod in the left
pane (in MO2, lower wins).

### Manual

Copy `SKSE\Plugins\gotobed.dll` over the existing one — only if you installed the mod manually.
With Vortex or MO2, always go through the manager.

### Verifying it worked

Launch the game. If it reaches the main menu with no error box, it worked. The plugin writes
`Documents\My Games\Skyrim Special Edition\SKSE\gotobed.log`. Then sleep in a bed and check the
things this mod exists for: no sleep menu popping up on activation, the sleepwear swap, and getting
out of bed with a movement key.

## What was broken

Skyrim 1.7.99 shipped a new Address Library format (**format 5** — a 96-byte header and a dense
array indexed directly by ID; a 1.7.99 `versionlib-1-7-99-0.bin` starts with the bytes
`05 00 00 00`). The released DLL was built against the old Ryan-rsm-McKenzie CommonLibSSE, pinned by
the mod's own vcpkg overlay port, which only understands formats 1 and 2 and rejects the file.

**And one of the mod's byte patches no longer landed where it used to** — see Verification.

## What was changed

- **Ported to CommonLibSSE-NG 6.7.0** — format 5 support plus the 1.7.99 layouts. The bundled
  `commonlibsse` overlay port is removed and CommonLib is built from source instead. **C++20 → C++23.**
- **`PlayerCharacter::Update` byte patch re-derived for 1.7.99.** This is the substantive fix; the
  details are in Verification below.
- **`SKSE::WinAPI` → `REX::W32`** is not needed here, but three other CommonLib renames are:
  `Actor::currentProcess` now goes through `GetActorRuntimeData()`, `middleHigh->unk2E4` is named
  `reservationSlot` (same offset `0x2E4`, and the name confirms the mod's use of it), and
  `TESObjectARMO::GetSlotMask()` returns a `REX::EnumSet`, so it needs `.underlying()`.
- **`PlayerCharacter` flags now go through accessors.** `jailSentence` and the flag byte the mod
  read as `unkBD8` are reached via `GetPlayerRuntimeData()` and `GetPlayerFlags()`. This one is not
  cosmetic: **on 1.7.99 those flags moved from offset `0xBD8` to `0xBE0`**, so the old direct read
  would have tested the wrong bytes. Bit `0x40` there is `escaping`.
- **`#undef GetObject`.** `<windows.h>` now arrives through CommonLibSSE-NG before the mod's own
  `NOGDI`, and wingdi's `GetObject` macro was rewriting
  `BGSDefaultObjectManager::GetObject` into `GetObjectA`.
- **`IsPressed()` inlined.** CommonLibSSE-NG declares `BSWin32KeyboardDevice`'s virtual overrides
  without defining them — they are the game's own vtable slots. Calling `IsPressed()` drags that
  translation unit into the link and MSVC then cannot emit the vtable. The mod now performs the
  identical check (`curState[key] & 0x80`) directly.

**No feature was added, removed, or redesigned.** The DLL still reports itself as 2.0.0.112.

## Verification

Every one of the mod's patch sites was checked by disassembly against a real 1.7.99 `SkyrimSE.exe`
and its `versionlib-1-7-99-0.bin`.

- **All 20** Address Library IDs the mod uses resolve on 1.7.99.
- **Seven of the eight** byte patches still land exactly right. Simulating each write on the real
  binary and disassembling the result, every one keeps its instruction boundary and length:

  | site | before | after |
  |---|---|---|
  | `TESFurniture::Activate + 0x177` | `mov cl, 1` | `jmp` (skips the sleep menu call) |
  | `TESFurniture::Activate + 0x113` | `jbe` | `jmp` |
  | `PlayerCharacter::CanSleepWait + 0x1B0` | `mov dl, 1` | `mov dl, 0` |
  | `AIProcess::OnSitSleepStateChange + 0x33F` | `je` | `jmp` |
  | `AIProcess::sub_674B60 + 0x13C` | `jb` | `jmp` |
  | `Actor::FinishLoadGame + 0x1B3` | `jne` | `jmp` |
  | `BGSProcedureSitSleepExecState::ActivateTarget + 0x2B4` | 5-byte `E8` call to AE id 20160 (`SetMarkerReserved`) — exactly the function the hook replaces | |

  The first one is worth spelling out: the mod writes `EB 0C`, and on 1.7.99 that jump lands on
  `+0x185`, precisely the instruction boundary after the `ShowSleepWaitMenu` call, its `mov al, 1`
  and its `jmp`. The displacement is exactly right by construction.

- **The eighth was wrong.** `PlayerCharacter::Update + 0x63E` is where the mod writes `0x02` to make
  movement keys get you out of a bed. That byte is meant to be the third octet of the immediate in
  `and eax, 0x3C000`, which isolates the sit/sleep state; turning the mask into `0x2C000` clears bit
  `0x10000`, so **Sleeping (state 7, `0x1C000`) compares equal to Sitting (state 3, `0xC000`)** in
  the `cmp eax, 0xC000` that follows.

  The block moved **7 bytes** in 1.7.99. At `+0x63E` the byte is now the `0F` of the following
  `jne`, and writing `0x02` there would have corrupted the branch into `add al, [rbp+0x9B]` —
  destroying the conditional entirely.

  ```
  +0x634  and  eax, 0x3C000     <- the byte to patch is at +0x637
  +0x639  cmp  eax, 0xC000
  +0x63E  jne  ...              <- where the old offset now points
  ```

  The patch uses `0x637` on 1.7.99 and keeps `0x63E` on AE 1.6.x, chosen at runtime via
  `REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99)`.

- The built DLL declares `versionIndependenceEx = 3` — the Address Library v5 bit is the fix; the
  shipped original declares `0`. `versionIndependence` stays `5` (AddressLibraryPostAE +
  StructsPost629), identical to the original.

**It has not been play-tested in-game.** The verification above is static analysis. Back up your
save before using this on a main playthrough, and please report anything odd — especially getting
out of bed with movement keys, since that is the re-derived patch.

## Building from source

`source/complete-source/` holds the complete corresponding source required by GPL-3 §6, and
`source/GoToBed-1.7.99-compat.patch` is the diff against upstream commit `fba07fff`; it applies
cleanly with `git apply`.

```sh
git clone https://github.com/andrelo1/gotobed-se.git
cd gotobed-se
git checkout fba07fff          # the 2.0.7 release, NOT main
git apply ../GoToBed-1.7.99-compat.patch
git clone --depth 1 --branch ng https://github.com/alandtse/CommonLibVR.git external/CommonLibNG
set VCPKG_ROOT=<path to vcpkg>
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -DVCPKG_OVERLAY_PORTS=vcpkg/ports ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
cmake --build build --config RelWithDebInfo
```

Requires Visual Studio 2022 (or Build Tools) with MSVC v143, Windows SDK 10, CMake and vcpkg.
Built with CommonLibSSE-NG 6.7.0 (commit `3d81614`), MSVC 14.44, Windows SDK 10.0.26100.

`symbols/gotobed.pdb` (full download only) makes crash logs show function names. It is not needed
to play.

## Credits

- **andrelo1** — creator of Go to bed. The mod is entirely their work.
- **meh321** — Address Library for SKSE Plugins.
- **ryobg / silvericed** — JContainers, which the mod uses.
- **CommonLibSSE-NG** contributors (CharmedBaryon, alandtse, powerof3, Ryan-rsm-McKenzie) — the
  1.7.99 and format-5 support this patch depends on.
- **The SKSE team** — SKSE64.

Made for the community. Nothing asked in return. If andrelo1 updates the mod, use their version —
this patch only exists so the mod can keep being played in the meantime.
