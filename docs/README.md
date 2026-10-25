# Namco Museum Megamix (native port) - user documentation

All end-user documentation lives in this directory (developer notes are in `CLAUDE.md` and `reversing/dol/NOTES.md`).

| document | contents |
|---|---|
| [OPTIONS.md](OPTIONS.md) | every command line flag, config key, key binding and environment variable |
| [JOBS.md](JOBS.md) | every game job (screen / mode) the native build runs, with its `--job` number and status |
| [DOLPHIN.md](DOLPHIN.md) | running the original game in Dolphin next to the port: input, screenshots, job jumps, side-by-side pictures |
| `megamix --help` | the command line flags, printed by the executable itself (same list as OPTIONS.md section 2) |

## Quick start

1. Extract your own copy of the disc so that `<data_root>/museum/en/...` exists (default `~/Games/Wii/_nmm/DATA/files`; change it with `data_root` in `megamix.cfg`).
2. `cmake -S . -B build -G Ninja && cmake --build build` (needs raylib and libjpeg-turbo).
3. `./build/megamix` for the game from the boot screens, `./build/megamix --help` for the options.

The data folder can also be given as `--data DIR` or `MEGAMIX_DATA=DIR`; a Dolphin "Extract Entire Disc" folder works as `DIR`, `DIR/files`
or `DIR/DATA/files`, and its `sys/main.dol` is picked up by itself. Without game data the game says where it looked and exits.

## Release packages

`tools/make_release.sh [--version V] [--clean] [--jobs N] [--check DATA_DIR] [--no-glibc] [--system-libs]` builds and packs `dist/megamix-V-linux-ARCH.tar.gz`:
the stripped executable, the `megamix.sh` launcher (runs from its own folder, so `megamix.cfg`,
`input.cfg` and `save/` sit next to it), a `megamix.cfg` template with `data_root`, a player README (`tools/release/README.txt`) and
README / OPTIONS / JOBS from this folder. It refuses to pack game data (main.dol, disc archives, BRRES / sound / movie files, saves);
`resources/debug_font.brres` stays out because it comes from main.dol. `--check DIR` boots the packed game against the data in `DIR`
from another folder with a throw-away save. The templates live in `tools/release/`.

By default raylib and libjpeg-turbo are **linked statically**: the script builds with `cmake -DMEGAMIX_PORTABLE=ON` in `build-portable/`,
which downloads and builds raylib 6.0 and libjpeg-turbo 3.1.0 (no SIMD) as static libraries. The C runtime is bundled in `lib/` (glibc + `ld-linux-x86-64.so.2`, libstdc++, libgcc_s, the X11 / xcb / SM / ICE /
GLU libraries; everything `ldd` lists except libOpenGL / libGLX / libGLdispatch, which must be the system's GPU driver stack) and
`megamix.sh` starts the game through the bundled loader (`ld-linux --library-path lib --argv0 ...`), so it runs on older glibc systems
such as a Chromebook's Linux container (`--no-glibc` leaves lib/ out). `--system-libs`
makes the old package instead (system raylib / libjpeg-turbo bundled as .so in `lib/`).

## What works today

* Pac-N-Roll: every job of the game is in (see JOBS.md): stage select, all stages, "connect the Nunchuk" card (`--no-nunchuk`), pause / retry / game over, staff roll (`--job 0x0de00000`) and the way back to the hub. Gaps inside the game are listed in OPTIONS.md section 9.
* Pac-Motos: enter it from the hub's Pac-Motos door or `--job 0x11000000`. Stage select, stage 1 (A on a stage, then A again once the stage is shown), HUD, lives, "Play again?", game over and RETURN to the hub work.
* The main menu hub after the title, with the doors to Pac-N-Roll and Pac-Motos (`--job 0x08a00000`).
* The debug top menu (`--job 0x08300000`, see OPTIONS.md section 8).
* Boot splash screens, save/load dialogs, the title screen (3D castle scene with Pac-Man, logo, fades).
* Sound: the game's cues are played from the real sound archive (`Wii_Wander.brsar`: sound effects; `sound/stream/*.brstm`: music). `--no-audio` turns it off.
* The attract movie (`--job 0x08800000`, with sound). The retail game never starts it by itself.
* Saves are the game's real format; use a copy while testing (`MEGAMIX_SAVE` / `save_dir`).

See [JOBS.md](JOBS.md) for exactly which screens are in.

## Keeping these docs current

Whoever adds or changes an option must update `OPTIONS.md`, the `--help` text in `src/platform/main.cpp` and, when the change is visible to players, "What works today" above. A new job in `platform/boot.cpp` goes into `JOBS.md`.
