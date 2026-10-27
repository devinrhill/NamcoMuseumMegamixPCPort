# Jobs wired into the native build

The game is a chain of "jobs" (screens / modes). A job number is `index << 20`, so job 137 is `0x08900000`.
Any job in this table can be entered directly with `./build/megamix --job HEX` (see OPTIONS.md).
Jobs that are not listed here are not ported yet; entering one (with `--job`, a debug menu row or a button that leads there) leaves an empty
screen: nothing runs any more and the game has to be restarted.

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
| 0 | `0x00000000` | developers' root menu ("Kingdom Final Sep 30 2010 21:51:53") | 0x800c25c0 | works | KINGDOM = job 128 (the normal boot); the six LABO rows (jobs 16..96) are not in the retail job table: empty screen, as on the Wii |
| 128 | `0x08000000` | system start (creates `cKingdomSys`, attaches `resident.arc`) | 0x8000e3bc | works | first job, goes to 130 |
| 129 | `0x08100000` | kingdom end (releases `cKingdomSys`, then job 0) | 0x8000e3e8 | works | nothing in retail asks for it; ends in the root menu (0) |
| 130 | `0x08200000` | hand-off to boot | 0x8000e414 | works | `--job` / `--next` hook in here |
| 131 | `0x08300000` | debug top menu (`cAppDebugTopMenu`) | 0x8000e1f8 | works | up/down deliberately swapped |
| 132 | `0x08400000` | boot splash screens (`cAppBoot`) | 0x8009f7a0 | works | |
| 133 | `0x08500000` | save / load, forced (`cAppSaveLoad(true)`) | 0x8000e420 | works | |
| 134 | `0x08600000` | save / load (`cAppSaveLoad(false)`) | 0x8000e5dc | works | |
| 136 | `0x08800000` | attract movie (`cAppAttract`) | 0x8019a864 | works | retail never enters it by itself |
| 137 | `0x08900000` | title screen (`cAppTitle`) | 0x8019a98c | works | Pac-Man's shadow missing |
| 138 | `0x08a00000` | main menu hub (`g03::cAppMenu`, castle garden) | 0x8019ab28 | partial | every game door works both ways (you come back in front of the game's building); door 7 enters the classic arcade building (140); the centre stairs are green (grey stone in the original) |
| 139 | `0x08b00000` | the collection's staff roll (`menu::cAppG10StaffRoll`) | 0x8019b250 | works | not compared with the original; then the title (137); nothing in retail's job flow asks for it |
| 140 | `0x08c00000` | classic arcade building (`g03::cAppMenu`, the hall of cabinets) | 0x8019aee4 | partial | walk to a cabinet: its panel shows the name, the HIGH SCORE from the save and the description; A starts the game (141 -> 142) and you come back in front of that cabinet; walking out of the door returns to the hub (138); only PAC-MAN runs (the other cabinets return at once); not compared with the original |
| 141 | `0x08d00000` | arcade game start (remembers the job to return to) | 0x800c20c0 | works | -> 142 |
| 142 | `0x08e00000` | arcade game host (`rso::cAppRSOOriginal`) | 0x800c2108 | partial | runs the chosen classic game; only PAC-MAN (index 9) is ported, the others return at once; the game's own menus work (A / PLUS: START menu with OPTIONS between games, PAUSE menu during one; HOME = leave the game); the screen adjustment menu is not ported |
| 143 | `0x08f00000` | arcade game end | 0x800c2230 | works | back to the job before 141 (through the save screen) |
| 144 | `0x09000000` | debug LOAD | 0x8000e79c | works | back to the previous job (debug top menu row) |
| 145 | `0x09100000` | debug SAVE | 0x8000e7d0 | works | enables the save manager, back to the previous job |
| 146 | `0x09200000` | debug DELETE | 0x8000e81c | works | deletes the save files, clears the 7 game blocks, disables saving, then the debug top menu (131) |
| 147 | `0x09300000` | debug WII MENU (`OSReturnToMenu`) | 0x8000e814 | works | ends the program (the Wii returns to its system menu) |

## g00 dummy game (jobs 160-175)

The developers' placeholder game, drawn only with the engine's debug text. Ported by hand from `main.dol` (no C#). Not in the retail
debug menu: start it with `--job 0x0a200000` (menu) or `--job 0x0a300000` (game).

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 160 | `0x0a000000` | g00 start (1-byte sys) | 0x800c6fc0 | works | asks for 161, which has no init in the DOL (nothing runs) |
| 162 | `0x0a200000` | g00 menu (`cAppG00DummyMenu`) | 0x800c706c | works | FADE IN, "[Surprise!]" / "[Halo]" pages (A next, B skip), NORM SELECT (A), "Do You Begin?" (A / B), then this menu again as in the DOL; 2 = leave (175) |
| 163 | `0x0a300000` | g00 game (`cAppG00DummyGame`) | 0x800c7194 | works | shoot the flying "\(^o^)/" with B at the pointer (A held + B repeats); ends after 1800 frames, 1000 points / shots or with 1 held, then 175 |
| 174 | `0x0ae00000` | RSO debug menu (`rso::cAppRSODebugMenu`, "RSO Release") | 0x800c1f98 | works | rows RSO 00..17 = the arcade game index (00..08 vol 1, 09..17 vol 2; 09 = PAC-MAN) -> 141, the save-data rows, EXIT = 131 |
| 175 | `0x0af00000` | g00 end (deletes the sys) | 0x800c7010 | works | back to the debug menu (131) |

## Galaga Remix (g01, jobs 176-191)

Ported by hand from `main.dol` (no C#). Single player is complete; multiplayer is partial.

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 176 | `0x0b000000` | Galaga start (creates `cG01GalagaSys`: resident archive, save record, score table) | 0x800b0cf4 | works | goes to 178 |
| 177 | `0x0b100000` | Galaga debug menu (`cAppGalagaDebugMenu`) | 0x800b0c8c | works | MENU row: left / right = player count 1..4; STAGE row: left / right = stage 1..10; MENU = job 178, STAGE = the stage (179) with the chosen stage and player count, EXIT = 191 |
| 178 | `0x0b200000` | Galaga stage select (`cAppG01GalagaMenu` + `cTaskConsMenuMan`) | 0x800b0d98 | partial | worlds / stages / Back / Rule / Multi consoles (player panels with names); 3D scene root + effect task not created (nothing on the screen uses them); a stage leads to 184 (Nunchuk card) -> 179, MULTI to 182; BACK leads to 191 |
| 179 | `0x0b300000` | Galaga stage (`cAppG01GalagaGame`; stage 10: `cAppG01GalagaBoss`) | 0x800b0ed4 | works (single player; multiplayer 2..4 players with separate pointers, not compared with a Wii) | stages 1..9: opening / goal demos, course camera, Pac-Man (hit / miss / goal run), enemies, bombs, rescued fighters + cookies, gates, spikes / spike masters, comets, shooting stars, lens flare, shooting (A / B + pointer), missiles, swing jump, READY / START, score / life panels, pause (HOME or MINUS: return to game / stage select = 178), "Play again?" (YES = 179, NO = 181), result screen; stage 9 leads to stage 10. Stage 10: the last boss with its enemies, demos and goal, then the staff roll (183). Multiplayer: per-player consoles, the multi result board (kills / defense / hit rate, crown) and the multi record save |
| 180 | `0x0b400000` | Galaga debug select (empty slot) | 0x800b0ce8 | works | empty in the DOL too: nothing runs (empty screen) |
| 181 | `0x0b500000` | Galaga game over (`cAppG01GalagaGameOver`) | 0x800b1250 | works | ANT_GAMEOVER, then the stage select (178); reached by NO in the stage's "Play again?" |
| 182 | `0x0b600000` | Galaga player matching (`cAppG01GalagaMatching`) | 0x800b13d0 | partial | shared matching menu with Galaga's layouts: player count, connection status (only connected Wii Remotes count: Wiimote 1 always, Wiimote N when gamepad N is plugged in or with `--pads`), the "Please add additional Wii Remote(s)." window when too few are connected; back to 178 with the chosen players (data mapping UNVERIFIED). ADD (pairing a new remote) has nothing to pair on a PC |
| 183 | `0x0b700000` | Galaga staff roll (`cAppG01GalagaStaffRoll`) | 0x800b150c | partial | shared staff roll with m01bgm_Ending; reached from the stage-10 boss clear (23rd session: GAME -> GOAL -> CLEAR -> EXIT verified headless) |
| 184 | `0x0b800000` | Galaga Nunchuk card (`cAppG01GalagaNunchuk`) | 0x800b1658 | works | ANT_NUNCHUK card, then 179; skipped when every player's remote has a Nunchuk |
| 191 | `0x0bf00000` | Galaga end (deletes `cG01GalagaSys`) | 0x800b0d40 | works | back to the hub (138) |

## Rally-X Remix (g02, jobs 192-207)

Ported by hand from `main.dol` (no C#). Single player is playable; multiplayer (199 / 200) is not ported. Nothing here was compared with the
original in Dolphin yet.

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 192 | `0x0c000000` | Rally-X start (creates the Rally-X sys: resident archive, save record) | 0x800c6444 | partial | goes to 194; the sys' SE group is not loaded |
| 193 | `0x0c100000` | Rally-X debug menu (`cAppRallyXDebugMenu`, native: the DOL slot is an empty stub) | (0x800c6430 stub) | works | like the Pac-Motos one: MENU / MULTI MENU / MATCHING / STAGE / GAMEOVER / STAFFROLL / EXIT; left / right on STAGE pick the stage (`w01s01`..`w01s05`, `w01red`, ..., `vs01`..) |
| 194 | `0x0c200000` | Rally-X stage select (`cAppG02RallyXMenu` + `cTaskConsMenuMan`) | 0x800c6504 | partial | world / stage buttons, best score / time board, world arrows, Back (-> 207), HOW TO PLAY (remote-only or Nunchuk pages), control type button; A on an open stage -> 201. The 3D world map of the pointed stage and Pac-Man's car are shown. MULTI PLAYER leads to the matching (199) and the multiplayer menu (200) |
| 195 | `0x0c300000` | Rally-X stage (`cAppG02RallyXGame`) | 0x800c677c | partial | single player game flow (opening camera turn, start signal, play, miss -> retry with lives, game over "Play again?", stage clear with fuel bonus + record save, boss stages), player car (Pac-Man) and enemy cars incl. both red modes, car camera, console (radar, fuel, score, flags, time), pause; flags (numbered / lucky / special / drop flag), items, stones / oil / spikes / mud / smoke, effects. Multiplayer (battle stages 25..28, 2..4 players from the matching and the menu 200): split screen (two stacked views for 2 players, quarters for 3 / 4), per player fuel / smoke / mud / number plate, flag counts with the leader's crown, a countdown timer, the item pick-ups that respawn (fuel, paint, smoke, armor), FINISH, winner / draw screens (back to 200). Not ported: Mii heads |
| 196 | `0x0c400000` | Rally-X debug select (empty slot) | 0x800c6438 | works | empty in the DOL too: nothing runs (empty screen) |
| 198 | `0x0c600000` | Rally-X game over (`cAppG02RallyXGameOver`) | 0x800c68c4 | partial (not run yet) | ANITA_GAMEOVER, then the stage select (194) |
| 199 | `0x0c700000` | Rally-X player matching (`cAppG02RallyXMatching`) | 0x800c6a44 | works | the shared matching screen (guest names, no Mii faces); then the multiplayer menu (200) |
| 200 | `0x0c800000` | Rally-X multiplayer menu (`cAppG02RallyXMultiMenu` + `cTaskConsMultiMenuMan`) | 0x800c6640 | works (scripted run) | per-player slots (number plate with the last winner's crown, name, control type button, coloured car), stage / flag count / time arrows with the 3D map of the stage, START (-> 201 -> 195), Single player (-> 194), Back (-> 207), HOW TO PLAY. No Mii heads on the cars; the control type button of players 2..4 is changed with A / B / Z of their own remote |
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

Ported by hand from `main.dol` (no C#). Single player and multiplayer are complete except the Mii icons.

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 224 | `0x0e000000` | Gator Panic start (creates the sys: three resident archives, the player data) | 0x8019a07c | works | goes to 231 |
| 225 | `0x0e100000` | Gator Panic debug menu (`cAppG04WaniWaniDebugMenu`) | 0x80199f64 | works | GAME (227), GAME_MULTI (230, the 4 player game), EXIT (239); only through `--job 0x0e100000` |
| 227 | `0x0e300000` | the game (`cAppG04WaniWaniGame`) | 0x8019a13c | works | menu (back / how to play / start / players), tutorial, READY / GO, the 2-minute game: gators from the CSV_ENEMYnn scripts, Pac-Man aimed with the Nunchuk stick and swung with the Wii Remote (Shake), hits / bites / combos, the angry event, birds, gator shadows, lens flare, time up, result with the tortoise, "Play again?", the high score saved; multiplayer with names, per-player consoles and the result board. Mii icons not ported |
| 229 | `0x0e500000` | Gator Panic matching (`cAppG04WaniWaniMatching`) | 0x8019a470 | works (shared matching base; data copy field mapping UNVERIFIED) | player entry; confirm -> job 231 with sys players > 1, cancel -> 1 player |
| 230 | `0x0e600000` | the 4 player game (`cAppG04WaniWaniGame` started with 4 players) | 0x8019a2a0 | works (headless only) | the debug menu's GAME_MULTI row |
| 231 | `0x0e700000` | "connect the Nunchuk" card (`cAppG04WaniWaniNunchuk`) | 0x8019a5ac | works | COMMON_ANITA_NUNCHUK; skipped when every player's remote has a Nunchuk (always on PC), then 227 |
| 239 | `0x0ef00000` | Gator Panic end (deletes the sys) | 0x8019a0e0 | works | back to the hub (138) |

## Pac-Motos Remix (g07, jobs 272-287)

Converted from the C# port and checked against `main.dol`. Every Pac-Motos job of the DOL's table is wired (281-285 do not exist).

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 272 | `0x11000000` | Pac-Motos start (creates `cG07MotosSys`) | 0x800a2b40 | works | goes to 274 |
| 273 | `0x11100000` | Pac-Motos debug menu (`cAppMotosDebugMenu`) | 0x800a29bc | works | STAGE row: left / right pick any of the 46 stages (`w01s01`.. / `vs01`..) |
| 274 | `0x11200000` | stage select (`cAppG07MotosMenu`) | 0x800a2ba0 | works | pointer cursor; A on a stage -> 280 -> 275; MULTIPLAYER -> 278; RETURN goes back to the hub (via 287) |
| 275 | `0x11300000` | the game (`cAppG07MotosGame`) | 0x800a2e18 | partial | stage, panels, enemies, Pac-Man movement and falling work; a miss restarts the job with one life less (4 lives), at 0 lives "Play again?" (yes = restart, no = 277); HUD (score, timer, stage, lives, items), item select and READY drawn; the boss intro text was Unity UI (not drawn); multiplayer battles (2..4 players, all 10 battle stages) run through rounds to the DRAW / winner screen with the players' names (no Mii faces); the board is see-through (opaque in the original) |
| 276 | `0x11400000` | DEBUG_SELECT | 0x800a2b28 | pass-through | straight into the game (275) with the debug menu's stage, as in the DOL |
| 277 | `0x11500000` | game over (`cAppG07MotosGameOver`) | 0x800a2f9c | works | GAME OVER screen, then back to the stage select (274) |
| 278 | `0x11600000` | multiplayer matching (`cAppG07MotosMatching`) | 0x800a3268 | partial | player count + connection status (only connected Wii Remotes count, see 182); A goes on. The orange title band is empty (Unity text) |
| 279 | `0x11700000` | multiplayer menu (`cAppG07MotosMultiMenu`) | 0x800a2cdc | works | battle stage, round / time / item settings, one pointer per player; START -> 280 |
| 280 | `0x11800000` | "connect the Nunchuk" card (`cAppG07MotosNunchuk`) | 0x800a33a4 | works | only without a Nunchuk (`--no-nunchuk`); multiplayer checks every player's pad; then 275 |
| 286 | `0x11e00000` | staff roll (`cAppG07MotosStaffRoll`) | 0x800a311c | works | shared credits (`staffroll.lzs`) with the Motos ending BGM; then 277 |
| 287 | `0x11f00000` | Pac-Motos end (deletes the sys) | 0x800a2b70 | works | back to the hub (138), in front of the Pac-Motos building |

## Grobda Remix (g11, jobs 336-351)

Ported by hand from `main.dol` (no C#). Single player is complete (not compared with Dolphin); multiplayer is partial.

| Job | Hex | What it is | DOL init | Status | Notes |
|---|---|---|---|---|---|
| 336 | `0x15000000` | Grobda start (creates the Grobda sys: resident archive) | 0x8000f0c8 | partial | goes to 338 |
| 337 | `0x15100000` | Grobda debug menu (`cAppGrobdaDebugMenu`) | 0x8000ef44 | works | MENU / MODE (player count) / STAGE (stage index; via 340 to the stage) / GAMEOVER / STAFFROLL / EXIT; only `--job 0x15100000` reaches it |
| 338 | `0x15200000` | Grobda menu (`cAppG11GrobdaMenu`) | 0x8000f164 | works (single player) | START (-> 344) / RULE (HOW TO PLAY) / MULTI (-> 342) / BACK (-> 351), best score panel, the player's tank on the menu stage; multiplayer panels / player icons not ported |
| 342 | `0x15600000` | Grobda matching (`cAppG11GrobdaMatching`) | 0x8000fa64 | works (shared `cAppGameBaseMenuMatching`; sys data copy reduced to player count, UNVERIFIED) | player entry; confirm -> menu 338 in multiplayer mode, cancel -> 1 player |
| 341 | `0x15500000` | Grobda game over (`cAppG11GrobdaGameOver`) | 0x8000f380 | works | BIN_ANT_GAMEOVER -> menu 338; only the debug menu reaches it |
| 344 | `0x15800000` | Grobda Nunchuk card (`cAppG11GrobdaNunchuk`) | 0x8000f804 | works | always skipped (the DOL's IsSkip returns 1) -> 339 |
| 339 | `0x15300000` | Grobda stage (`cAppG11GrobdaGame`) | 0x8000f338 | works (single player; multiplayer with the WIN / DRAW result board and player icons, no Mii faces; all six enemy kinds ported, Fortres/Frossor UNVERIFIED vs Dolphin) | the arena, the console (score, timer, high score, energy gauge, combo banner), READY / GO, 2:00 timer, the player's tank (stick / D-pad, laser, shield (2 / A), grenade (swing), energy, destroyed + respawn), purple / orange / yellow tanks in waves chosen by score, explosions with combos, wrecks, pause (HOME / MINUS), record save on the stage end (-> job 338); green / brown Frossors and the Fortres |
| 340 | `0x15400000` | Grobda debug select (pass-through) | 0x8000f0b0 | pass-through | -> the stage (339); reached by the debug menu's MODE / STAGE rows |
| 350 | `0x15e00000` | Grobda staff roll (`cAppG11GrobdaStaffRoll`) | 0x8000f618 | works | -> game over 341; only the debug menu reaches it |
| 351 | `0x15f00000` | Grobda end (deletes the sys) | 0x8000f10c | works | back to the hub (138) |
