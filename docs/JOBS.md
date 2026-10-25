# Jobs wired into the native build

The game is a chain of "jobs" (screens / modes). A job number is `index << 20`, so job 137 is `0x08900000`.
Any job in this table can be entered directly with `./build/megamix --job HEX` (see OPTIONS.md).
Jobs that are not listed here are not ported yet; entering one does nothing (or falls back to the debug menu).

The table lives in `src/platform/boot.cpp`. **Whenever a job is added there, add it here too.**

Sections are in job-number order, one per game:
[Boot, title, menus](#boot-title-menus) (128-140) ·
[g00 dummy game](#g00-dummy-game-jobs-160-175) (160-175) ·
[Galaga Remix](#galaga-remix-g01-jobs-176-191) (176-191) ·
[Rally-X Remix](#rally-x-remix-g02-jobs-192-207) (192-207) ·
[Pac-N-Roll Remix](#pac-n-roll-remix-g03-jobs-208-223) (208-223) ·
[Gator Panic Remix](#gator-panic-remix-g04-jobs-224-239) (224-239) ·
[Pac-Motos Remix](#pac-motos-remix-g07-jobs-272-287) (272-287) ·
[Grobda Remix](#grobda-remix-g11-jobs-336-351) (336-351).

Status: **works** = runs and was checked on screen; **partial** = runs, with known gaps (see the Notes column); **pass-through** = only forwards to the next job (as the DOL does); **stub** = the screen is not ported yet, the job forwards to the next one.

## Boot, title, menus

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 128 | `0x08000000` | system start (creates `cKingdomSys`, attaches `resident.arc`) | 0x8000e3bc | works | first job, goes to 130 |
| 130 | `0x08200000` | hand-off to boot | 0x8000e414 | works | `--job` / `--next` hook in here |
| 131 | `0x08300000` | debug top menu (`cAppDebugTopMenu`) | 0x8000e1f8 | partial | rows without a ported job do nothing; up/down deliberately swapped |
| 132 | `0x08400000` | boot splash screens (`cAppBoot`) | 0x8009f7a0 | works | |
| 133 | `0x08500000` | save / load, forced (`cAppSaveLoad(true)`) | 0x8000e420 | works | |
| 134 | `0x08600000` | save / load (`cAppSaveLoad(false)`) | 0x8000e5dc | works | |
| 136 | `0x08800000` | attract movie (`cAppAttract`) | 0x8019a864 | works | retail never enters it by itself |
| 137 | `0x08900000` | title screen (`cAppTitle`) | 0x8019a98c | works | wind particles / Pac-Man shadow missing |
| 138 | `0x08a00000` | main menu hub (`g03::cAppMenu`, castle garden) | 0x8019ab28 | partial | Pac-N-Roll and Pac-Motos doors work both ways (you come back in front of the game's building); classic menu (140) and the other games not ported |

## g00 dummy game (jobs 160-175)

The developers' placeholder game, drawn only with the engine's debug text. Ported by hand from `main.dol` (no C#). Not in the retail
debug menu: start it with `--job 0x0a200000` (menu) or `--job 0x0a300000` (game).

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 160 | `0x0a000000` | g00 start (1-byte sys) | 0x800c6fc0 | works | asks for 161, which has no init in the DOL (nothing runs) |
| 162 | `0x0a200000` | g00 menu (`cAppG00DummyMenu`) | 0x800c706c | works | FADE IN, "[Surprise!]" / "[Halo]" pages (A next, B skip), NORM SELECT (A), "Do You Begin?" (A / B), then this menu again as in the DOL; 2 = leave (175) |
| 163 | `0x0a300000` | g00 game (`cAppG00DummyGame`) | 0x800c7194 | works | shoot the flying "\(^o^)/" with B at the pointer (A held + B repeats); ends after 1800 frames, 1000 points / shots or with 1 held, then 175 |
| 175 | `0x0af00000` | g00 end (deletes the sys) | 0x800c7010 | works | back to the debug menu (131) |

## Galaga Remix (g01, jobs 176-191)

Ported by hand from `main.dol` (no C#). Single player is complete; multiplayer is partial.

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 176 | `0x0b000000` | Galaga start (creates `cG01GalagaSys`: resident archive, save record, score table) | 0x800b0cf4 | works | goes to 178 |
| 177 | `0x0b100000` | Galaga debug menu (`cAppGalagaDebugMenu`) | 0x800b0c8c | works | MENU row: left / right = player count 1..4; STAGE row: left / right = stage 1..10; MENU = job 178; STAGE job not ported yet |
| 178 | `0x0b200000` | Galaga stage select (`cAppG01GalagaMenu` + `cTaskConsMenuMan`) | 0x800b0d98 | partial | worlds / stages / Back / Rule / Multi consoles (player panels with names); 3D scene root + effect task not created (nothing on the screen uses them); the jobs it leads to (0x0b800000 game, 0x0b600000 multi, 0x0bf00000 back) are not ported yet |
| 179 | `0x0b300000` | Galaga stage (`cAppG01GalagaGame`; stage 10: `cAppG01GalagaBoss`) | 0x800b0ed4 | works (single player; multiplayer result board ported, multiplayer play UNVERIFIED) | stages 1..9: opening / goal demos, course camera, Pac-Man (hit / miss / goal run), enemies, bombs, rescued fighters + cookies, gates, spikes / spike masters, comets, shooting stars, lens flare, shooting (A / B + pointer), missiles, swing jump, READY / START, score / life panels, pause (HOME or MINUS: return to game / stage select = 178), "Play again?" (YES = 179, NO = 181), result screen; stage 9 leads to stage 10. Stage 10: the last boss with its enemies, demos and goal, then the staff roll (183). Multiplayer: per-player consoles, the multi result board (kills / defense / hit rate, crown) and the multi record save |
| 181 | `0x0b500000` | Galaga game over (`cAppG01GalagaGameOver`) | 0x800b1250 | works | ANT_GAMEOVER, then the stage select (178); reached by NO in the stage's "Play again?" |
| 182 | `0x0b600000` | Galaga player matching (`cAppG01GalagaMatching`) | 0x800b13d0 | partial | shared matching menu with Galaga's layouts; back to 178 with the chosen players (data mapping UNVERIFIED) |
| 183 | `0x0b700000` | Galaga staff roll (`cAppG01GalagaStaffRoll`) | 0x800b150c | partial | shared staff roll with m01bgm_Ending; reached from the stage-10 boss clear (23rd session: GAME -> GOAL -> CLEAR -> EXIT verified headless) |
| 184 | `0x0b800000` | Galaga Nunchuk card (`cAppG01GalagaNunchuk`) | 0x800b1658 | works | ANT_NUNCHUK card, then 179; skipped when every player's remote has a Nunchuk |

## Rally-X Remix (g02, jobs 192-207)

Ported by hand from `main.dol` (no C#); started (stage shown, no gameplay yet).

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 192 | `0x0c000000` | Rally-X start (creates the Rally-X sys: resident archive, save record) | 0x800c6444 | partial | goes to 194; the sys' SE group is not loaded |
| 193 | `0x0c100000` | Rally-X debug menu (`cAppRallyXDebugMenu`, native: the DOL slot is an empty stub) | (0x800c6430 stub) | works | like the Pac-Motos one: MENU / MULTI MENU / MATCHING / STAGE / GAMEOVER / STAFFROLL / EXIT; left / right on STAGE pick the stage (`w01s01`..`w01s05`, `w01red`, ..., `vs01`..) |
| 194 | `0x0c200000` | Rally-X stage select (`cAppG02RallyXMenu` + `cTaskConsMenuMan`) | 0x800c6504 | partial (not run yet) | world / stage buttons, best score / time board, world arrows, Back (-> 207), HOW TO PLAY (remote-only or Nunchuk pages), control type button, the 3D world map + Pac-Man's car; A on an open stage -> 201. MULTI PLAYER leads to 199, which is not ported |
| 195 | `0x0c300000` | Rally-X stage (`cAppG02RallyXGame`) | 0x800c677c | partial | single player game flow (opening camera turn, start signal, play, miss -> retry with lives, game over "Play again?", stage clear with fuel bonus + record save), player car (Pac-Man) and enemy cars driving, car camera, console (radar, fuel, score, flags, time), pause. Flags / items / stones / oil / mud / smoke are shown but not yet touched by the cars (Wave 2); no effects; multiplayer not ported |
| 198 | `0x0c600000` | Rally-X game over (`cAppG02RallyXGameOver`) | 0x800c68c4 | partial (not run yet) | ANITA_GAMEOVER, then the stage select (194) |
| 199 | `0x0c700000` | Rally-X player matching | 0x800c6a44 | not ported | |
| 200 | `0x0c800000` | Rally-X multiplayer menu | 0x800c6640 | not ported | |
| 201 | `0x0c900000` | Rally-X Nunchuk card (`cAppG02RallyXNunchuk`) | 0x800c6ccc | partial (not run yet) | COMMON_ANITA_NUNCHUK, then the stage (195); skipped for the remote-only control type or when a Nunchuk is in |
| 206 | `0x0ce00000` | Rally-X staff roll (`cAppG02RallyXStaffRoll`) | 0x800c6b80 | partial (not run yet) | shared staff roll, then the game over (198); its BGM m02bgm_Ending is not ported (silent) |
| 207 | `0x0cf00000` | Rally-X end (deletes the sys) | 0x800c64a8 | works | back to the hub (138) |

## Pac-N-Roll Remix (g03, jobs 208-223)

Converted from the C# port and checked against `main.dol`.

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 208 | `0x0d000000` | Pac-N-Roll start (creates `cG03PacnRollSys`) | 0x8012b18c | works | goes to 210 |
| 209 | `0x0d100000` | Pac-N-Roll debug menu | 0x8012b008 | works | STAGE row picks the round |
| 210 | `0x0d200000` | stage select (`cAppG03PacnRollMenu`) | 0x8012b378 | works | RETURN goes back to the hub (via 223) |
| 211 | `0x0d300000` | the game (`cAppG03PacnRollGame`) | 0x8012b4b4 | partial | `--round N`; gaps listed in OPTIONS.md section 9 |
| 212 | `0x0d400000` | DEBUG_SELECT | 0x8012b174 | pass-through | straight into the game (211) with the round the Pac-N-Roll system holds (the debug menu's STAGE row), as in the DOL |
| 213 | `0x0d500000` | game over (`cAppG03PacnRollGameOver`) | 0x8012b698 | works | |
| 214 | `0x0d600000` | EDIT (developer stage editor slot, empty in retail) | 0x8012b174 | pass-through | straight into the game (211), as in the DOL |
| 215 | `0x0d700000` | EDIT2 (same) | 0x8012b174 | pass-through | straight into the game (211), as in the DOL |
| 216 | `0x0d800000` | "connect the Nunchuk" card (`cAppG03PacnRollNunchuk`) | 0x8012b964 | works | shown for 3 s (HOME pauses the clock) only when Wiimote 1 has no Nunchuk: `--no-nunchuk`; otherwise skipped, as on a Wii with a Nunchuk. Then 211 |
| 217 | `0x0d900000` | unnamed (empty in retail) | 0x8012b174 | pass-through | straight into the game (211), as in the DOL |
| 222 | `0x0de00000` | staff roll (`cAppG03PacnRollStaffRoll`) | 0x8012b818 | works | after the last stage; credits text + screenshots + the 3D Pac-Man chase strip (DOL camera: orthographic from above; look UNVERIFIED), any button but the D-pad / HOME skips, HOME pauses; then 213 |
| 223 | `0x0df00000` | Pac-N-Roll end (deletes the sys) | 0x8012b21c | works | back to the hub (138) |

Jobs 218-221 (`0x0da00000`..`0x0dd00000`) are not in the DOL's job table.

## Gator Panic Remix (g04, jobs 224-239)

Ported by hand from `main.dol` (no C#); started in the 25th session: the island is shown, no gameplay yet.

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 224 | `0x0e000000` | Gator Panic start (creates the sys: three resident archives, the player data) | 0x8019a07c | works | goes to 231 |
| 225 | `0x0e100000` | Gator Panic debug menu (`cAppG04WaniWaniDebugMenu`) | 0x80199f64 | works | GAME (227), GAME_MULTI (230, the 4 player game), EXIT (239); only through `--job 0x0e100000` |
| 227 | `0x0e300000` | the game (`cAppG04WaniWaniGame`) | 0x8019a13c | works (alone) | menu (back / how to play / start / players), READY / GO, the 2-minute game: gators from the CSV_ENEMYnn scripts, Pac-Man aimed with the Nunchuk stick and swung with the Wii Remote (Shake), hits / bites / combos, the angry event, time up, result with the tortoise, "Play again?", the high score saved. Multiplayer, lens flare, birds, gator shadows not ported |
| 231 | `0x0e700000` | "connect the Nunchuk" card (`cAppG04WaniWaniNunchuk`) | 0x8019a5ac | works | COMMON_ANITA_NUNCHUK; skipped when every player's remote has a Nunchuk (always on PC), then 227 |
| 239 | `0x0ef00000` | Gator Panic end (deletes the sys) | 0x8019a0e0 | works | back to the hub (138) |
| 229 | `0x0e500000` | Gator Panic matching (`cAppG04WaniWaniMatching`) | 0x8019a470 | works (shared matching base; data copy field mapping UNVERIFIED) | player entry; confirm -> job 231 with sys players > 1, cancel -> 1 player |

Not wired yet: 225 debug menu (`0x0e100000`), 230 multiplayer game (`0x0e600000`).

## Pac-Motos Remix (g07, jobs 272-287)

Converted from the C# port and checked against `main.dol`. Every Pac-Motos job of the DOL's table is wired (281-285 do not exist).

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 272 | `0x11000000` | Pac-Motos start (creates `cG07MotosSys`) | 0x800a2b40 | works | goes to 274 |
| 273 | `0x11100000` | Pac-Motos debug menu (`cAppMotosDebugMenu`) | 0x800a29bc | works | STAGE row: left / right pick any of the 46 stages (`w01s01`.. / `vs01`..) |
| 274 | `0x11200000` | stage select (`cAppG07MotosMenu`) | 0x800a2ba0 | works | pointer cursor; A on a stage -> 280 -> 275; MULTIPLAYER -> 278; RETURN goes back to the hub (via 287) |
| 275 | `0x11300000` | the game (`cAppG07MotosGame`) | 0x800a2e18 | partial | stage, panels, enemies, Pac-Man movement and falling work; a miss restarts the job with one life less (4 lives), at 0 lives "Play again?" (yes = restart, no = 277); HUD (score, timer, stage, lives, items), item select and READY drawn; the boss intro text was Unity UI (not drawn) |
| 276 | `0x11400000` | DEBUG_SELECT | 0x800a2b28 | pass-through | straight into the game (275) with the debug menu's stage, as in the DOL |
| 277 | `0x11500000` | game over (`cAppG07MotosGameOver`) | 0x800a2f9c | works | GAME OVER screen, then back to the stage select (274) |
| 278 | `0x11600000` | multiplayer matching (`cAppG07MotosMatching`) | 0x800a3268 | partial | player count + connection status; A goes on (keyboard / test hooks act as up to 4 Wiimotes). The orange title band is empty (Unity text) |
| 279 | `0x11700000` | multiplayer menu (`cAppG07MotosMultiMenu`) | 0x800a2cdc | works | battle stage, round / time / item settings, one pointer per player; START -> 280 |
| 280 | `0x11800000` | "connect the Nunchuk" card (`cAppG07MotosNunchuk`) | 0x800a33a4 | works | only without a Nunchuk (`--no-nunchuk`); multiplayer checks every player's pad; then 275 |
| 286 | `0x11e00000` | staff roll (`cAppG07MotosStaffRoll`) | 0x800a311c | works | shared credits (`staffroll.lzs`) with the Motos ending BGM; then 277 |
| 287 | `0x11f00000` | Pac-Motos end (deletes the sys) | 0x800a2b70 | works | back to the hub (138), in front of the Pac-Motos building |

## Grobda Remix (g11, jobs 336-351)

Ported by hand from `main.dol` (no C#); started (arena and tank driving, no game flow yet).

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 336 | `0x15000000` | Grobda start (creates the Grobda sys: resident archive) | 0x8000f0c8 | partial | goes to 338 |
| 337 | `0x15100000` | Grobda debug menu (`cAppGrobdaDebugMenu`) | 0x8000ef44 | works | MENU / MODE (player count) / STAGE (stage index; starts the stage here, job 340 is empty in the DOL) / GAMEOVER / STAFFROLL / EXIT; only `--job 0x15100000` reaches it |
| 338 | `0x15200000` | Grobda menu (`cAppG11GrobdaMenu`) | 0x8000f164 | works (single player) | START (-> 344) / RULE (HOW TO PLAY) / MULTI (-> 342, not ported) / BACK (-> 351), best score panel, the player's tank on the menu stage; multiplayer panels not ported |
| 342 | `0x15600000` | Grobda matching (`cAppG11GrobdaMatching`) | 0x8000fa64 | works (shared `cAppGameBaseMenuMatching`; sys data copy reduced to player count, UNVERIFIED) | player entry; confirm -> menu 338 in multiplayer mode, cancel -> 1 player |
| 341 | `0x15500000` | Grobda game over (`cAppG11GrobdaGameOver`) | 0x8000f380 | works | BIN_ANT_GAMEOVER -> menu 338; only the debug menu reaches it |
| 344 | `0x15800000` | Grobda Nunchuk card (`cAppG11GrobdaNunchuk`) | 0x8000f804 | works | always skipped (the DOL's IsSkip returns 1) -> 339 |
| 339 | `0x15300000` | Grobda stage (`cAppG11GrobdaGame`) | 0x8000f338 | works (single player; all six enemy kinds ported, Fortres/Frossor UNVERIFIED vs Dolphin) | the arena, the console (score, timer, high score, energy gauge, combo banner), READY / GO, 2:00 timer, the player's tank (stick / D-pad, laser, shield (2 / A), grenade (swing), energy, destroyed + respawn), purple / orange / yellow tanks in waves chosen by score, explosions with combos, wrecks, pause (HOME / MINUS), record save on the stage end (-> job 338); green / brown Frossors and the Fortres |
| 350 | `0x15e00000` | Grobda staff roll (`cAppG11GrobdaStaffRoll`) | 0x8000f618 | works | -> game over 341; only the debug menu reaches it |
| 351 | `0x15f00000` | Grobda end (deletes the sys) | 0x8000f10c | works | back to the hub (138) |
