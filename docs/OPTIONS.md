# Namco Museum Megamix (native port) - options

Every way to configure or steer the native build. **Keep this file up to date**: add a row whenever a command line flag, environment
variable, config key or key binding is added, renamed or removed, together with the `--help` text (`PrintHelp` in `src/platform/main.cpp`).

Where settings come from, in priority order: command line > environment > `megamix.cfg` / `input.cfg` > built-in defaults.

## 1. Files

| file | purpose |
|---|---|
| `megamix.cfg` (working directory) | paths, sound, language, see section 3; optional |
| `input.cfg` (working directory) | controller bindings, written with defaults on first run, see section 4 |
| `<save_dir>/museum.bin`, `banner.bin` | the game's save data (NAND `00010000/534e4d45/data` layout, checked against a real Dolphin save) |
| disc files (`data_root`) | the extracted disc `files` folder: `museum/en/...`, `museum/resident.arc`, `museum/resource.lzs`, `museum/en/g10_menu/attract.thp`, `museum/en/sound/Wii_Wander.brsar` and `museum/en/sound/stream/*.brstm` |

## 2. Command line (`./build/megamix ...`)

| flag | meaning |
|---|---|
| `--job HEX` | start at this job instead of the boot chain, e.g. `--job 0x08300000` (debug menu), `--job 0x08900000` (title), `--job 0x08800000` (attract movie). Job number = table index << 20; every wired job is listed in JOBS.md |
| `--round N` | Pac-N-Roll stage to play, 0..25 (0 = World 1 Stage 1, 5 = W2S1, 9 = W3S1, 14 = W4S1, 20 = W5S1) when the game job is entered directly (`--job 0x0d300000`; default 0) |
| `--next HEX` | job the save/load transition returns to when entered with `--job` (sets `cKingdomSys+0xa88`) |
| `--dump-tree [N]` | run N frames (default 400) without waiting for input, print the task tree, exit. Must be the first argument |
| `--no-nunchuk` | Wiimote 1 counts as having no Nunchuk, so Pac-N-Roll shows its "connect the Nunchuk" card (job `0x0d800000`, 3 seconds) before a stage; by default the PC always has a stick and the card is skipped, as on a Wii with a Nunchuk |
| `--no-smooth` | turn off the edge smoothing (4x MSAA on polygon edges; text and textures are never filtered by it; on by default); same as `MEGAMIX_NOSMOOTH=1` |
| `--shot FRAME FILE` | save the window to a PNG at that frame; repeatable |
| `--stick FRAME X Y` | Nunchuk stick (-1..1, y up) from that frame on; repeatable (tests steering without a keyboard) |
| `--press FRAME BUTTON` | one-frame press of a Wiimote button: `a b 1 2 plus minus home left right up down z c shake` (physical directions); repeatable. Prefix `N:` presses on Wiimote N instead of 1 (`--press 300 2:a`: player 2 in multiplayer) |
| `--hold FRAME BUTTON` | hold a Wiimote button from that frame on (names as `--press` except `home`, including the `N:` Wiimote prefix; `up`/`down`/`left`/`right` are the Wiimote buttons) |
| `--point FRAME X Y` | put the pointer at game pixel X,Y (640x480, origin top-left) from that frame on; replaces the mouse; repeatable (the last entry whose FRAME has passed wins) |
| `--preview-all` | developer switch (not part of the 1:1 game): every Pac-N-Roll, Pac-Motos and Galaga stage and world reads as unlocked so each menu world can be viewed; progress is not saved while it is on |
| `--g03-life N` | developer switch (not 1:1): Pac-N-Roll starts with N extra lives instead of the save's value (also `MEGAMIX_G03_LIFE=N`) |
| `--g04-time S` | developer switch (not 1:1): Gator Panic's play time is S seconds instead of CSV_GAME's 120 (tests the time up, result and "Play again?" quickly) (also `MEGAMIX_G04_TIME=S`) |
| `--g02-stage N` | developer switch (not 1:1): the Rally-X stage job entered directly (`--job 0x0c300000`) plays stage index N (1..30; 1 = world 1 stage 1) (also `MEGAMIX_G02_STAGE=N`) |
| `--g01-autoaim` | developer switch (not 1:1): while A is held with the pointer on screen, Galaga shots aim at the nearest enemy or missile in front of the camera (also `MEGAMIX_G01_AUTOAIM=1`) |
| `--g01-invincible` | developer switch (not 1:1): Galaga's Pac-Man takes no damage, to test a whole course (also `MEGAMIX_G01_INVINCIBLE=1`) |
| `--g01-stage N` | developer switch (not 1:1): the Galaga stage job entered directly (`--job 0x0b300000`) plays stage N (1..10) instead of stage 1 (also `MEGAMIX_G01_STAGE=N`) |
| `--g03-cookies N` | developer switch (not 1:1): Pac-N-Roll starts with N Pac-Dots in hand, to test gates (also `MEGAMIX_G03_COOKIES=N`) |
| `--g03-kill N` | developer switch (not 1:1): Pac-N-Roll deals lethal damage to Pac-Man at game frame N, to test the miss / retry / game over flow (also `MEGAMIX_G03_KILL=N`) |
| `--g03-start X Y Z` | developer switch (not 1:1): Pac-N-Roll starts Pac-Man at this stage position (also `MEGAMIX_G03_START=x,y,z`) |
| `--g03-standin` | developer switch: Pac-N-Roll runs the old native stand-in game (`cAppG03PacnRollPlay`, a bare movement test) instead of the real game `cAppG03PacnRollGame` |
| `--tap-every N` | inject an A press every N frames (testing) |
| `--no-audio` | run without sound: no audio device is opened (movie and sound effects silent) |
| `--data DIR` | the game data: the extracted disc's `files` folder (the one holding `museum/`). A Dolphin "Extract Entire Disc" folder works as `DIR`, `DIR/files` or `DIR/DATA/files`. Overrides `data_root` and `MEGAMIX_DATA`. When no game data is found the game shows a message (in the window and on stderr) and exits with code 2 |
| `--dol FILE` | the game executable `main.dol`. Overrides `dol_path` and `MEGAMIX_DOL`. Optional: without it the debug font (debug menus) and the engine textures depth / ramp16 (some shadows) are missing |
| `-h`, `--help` | print the option summary and exit |
| `--view-model ARCHIVE ENTRY [MODEL]` | orbit viewer for a BRRES model, e.g. `--view-model g10_menu/g10title.lzs TITLE_PACMAN_MDL`. Left drag = orbit, wheel = zoom |
| `--chr ENTRY` / `--srt ENTRY` / `--pat ENTRY` | (with `--view-model`) play a bone / texture-scroll / texture-swap animation entry of the same archive |
| `--at FRAME` | (with `--view-model`) freeze the animations at that frame instead of playing them |

