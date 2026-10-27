# Namco Museum Megamix (native port) - user documentation

# NOTICE!!!!
This project WAS made primarily with Claude Code. Test scripts and decompiled code were all generated from Claude Code.
I have been solely reverse engineering and working on my own C++ and Python tools for several years now before AI became a thing,
and my tools and years of gained insights were used extensively in the development of this project.

- Devin Hill

# The main stuff

A native PC (Linux) rebuild of the Wii game *Namco Museum Megamix* (US disc, SNMEAF) that runs the original game's data. The game logic
follows the Wii executable (`main.dol`) function by function; Wii services (graphics, sound, input, saves, disc) are replaced by a
platform layer on raylib / OpenGL. **No game data is included**: you need your own copy of the disc.

| document | contents |
|---|---|
| [OPTIONS.md](OPTIONS.md) | every command line flag, config key, key binding and environment variable |
| [JOBS.md](JOBS.md) | every game job (screen / mode) the native build runs, with its `--job` number and status |
| [GALAGA_STAGES.md](GALAGA_STAGES.md) | how Galaga Remix stages work (course commands, spawns, groups and gates, orbits), for the dev window's Galaga editor |
| `megamix --help` | the command line flags, printed by the executable itself (same list as OPTIONS.md section 2) |

## Quick start

1. Extract your disc with Dolphin (right-click the game > Properties > Filesystem > right-click the disc > Extract Entire Disc...). That
   gives a folder (Dolphin calls it `DATA`) with `files/` and `sys/` inside; `files/museum/` is what the game needs.
2. Install raylib (>= 5, tested with 6.0) and libjpeg-turbo, then build:
   `cmake -S . -B build -G Ninja && cmake --build build`
3. Run `./build/megamix --data /path/to/DATA` (or put `data_root = /path/to/DATA` in `megamix.cfg` in the working directory, or
   `MEGAMIX_DATA=/path/to/DATA`). `./build/megamix --help` lists the options.

`--data` accepts the `DATA` folder, its `files` folder or the folder above `DATA`; the disc's `sys/main.dol` is picked up by itself (it
provides the debug font and two engine textures). Without game data the game says where it looked and exits with code 2.
Without any setting the game looks for a `data` folder in the working directory.

Saves go to `save/` in the working directory (the game's own Wii save format). Use a copy while testing (`MEGAMIX_SAVE` / `save_dir`).

## Release packages

`tools/make_release.sh [--version V] [--clean] [--jobs N] [--check DATA_DIR] [--no-glibc] [--system-libs]` builds and packs
`dist/megamix-V-linux-ARCH.tar.gz`: the stripped executable, the `megamix.sh` launcher (runs from its own folder, so `megamix.cfg`,
`input.cfg` and `save/` sit next to it), a `megamix.cfg` template with `data_root`, a player README (`tools/release/README.txt`) and
README / OPTIONS / JOBS from this folder. It refuses to pack game data (main.dol, disc archives, BRRES / sound / movie files, saves).
`--check DIR` boots the packed game for 200 frames against the data in `DIR`, from another folder with a throw-away save.
The templates live in `tools/release/`.

* **raylib and libjpeg-turbo are linked statically**: the script builds with `cmake -DMEGAMIX_PORTABLE=ON` in `build-portable/`, which
  downloads raylib 6.0 and libjpeg-turbo 3.1.0 and builds them as static libraries (libjpeg-turbo without SIMD).
* **The C runtime is bundled in `lib/`**: glibc with its loader `ld-linux-x86-64.so.2`, libstdc++, libgcc_s and the X11 / xcb / SM / ICE /
  GLU libraries (everything `ldd` lists except libOpenGL / libGLX / libGLdispatch, which must be the system's GPU driver stack).
  `megamix.sh` starts the game through the bundled loader, so it also runs where the system glibc is older (e.g. a Chromebook's Linux
  container). `--no-glibc` leaves `lib/` out (the package then needs the build host's glibc).
* `--system-libs` makes the older kind of package: the system's raylib / libjpeg-turbo as `.so` files in `lib/`.

## What works today

* Boot splash screens, save / load dialogs, the title screen (3D castle scene, logo, particles), the attract movie (`--job 0x08800000`,
  with sound; the retail game never starts it by itself) and the developers' debug menu (`--job 0x08300000`, OPTIONS.md section 8).
* The main menu hub (castle garden): every game door leads into its game and back. The classic arcade building (job 140) is not ported.
* **Pac-N-Roll Remix**: stage select, all stages, "connect the Nunchuk" card, pause / retry / game over, staff roll, back to the hub.
  Gaps: OPTIONS.md section 9.
* **Pac-Motos Remix**: stage select, the stages, lives, "Play again?", game over, staff roll, multiplayer matching / menu, back to the hub.
* **Galaga Remix**: stage select, all ten stages including the boss, result, "Play again?", game over, staff roll; multiplayer for 2..4
  players (each player aims with their own pointer: Wiimote 1 = the mouse, Wiimotes 2..4 = their gamepad's right stick), result board and records.
* **Gator Panic Remix**: single player and multiplayer (2..4 players), tutorial, result, high score.
* **Rally-X Remix**: single player: every stage incl. the boss stages, items, flags, gimmicks, pause, game over, records. The stage select
  does not show its 3D map or Pac-Man's car yet. No multiplayer.
* **Grobda Remix**: single player (all enemy kinds, pause, records); multiplayer rounds with the result board (no Mii faces on the player icons).
* Sound: sound effects and music from the game's own sound archive (`Wii_Wander.brsar`, `sound/stream/*.brstm`). `--no-audio` turns it off.
* Saves use the game's real format (`museum.bin`, `banner.bin`, the same files the Wii writes; they can be copied to and from Dolphin's `Wii/title/00010000/534e4d45/data/`). A `pacnroll.bin` from older builds is imported once.

The game is 4:3 only (the original has no 16:9 mode: a widescreen Wii stretches it), so the port shows it with black bars.
Not there yet: Mii faces, the Home Button menu (HOME only freezes the game), shadows in some scenes, 3D sound for more than one listener (split screen),
Rally-X multiplayer. Known visual bugs: the Pac-Motos board looks see-through (the board's own colour animation makes it translucent; not yet compared with a Wii). See [JOBS.md](JOBS.md)
for every screen and its status.

## Keeping these docs current

Whoever adds or changes an option must update `OPTIONS.md`, the `--help` text (`PrintHelp` in `src/platform/main.cpp`) and, when the change
is visible to players, "What works today" above and `tools/release/README.txt`. A new job in `src/platform/boot.cpp` goes into `JOBS.md`.