Example: `MEGAMIX_SAVE=/tmp/save ./build/megamix --dump-tree 130 --job 0x08900000 --press 100 a --shot 120 title.png`
(use a scratch save folder for test runs: a run that reaches a result screen writes the save).

Flags that take no value (`--preview-all`, `--g01-invincible`, `--g01-autoaim`, `--g03-standin`) are only seen when another argument follows
them; `--no-smooth`, `--no-nunchuk` and `--no-audio` work anywhere.

## 3. `megamix.cfg` (`key = value`, `#` starts a comment)

| key | default | meaning |
|---|---|---|
| `data_root` | `data` in the working directory if it exists, else `~/Games/Wii/_nmm/DATA/files` (the original developer's folder) | extracted disc `files` directory (the one holding `museum/`); its parent, or a Dolphin `DATA` folder above it, is accepted too; `~/` is expanded |
| `language` | `en` | folder under `museum/` to load. The US disc (SNMEAF) has only `en`; other values (`jp`, `de`, `fr`, `es`, `it`, `nl`, `kr`) only change the language the game reports (`SCGetLanguage`) and need a disc with that folder. Only `en` is tested |
| `save_dir` | `save` | folder standing in for the game's NAND save directory |
| `dol_path` | `main.dol` in the working directory; when that file does not exist, the disc's `sys/main.dol` next to the `files` folder | the game executable (engine textures depth / ramp16 and the debug font; a `resources/debug_font.brres` in the working directory is read first if present) |
| `audio` | `on` | `off` (or `0`, `false`, `no`) disables all sound, same as `--no-audio` |
| `volume` | `100` | master volume 0..100 for the sound effects and music |

Unknown keys are reported on stderr and ignored. The environment variables of section 6 override these keys.

## 4. `input.cfg` (controller bindings)

Format: `<input> = <source>, <source>, ...` (any listed source triggers the input). Delete the file to restore the defaults.

* **Pad semantics (from the DOL)**: the game sees the same button words as on the Wii (A = decide, MINUS = pause in Pac-N-Roll, HOME held = freeze); the title / dialogs accept **A** (not 1, 2 or Plus), a dialog accepts A, B or Z.
* **Inputs**: `Left Right Up Down Plus Minus Home 1 2 A B Z C Shake` (Wiimote / Nunchuk; `Shake` = swing the remote, default `Space` / gamepad R2: Pac-N-Roll's **dash** and hang escape and Gator Panic's **whack** need a swing, there is no accelerometer; in Gator Panic the Nunchuk stick picks the hole: up / left / right / centre), `StickUp StickDown StickLeft StickRight` (Nunchuk stick).
* **Sources**: `Key:<name>` keyboard (channel 1 only), `Pad:<button>` gamepad of the same channel (`South East West North L1 R1 L2 R2 Back Start Guide L3 R3 DUp DDown DLeft DRight`),
  `Pad2:<button>` / `Axis2:...` a specific gamepad (index from 0), `Axis:<axis>+/-` (`LeftX LeftY RightX RightY LTrigger RTrigger`), `Mouse:<button>` (`Left Right Middle`, channel 1 only).
* `Orientation = Sideways | Upright` (default `Sideways`): `Sideways` rotates the D-pad bindings as for a remote held sideways, `Upright` passes them
  through. It applies to the boot screens, save dialogs, title, attract movie and the debug menus. The hub and every game (they use the Nunchuk, so the
  remote is held upright) always use `Upright`, whatever the setting.

The IR pointer is the mouse: it covers the 4:3 game screen centred in the window (outside the window = no pointer, the menus then use the D-pad). The cursor is hidden while HOME is held (HOME stands in for the Home Button Menu: while it is held every game task freezes, like the original with the HBM open). Click = A.

In Pac-N-Roll (as in the original) Pac-Man **dashes when you swing the remote (Space)**, is steered with the **Nunchuk stick** (`WASD` / the gamepad's left stick), brakes while **B or Z** is held, and the D-pad (arrows) turns the camera.

Default keys: arrows = D-pad, `J` / Enter / left mouse = A, `K` / right mouse = B, `1` / `2` = the remote's 1 / 2 buttons, `=` = Plus, `-` = Minus, `H` = Home,
left Shift = Z, left Ctrl = C, Space = swing (Shake), `WASD` = Nunchuk stick.
Default gamepad: South = A, East = B, West = 1, North = 2, Start = Plus, Back = Minus, Guide = Home, L1 = Z, R1 = C, R2 = swing, D-pad = D-pad,
left stick = Nunchuk stick. Keyboard and mouse drive Wiimote 1; the first gamepad drives Wiimote 1, the second Wiimote 2, and so on.

## 5. Runtime keys

| key | meaning |
|---|---|
| `P` | print the task tree to the terminal |

## 6. Environment variables

| variable | meaning |
|---|---|
| `MEGAMIX_DATA` | overrides `data_root` |
| `MEGAMIX_SAVE` | overrides `save_dir` (point at a **copy** of a save) |
| `MEGAMIX_DOL` | overrides `dol_path` |
| `MEGAMIX_NO_AUDIO=1` | same as `--no-audio` |
| `MEGAMIX_SND_LOG=1` | print every sound cue the game plays (label, voice, stream or wave) |
| `MEGAMIX_TRACE_ANITA=1` | print every 2D layout quad's corners |
| `MEGAMIX_ONLY_MAT=name` | debug: the model renderer draws only materials with this name (with `--view-model`) |
| `MEGAMIX_BG=r,g,b` | debug: clear colour of the window |
| `MEGAMIX_TEV_STAGES=N` | debug: the model renderer runs only the first N TEV stages of every material |
| `MEGAMIX_NOBLEND=1` | debug: the model renderer draws every material opaque |
| `MEGAMIX_DUMP_TEV=name` | debug: print the TEV program of the material with this name |
| `MEGAMIX_NOSMOOTH=1` | turn off the MSAA edge smoothing (see `--no-smooth`); the old name `MEGAMIX_NOFXAA=1` still works |
| `MEGAMIX_NOSCENE=1` | title: skip the 3D scene (2D layers only) |
| `MEGAMIX_NOENGTEX=1` | do not provide the engine textures `ramp16_tex` / `depth_tex` (shadow receivers then sample white) |
| `MEGAMIX_GATELOG=1` | g03: log every gate / box read from the stage data (kind, cookies, position) so tests can place Pac-Man with `--g03-start` |
| `MEGAMIX_NOPTK=1` | title: do not draw the jet particles (wind / leaves) |
| `MEGAMIX_PTKLOG=1` | g03: log every particle emitter the game spawns (slot / emitter id / generator count) |
| `MEGAMIX_IND_LOG=1` | log every material with indirect texture stages (Gator Panic water): indirect maps / coordinates, matrix 0, per-stage IND_CMD fields |
| `MEGAMIX_SKIP_SHAPE=name` | 3D debug: do not draw the shape with this name |
| `MEGAMIX_NOCULL=1` | 3D debug: disable back-face culling |
| `MEGAMIX_NOVCOL=1` | 3D debug: ignore vertex colours |
| `MEGAMIX_NODEPTH=1` | 3D debug: no depth test |
| `MEGAMIX_CHAN_LOG=1` | 3D debug: log the colour channels of every material |
| `MEGAMIX_TEXGEN_LOG=1` | 3D debug: log the materials whose texture coordinates are not read from a UV set (environment maps) |
| `MEGAMIX_TEV_GENERIC=1` | 3D debug: use the generic TEV shader instead of the one specialised per material (comparison) |
| `MEGAMIX_ONE_PASS=1` | 3D debug: draw models in the old per-model order instead of an opaque pass then a sorted translucent pass |
| `MEGAMIX_DRAWSTATS=1` | print the average draw calls / vertices / indices per frame every 120 frames |
| `MEGAMIX_UNCAPPED=1` | no 60 fps frame cap (for measurements with `MEGAMIX_DRAWSTATS`; the game then runs too fast) |
| `MEGAMIX_G03_START=x,y,z` | same as `--g03-start X Y Z` |
| `MEGAMIX_G03_LIFE=N` | same as `--g03-life N` |
| `MEGAMIX_G03_COOKIES=N` | same as `--g03-cookies N` |
| `MEGAMIX_G03_KILL=N` | same as `--g03-kill N` |
| `MEGAMIX_PREVIEW_ALL=1` | same as `--preview-all` |
| `MEGAMIX_G01_STAGE=N` | same as `--g01-stage N` |
| `MEGAMIX_G01_INVINCIBLE=1` | same as `--g01-invincible` |
| `MEGAMIX_G01_AUTOAIM=1` | same as `--g01-autoaim` |
| `MEGAMIX_G02_STAGE=N` | same as `--g02-stage N` |
| `MEGAMIX_G04_TIME=S` | same as `--g04-time S` |
| `MEGAMIX_G04DBG=1` | Gator Panic game: print the app mode changes and, once a second, the timer, hits, score, the four gators' states and Pac-Man's state |
| `MEGAMIX_G01_PLAYERS=N` | Galaga: with `--job 0x0b200000` (stage select) or `0x0b300000` (stage), play with N players (2..4, pads 0..N-1; empty names become PLAYERn) |
| `MEGAMIX_G11_PLAYERS=N` | Grobda: with `--job 0x15300000`, start the stage with N players (1..4; N > 1 = multiplayer rules) |
| `MEGAMIX_G01DBG=1` | Galaga stage: print player 1's score / kills / shots / hits and the life gauge to the terminal once a second, plus app mode changes, Pac-Man states, kills, bombs, fighters and course effects with their frame numbers (the frame `--shot` uses) |
| `MEGAMIX_LENSDBG=1` | Galaga stage: print the lens flare's light position on screen, its visibility and every sprite layer each frame |
| `MEGAMIX_SCENE_DEBUG=1` | Pac-N-Roll debug output: scene / camera state, the list of drawn models at frame 100, Pac-Man's position every 30 frames; in Galaga (per-frame model submission) also the loaded visible models no task submitted that frame |
| `MEGAMIX_NO_ZSORT=1` | debug: draw translucent models in load order instead of back to front (the original sorts them, nw4r ScnRoot::ZSort) |
| `MEGAMIX_NOPOSE=1` | models are drawn in their bind pose (debug) |
| `MEGAMIX_MENU_DEBUG=1` | Pac-N-Roll stage select: print the menu Pac-Man's draw state each frame |
| `MEGAMIX_TUTORIAL=1` | Pac-N-Roll stage select: open the tutorial as soon as the menu is idle (clicking the Rule button with the pointer does the same) |
| `MEGAMIX_RUMBLE_LOG=1` | debug: print every rumble request (`rumble: pad N MS ms`, or "refused" while the remote counts as lying still: 600 frames without input). Rumble itself drives the channel's gamepad motors |
| `VIEW_YAW=radians` | `--view-model`: starting yaw of the orbit camera |
| `VIEW_NOLIGHT=1` | `--view-model`: ambient light only (default is the title's model light) |

## 7. Build-time

| item | meaning |
|---|---|
| `cmake -S . -B build -G Ninja` | needs a C / C++20 compiler, CMake >= 3.20, raylib (>= 5, tested with 6.0) and libjpeg-turbo (`libturbojpeg`, THP movie frames) |
| `-DMEGAMIX_PORTABLE=ON` | download raylib 6.0 and libjpeg-turbo 3.1.0 and link them statically instead of using the system's (used by `tools/make_release.sh`; off by default) |
| `cmake/sources.cmake` | the list of source files that compile and link |
| `tests/run.sh` | host tests (no window); the decoder tests also run when the disc's `files` folder is found (`MEGAMIX_DATA=/path/to/files`) |
| `tools/check_all.sh` | syntax-check every `src/**/*.cpp` (slow) |
| `tools/safe_run.sh LIMIT -- CMD` | run the game with a wall-clock limit (test runs never leave a window behind) |
| `tools/make_release.sh` | build a player package, see README.md "Release packages" |

## 8. Debug menu (job `0x08300000`)

`./build/megamix --job 0x08300000` opens the original debug build's top menu ("Kingdom Final Sep 30 2010 21:51:51"): BOOT, ATTRACT, TITLE, the
games (G01 Galaga, G02 Rally X, G03 Pac-N-Roll, G04 Wani Panic, G07 Motos, G11 Grobda, RSO Original) and three save data rows. **RSO : ORIGINAL and
the Load / Save / Delete Save Data rows lead to jobs that are not ported: the screen goes empty and stays so** (restart the game).

* Up / Down arrows (or the D-pad) move the cursor, A selects. The original reads KPAD LEFT / RIGHT for this and moves up on LEFT; this port swaps the two so
  the **Up** arrow moves **up** with the default `Orientation = Sideways` (with `Orientation = Upright` the Left arrow then moves down and Right moves up).
* The text is the engine's own debug text (8x8 glyph font read from `main.dol` 0x80442bc0, see `dol_path`, or from `resources/debug_font.brres` in the working directory if that exists;
  without either the debug menus have no text).

Every game has its own debug menu with the same controls: Galaga `--job 0x0b100000`, Rally-X `0x0c100000`, Pac-N-Roll `0x0d100000`, Gator Panic
`0x0e100000`, Pac-Motos `0x11100000`, Grobda `0x15100000` (JOBS.md lists their rows).

The **Pac-N-Roll debug menu** (MENU, STAGE, EDIT, EDIT2, EDIT_ENEMY, GAMEOVER, STAFFROLL, EXIT): on STAGE / EDIT / EDIT2 / EDIT_ENEMY the
**Left / Right** arrows step the round (0..27) while the cursor is on one of those four rows; the STAGE row always shows `w%02ds%02d` (world / stage, e.g.
`w01s05`). Going back from `w01s01` wraps to round 27 and one more to round 26 (the text stays `w01s01`, as in the original, because world / stage are 0
for rounds above 25): round 26 = `g03wXXs03` (the Namco Classic Arcade hall), round 27 = `g03wXXs04` (castle garden). A on any of the four rows starts
the game with that round (the four jobs go straight into the game in the original too). GAMEOVER and STAFFROLL show those screens.

## 9. Pac-N-Roll (jobs `0x0d000000`..`0x0df00000`)

`--job 0x0d000000` (or the "G03 : PACN ROLL" row of the debug menu, or the hub's Pac-N-Roll door) opens the stage select; `--job 0x0d300000 --round N`
starts round N directly.

* **Controls (as on the Wii):** the Nunchuk stick (`WASD` / the gamepad's left stick) rolls Pac-Man, relative to the camera; **swinging the remote
  (Space / R2) dashes**; **B or Z** held brakes; the D-pad (arrows) turns the camera; **MINUS** pauses.
* **Stage select:** choose a world and stage with the pointer or the D-pad and A; the "connect the Nunchuk" card (job 216) only appears with
  `--no-nunchuk`. RETURN (top left; press up from the stage row until it is highlighted, then A) goes back to the hub.
* **Progress** (unlocked stages, records) is saved in `<save_dir>/pacnroll.bin` (separate from `museum.bin`; where the original keeps it is not traced yet).
* **Not ported yet:** moving gimmicks (objects Pac-Man can stand on that move), water, hanging on ledges, the knight / wing forms, some round-specific
  behaviour (rounds 5, 9, 15, 22). Ghosts, bosses, gates, boxes, items, lives, pause, retry, game over and the staff roll are in.
