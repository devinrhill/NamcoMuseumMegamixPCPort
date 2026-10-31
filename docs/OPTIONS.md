# Namco Museum Megamix (native port) - options

Every way to configure or steer the native build. **Keep this file up to date**: add a row whenever a command line flag, environment
variable, config key or key binding is added, renamed or removed, together with the `--help` text (`PrintHelp` in `src/platform/main.cpp`).

Where settings come from, in priority order: command line > environment > `megamix.cfg` / `input.cfg` > built-in defaults.

## 1. Files

| file | purpose |
|---|---|
| `megamix.cfg` (working directory) | paths, sound, language, see section 3; optional |
| `input.cfg` (working directory) | controller bindings, written with defaults on first run, see section 4 |
| `<save_dir>/museum.bin`, `banner.bin` | the game's save data (NAND `00010000/534e4d45/data` layout, checked against a real Dolphin save): every game's records live in museum.bin (7 blocks of 0x2000 bytes: Galaga, Rally-X, Pac-N-Roll, Gator Panic, Pac-Motos, Grobda, the hub / classic hall); banner.bin is built like the game builds it (title NAMCO MUSEUM MEGAMIX, the disc's banner and icons). Copied into a Dolphin user folder's `Wii/title/00010000/534e4d45/data/` the two files load in the real game |
| `~/.config/megamix/ui_theme.json` | the dev window's colours and font (`--devui`; written with the defaults on first use, `$MEGAMIX_UI_THEME` / `$XDG_CONFIG_HOME` move it). A colour is `"#rrggbb"`, `"#rrggbbaa"` or `[r, g, b, a]`; `"font"` is a .ttf path (empty = `resources/fonts/Verdana.ttf`, the font of unpac's editors, when it is in the working directory; `"raylib"` = raylib's pixel font; `resources/fonts/` also has VerdanaMono.ttf and Helvetica.ttf). Saved edits show within a second |
| `~/.cache/megamix/ui_scale` | the dev window's zoom (Ctrl + = / - / 0; `$XDG_CACHE_HOME` moves it) |
| disc files (`data_root`) | the extracted disc `files` folder: `museum/en/...`, `museum/resident.arc`, `museum/resource.lzs`, `museum/en/g10_menu/attract.thp`, `museum/en/sound/Wii_Wander.brsar` and `museum/en/sound/stream/*.brstm` |

## 2. Command line (`./build/megamix ...`)

| flag | meaning |
|---|---|
| `--job HEX` | start at this job instead of the boot chain (`--job 0` is job 0, the root debug menu), e.g. `--job 0x08300000` (debug menu), `--job 0x08900000` (title), `--job 0x08800000` (attract movie). Job number = table index << 20; every wired job is listed in JOBS.md |
| `--round N` / `--round wXsY` | Pac-N-Roll stage to play, as a number 0..25 (0 = World 1 Stage 1, 5 = W2S1, 9 = W3S1, 14 = W4S1, 20 = W5S1) or as a stage name, case-insensitive: `w5s1`, `w2s3`, or `wXsb` for a world's boss stage (its last stage: W2S4, W3S5, W4S6, W5S6; World 1 has none). Stages per world: 5, 4, 5, 6, 6. Used when the game job is entered directly (`--job 0x0d300000`; default 0). An invalid value prints a message and is ignored |
| `--next HEX` | job the save/load transition returns to when entered with `--job` (sets `cKingdomSys+0xa88`) |
| `--dump-tree [N]` | run N frames (default 400) without waiting for input, print the task tree, exit. Must be the first argument |
| `--no-nunchuk` | Wiimote 1 counts as having no Nunchuk, so Pac-N-Roll shows its "connect the Nunchuk" card (job `0x0d800000`, 3 seconds) before a stage; by default the PC always has a stick and the card is skipped, as on a Wii with a Nunchuk |
| `--pads N` | Wiimotes 1..N count as connected even without a gamepad (default 1: Wiimote 1 is always there). The multiplayer "add players" screens only offer remotes that are connected, as on the Wii: each gamepad connects one Wiimote (the second gamepad = Wiimote 2, ...); this switch lets a keyboard-only setup or a scripted run (`--press 300 2:a`) take part with more players |
| `--seed N` | Random seed the title screen uses instead of the clock (the original seeds from the clock each time the title starts). For scripted runs that must repeat exactly. Without it, runs that pass through the title differ from each other, as on the Wii |
| `--no-smooth` | turn off the edge smoothing (4x MSAA on polygon edges; text and textures are never filtered by it; on by default); same as `MEGAMIX_NOSMOOTH=1` |
| `--shot FRAME FILE` | save the window to a PNG at that frame; repeatable |
| `--stick FRAME X Y` | Nunchuk stick (-1..1, y up) from that frame on; repeatable (tests steering without a keyboard) |
| `--press FRAME BUTTON` | one-frame press of a Wiimote button: `a b 1 2 plus minus home left right up down z c shake` (physical directions); repeatable. Prefix `N:` presses on Wiimote N instead of 1 (`--press 300 2:a`: player 2 in multiplayer) |
| `--hold FRAME BUTTON` | hold a Wiimote button from that frame on (names as `--press` except `home`, including the `N:` Wiimote prefix; `up`/`down`/`left`/`right` are the Wiimote buttons) |
| `--point FRAME [N:]X Y` | put the pointer at game pixel X,Y (640x480, origin top-left) from that frame on; replaces the mouse; repeatable (the last entry whose FRAME has passed wins). Prefix `N:` on X sets Wiimote N's pointer instead (`--point 300 2:320 240`), replacing that player's right-stick pointer |
| `--devui` | developer switch (not part of the 1:1 game): open the dev window, a panel to the right of the game (the window widens by the panel's width so the game keeps its size, and narrows again when it is hidden or zoomed; F9 shows / hides it at any time); also `MEGAMIX_DEVUI=1`. Tabs: **Jobs** (the current / last job; every job of the master table filtered by game, the ones the port has no init for dimmed or hidden; **Go** (or a double click on a row) switches to the picked job at once, via the SAVE transition or via the LOAD fade, like `--job` it skips the games' INIT / TERM jobs), **Menu** (only while a game's menu runs: Galaga, Rally-X, Pac-N-Roll, Pac-Motos, Grobda; every stage with its lock state, **Unlock every stage** / Unlock / Lock one (in memory, the menu reloads to show it; **Write save** keeps them), **Play** or a double click starts a stage at once like the game's debug menu; Pac-Motos also has **Difficulty** Normal / Easy / Hard: single-player stages started afterwards use the disc's Easy / Hard parameters (enemies, player, meteors, axis attack, bosses), which the game itself never uses), **Editor** (only inside a Pac-N-Roll stage: a placement editor for the stage's items, boxes, gates, checkpoints, start, goal, fall / water / wind / camera areas, ghost areas, move points, ghosts and ghost routes. Pages Tools (pick what a click on the stage places), Selection (every thing in a list, its fields), Show (which kinds have handles), Delete, Check (problems such as a missing goal). In the game view: click a handle to select (Shift adds), drag to move it (Alt drag = up / down, Ctrl = on the grid), Del / X delete, Ctrl+Z / Ctrl+Y undo / redo, Ctrl+D duplicate, R turns 15 degrees (Shift back), PgUp / PgDn raise / lower, Esc deselects, Enter applies; the game gets no mouse / keys while the pointer is over the game (untick **Edit in the game view** to play). **No title card** (the same switch as `--g03-skip-title`: a restarted stage starts without its stage name card) and **Hide the HUD** (the stage's consoles are not drawn; the pause menu still is). **Draw everything** (default on, while the Editor tab is shown: every item / gimmick draws instead of only the ones inside the game camera's view volume). A **test start** (Tools page; editor only, never saved: the Test start tool, Pac-Man's place or the selected thing) starts Pac-Man there in the edited round; **Restart** restarts the stage with it (it wins over `--g03-start`). **Editor camera**: right drag turns, middle drag (or a left drag that does not start on a handle, for trackpads) pans, the wheel zooms (the far clip is pushed out while it is on). **Apply + restart** restarts the stage with the edits (only that round reads them, until **Back to the disc** or the end of the run); **Save** / **Load** keep a round's edited files in `$MEGAMIX_G03_EDITS/roundNN/` (default `~/.local/share/megamix/g03_edits`; the graybox primitives go to `graybox.json` there); nothing is written to the disc data). **Which round**: the editor holds the data of the round the game is *running* (the one its stage was built from, not the sys's copy, which menus and retries change) and reads it from that round's own stage archive, whatever other archives are attached. *data from* + **Load it** pick another round's data; **The game's round** goes back; *Follow it* switches to the game's round whenever it changes (off after you pick one); the edits of the round you leave are kept and come back when you return to it (**Back to the disc** drops them). *Apply plays* is the round **Apply + restart** (and Restart) start with the edited files (default: the data's own round; pick another to copy one stage's layout into a different stage; the camera areas file is renamed for it). The **Graybox** page builds a stage out of primitives, for hit boxes only: **+ Box / + Ramp / + Cylinder** (or *Click in the view to place*, then a click on the ground) add one, **Add a floor** puts a 20 x 1 x 20 slab under Pac-Man. A primitive is drawn flat grey with its outline and is solid for Pac-Man at once (no restart; untick *Collision* for a see-through marker). Click it in the game view to select it; keys **1 / 2 / 3** (or the buttons) pick Move / Rotate / Scale: Move shows three arrows (the primitive's own axes, or the world's with *Move along the world axes*), Rotate three rings (X turns about its own X axis, Y about Y after the Z turn, Z about the world's Z: the three angles of the numeric *turn* field), Scale one handle per local axis (**Shift** + drag scales all axes together). **Ctrl** snaps (steps in the three *snap* boxes), dragging the body slides it over the floor (**Alt** = up / down), **PgUp / PgDn** raise / lower, **Del / X** delete, **Ctrl+D** duplicate, **Ctrl+Z / Y** undo / redo (the graybox and the placement files undo in the order they were edited). **Q** (or the *axes* box) switches Move and Rotate between the primitive's own axes and the world's (Scale is always along its own axes; rotating about a world axis turns the whole orientation about it and rewrites the three angles). The fields give position, turn (degrees, X Y Z, applied as Rz * Ry * Rx) and size per local axis, the shape and a name. *Graybox stage only* (then **Restart**) hides the stage itself, so Pac-Man rolls on the primitives alone (same as `--g03-gray-only`). Primitives belong to the round that is running, **Pac-N-Roll** (any Pac-N-Roll job: the `cG03PacnRollSys` as fields: round, world / stage, Previous / Next / Restart stage, retry / all clear / newly opened / debug-unlock flags, enemies and items counters, the saved lives / max HP and the retry data), **Pac-Man** (only inside a Pac-N-Roll stage: his `cTaskCharPacman` members as fields: translation, velocity, last floor position, radius (his model scales with it, radius / 0.38, unless *Scale his model with the radius* is off), HP / max HP / lives, form (Normal / Wing / Knight through the game's own change effect), the power cookie (starts / ends the real one: Pac-Man, bosses and ghosts), enemies eaten and combo, every state flag as a checkbox (the last three end the run) and the movement parameters of his current form; *No damage*, *Zero velocity*, *Up 5*. A yellow ring + post marks him in the game view (also while the Editor tab is shown): drag it with the left mouse to put him somewhere else (Alt = up / down, Ctrl snaps to 0.5); he keeps still where he is dropped), **Motos editor** (only inside a single-player Pac-Motos stage: an editor for the stage's map, MAPOO_OBJ: panels, gimmicks, items, enemies, players, the panel light pattern and enemy formations. Pages Tools (1 Select, 2 Panel, 3 Gimmick, 4 Item, 5 Enemy, 6 Player, 7 Erase; the brush's kind and its parameters: direction, formation role / group, Nabicon timers, hatch phase / times), Selection (every node in a list, its kind, position and parameters, its formation's steps), Stage (light pattern, grid / markers, shift everything one cell, clear panels / gimmicks / items / enemies), Check (problems such as no player start or two panels in a cell). In the game view: left click / drag places (panels paint), Select tool click + drag moves (snapped to cells; Shift or **Free 1/16 positions** for free positions), right click removes the top thing of a cell (a right drag turns the editor camera, or with it off swipes things away), R turns, Del / X deletes, Ctrl+Z / Ctrl+Y undo / redo, Ctrl+D duplicates, Ctrl+S saves, H J K L shift everything, Esc deselects, Enter applies. While the tab is shown the stage stands still (players, enemies, gimmicks, meteors, the slope, the READY / game steps; the panels' animations and light pattern keep playing): switch tabs or hide the window to play. **Skip the item settings** confirms the stage's item setup screen at once (no items used) and hides the READY console. While the tab is shown on the item setup screen, the stock of power / jump / charge reads 99 each (the game still takes at most 5 of each); the real stock comes back when the stage ends or is left and nothing is used up (items picked up in that stage are not kept). **Preview the edited map** (default on, while the tab is shown): the edited map's panels (with their light pattern), walls, gimmicks, items, enemies (a boss as its main body only) and players are drawn as models in place of the stage's own, before Apply; the background stays, nothing in the stage changes, models this stage's archives lack are listed and not drawn. **Difficulty** is the Menu tab's setting (shown with the one the stage is playing; Restart to change it). **Editor camera** as in the Editor tab, **Top view** looks straight down on the arena. **Apply + restart** restarts the stage with the edits (only that stage reads them, until **Back to the disc** or the end of the run); **Save** / **Load** keep a stage's MAPOO_OBJ in `$MEGAMIX_G07_EDITS/stageNN/` (default `~/.local/share/megamix/g07_edits`); nothing is written to the disc data), **Galaga editor** ([GALAGA_STAGES.md](GALAGA_STAGES.md) explains the stage format; only inside a single-player Galaga course stage, stages 1 to 9, not the boss stage: an editor for the stage's course script (the scn file: course commands ADVANCE / RAMP / WAIT / GATE and enemy spawns) and the enemy orbits (the .emy files listed in the stage's EnemyOrbit csv). Pages Scene (every record in a list with its course time and, for spawns, the game frame it triggers at or "never"; + Spawn / + Ramp / + Wait / + Gate at the playhead, Dup, Delete, Up / Down, **Play to it**; the selected record's fields: enemy, orbit, course time, delay, position, group, p1, marker, or command, speeds, delay, gate groups), Orbit (the selected spawn's orbit: New orbit / Copy orbit (a new attackN.emy), its nodes with frame, position, rotation, tangent, interpolation, facing, jump target, fire pattern; + Node / Delete node / Sort nodes), Check (problems such as spawns that never trigger, orbits without a file, a course that never ends; **Select it**). While the tab is shown the stage stands still (the game's pause flag; untick **Stand still** to play); the selected spawn's position (yellow), its orbit's path and nodes (blue, the selected one orange) and Hermite tangents (orange tips) are drawn in the game camera's frame at that moment and can be dragged in the game view (X / Y / Z during a drag keep it on that local axis, again = free). **Play to it** (or a double click on a record) lets the stage run until the course reaches that record (restarting the stage when it is behind) and stands still again; gates wait for their enemies to be shot. N adds a spawn at the playhead, T sets the selected spawn's course time to the playhead, Del deletes the record, Ctrl+Z / Ctrl+Y undo / redo, Ctrl+D duplicates, Ctrl+S saves, Enter applies. **Editor camera** as in the other editors (only while the stage stands still). **Apply + restart** restarts the stage with the edits (only that stage reads them, until **Back to the disc** or the end of the run); **Save** / **Load** keep a stage's scn, its orbits and its EnemyOrbit csv in `$MEGAMIX_G01_EDITS/stageNN/` (default `~/.local/share/megamix/g01_edits`); nothing is written to the disc data. Enemy names are fixed (zako, goei, boss, ...); there is no packing back into the game's archives), **Save** (edit a copy of the save data: **Open** a save folder (default the current one) reads museum.bin and banner.bin; pages Galaga (every stage's single / multiplayer record, open flags, all open), Rally-X (stage open / best score / best time / new marks, all clear), Pac-N-Roll (museum.bin block 2: lives, max HP, all clear, per round open / pizza / time / ghosts / cookies / new marks), Pac-Motos (block 4: per stage open / clear / high score / record time / new marks, item stock, player stock, all clear), Gator / Grobda (high scores and their valid flags), Arcade (the classic hall's high scores) and Raw (any block in hex, one word editable); **Write** writes them to the **write to** folder (default `<save folder>_edit`; it may be the opened folder, which is then overwritten; museum.bin's CRC-32 is recomputed, banner.bin copied, or the game's own banner when the folder has none or a blank one). The written folder can go into a Dolphin user folder's `Wii/title/00010000/534e4d45/data/` to play the edited save in the real game. The running game keeps the save it loaded: restart it (or start it with `MEGAMIX_SAVE=<that folder>`) to play with the edited save; written over its own save folder, its next save would overwrite the edits), **SCN0** (every scene animation resource (SCN0: lights, ambient lights, light sets, fogs, cameras) the game has loaded this run, by entry name and checksum; edit a light's type / on-off / colour / position / aim / specular / attenuation, an ambient colour, a light set's ambient and lights, a fog's type / range / colour, a camera's position / aim or rotation / fov / near / far. The edits are temporary: they last until you quit, nothing is written. They apply to every later read of that resource; most screens read their scene once when the job starts, so press **Restart the job** to see them (the tab says which kind the picked scene is). An edit of a keyed (animated) value makes it constant. **Revert** / **Revert all** go back to the disc values) and **UI** (theme file, zoom, a sample of every widget). The panel's width: drag its left edge (the grip, a bar with dots; the cursor turns into the resize arrows) or use the UI tab's *Panel width* (240 to 900 units, kept in `~/.cache/megamix/ui_panel_w` next to the zoom). When the tabs do not fit, the row scrolls sideways (the < / > ends or the mouse wheel over it) |
| `--preview-all` | developer switch (not part of the 1:1 game): every Pac-N-Roll, Pac-Motos, Galaga and Rally-X stage and world reads as unlocked so each menu world can be viewed; progress is not saved while it is on |
| `--g03-life N` | developer switch (not 1:1): Pac-N-Roll starts with N extra lives instead of the save's value (also `MEGAMIX_G03_LIFE=N`) |
| `--g04-time S` | developer switch (not 1:1): Gator Panic's play time is S seconds instead of CSV_GAME's 120 (tests the time up, result and "Play again?" quickly) (also `MEGAMIX_G04_TIME=S`) |
| `--g02-stage N` | developer switch (not 1:1): the Rally-X stage job entered directly (`--job 0x0c300000`) plays stage index N (1..30; 1 = world 1 stage 1) (also `MEGAMIX_G02_STAGE=N`) |
| `--g01-autoaim` | developer switch (not 1:1): while A is held with the pointer on screen, Galaga shots aim at the nearest enemy or missile in front of the camera (also `MEGAMIX_G01_AUTOAIM=1`) |
| `--g01-invincible` | developer switch (not 1:1): Galaga's Pac-Man takes no damage, to test a whole course (also `MEGAMIX_G01_INVINCIBLE=1`) |
| `--g01-stage N` | developer switch (not 1:1): the Galaga stage job entered directly (`--job 0x0b300000`) plays stage N (1..10) instead of stage 1 (also `MEGAMIX_G01_STAGE=N`) |
| `--g03-no-damage` | developer switch (not 1:1): Pac-N-Roll's Pac-Man loses no HP from hits (hit sound and knockback stay; falling off the stage still misses; `--g03-kill` has no effect with it) (also `MEGAMIX_G03_NO_DAMAGE=1`) |
| `--g03-no-gates` | developer switch (not 1:1): Pac-N-Roll stages spawn none of their Golvis gates, so the whole course is open (also `MEGAMIX_G03_NO_GATES=1`) |
| `--g03-skip-title` | developer switch (not 1:1): a Pac-N-Roll stage starts without the stage name card, the way the game starts a retry; the opening camera move still plays (also `MEGAMIX_G03_SKIP_TITLE=1`) |
| `--g03-fly` | developer switch (not 1:1): Pac-N-Roll's Pac-Man flies instead of rolling during play: Nunchuk stick = move on the X / Z plane relative to the camera (up = away from it), **C** = ascend, **Z** = descend, **B** held = 3x speed, **A** held = 2x speed (both held = 6x). No gravity, no collision with the stage (he passes through walls and floors) and no fall-off miss; enemies and gimmicks can still hurt him (combine with `--g03-no-damage`). Wiimote **1** switches between flying and normal rolling (gravity, collision and the fall-off miss come back; press it again to fly from where he is); a run starts flying (also `MEGAMIX_G03_FLY=1`) |
| `--g03-show-hit` | developer switch (not 1:1): Pac-N-Roll draws the stage's HIT geometry (the collision faces Pac-Man rolls on and bumps into, without the gimmicks) as a wireframe over the scene, hidden by what is in front of it. Colours by face kind: grey wall, yellow bounce wall, green floor, dark green grass, blue water, cyan dash pad, orange jump pad, red magma, **purple poison**, dark pink camera-only, gold hub door / other (also `MEGAMIX_G03_SHOW_HIT=1`) |
| `--g03-show-gimmick-hit` | developer switch (not 1:1): Pac-N-Roll draws only the gimmicks' hit data: their collision faces (same colours as `--g03-show-hit`; a falling wall's faces that hurt Pac-Man in **white**), and drawn over everything, the boxes / spheres the gimmicks' own tests use, in **white** when they hurt (rolling log box, arrow, iron ball) and pink for the falling wall's trigger area. A box or sphere is where Pac-Man's centre must be to be hit. Boxes show only while the gimmick runs (not paused, near Pac-Man). Both switches can be on together (also `MEGAMIX_G03_SHOW_GIMMICK_HIT=1`) |
| `--g03-bg-collision` | developer switch (not 1:1): Pac-N-Roll's collision is built from the models that are **drawn** instead of the stage's hidden hit models. The stage's hit models are not loaded; every background model becomes a collision object made of its own triangles, and every gimmick / wooden box / gate takes its faces from its visible model (only those that have hit data in the game). Animated models are re-shaped every frame. Faces are only floor (up to 60 degrees from flat) or wall, so the special surfaces are gone: no bounce pads, dash / jump pads, magma, poison, water, grass or camera-only faces. Translucent parts (water surfaces, glass) and billboards (sprites that turn to the camera) are not solid. `--g03-show-hit` / `--g03-show-gimmick-hit` draw the faces it built (also `MEGAMIX_G03_BG_COLLISION=1`) |
| `--g03-no-menu-warp` | developer switch (not 1:1): the main menu hub never warps to a game: rolling onto a game building's door does nothing, so the whole garden can be explored (also `MEGAMIX_G03_NO_MENU_WARP=1`) |
| `--g03-jump` | developer switch (not 1:1): Pac-N-Roll's Pac-Man (also in the hub) jumps when **C** is pressed while he is on a floor, with the jump sound and animation; the upward speed is the strength (default 0.35) times 3 while **B** is held and times 2 while **A** is held (both = 6x), as `--g03-fly`'s speed. B also brakes, as in the game |
| `--g03-jump-strength S` | `--g03-jump` with strength S instead of 0.35 (also `MEGAMIX_G03_JUMP=S`, which switches the jump on) |
| `--g03-freecam` | developer switch (not 1:1): Pac-N-Roll's camera turns left / right as in the game but without its limits: no camera-area turning ranges, no wall checks and no automatic turn away from walls, so the "no camera control" sign never shows (the camera may go through walls). The right stick's Y axis (or the D-pad up / down) raises / lowers the point the camera looks at, which changes the pitch; the offset stays until moved back. Also `MEGAMIX_G03_FREECAM=1` |
| `--g03-explore ARCHIVE ENTRY[,ENTRY...]` | developer switch (not 1:1): roll Pac-N-Roll's Pac-Man around any models from any game's archive, e.g. Pac-Motos world 2: `--g03-explore g07_motos/g07w02s01.lzs MDL_BG_1,MDL_BG_2+TSA_BG_2,MDL_BG_3,MDL_BG_4` (an entry that is not in the archive is reported and skipped). `ENTRY+ANIM+ANIM` plays those animation entries on that model only. It starts the Pac-N-Roll game by itself (`--round` picks the stage whose lights, camera and music are used; default World 1 Stage 1). The stage is replaced by the models, placed at the origin (a Pac-Motos stage's `MDL_BG_1..4` are placed, scaled and animated at the rate its own `CSV_BG` gives, as in Pac-Motos); collision is built from them the way `--g03-bg-collision` does it (floor / wall only); the stage's items, ghosts, gates, boxes, checkpoints, water, wind, fall areas, camera areas and goal are left out. Pac-Man is dropped onto the highest floor under the middle of the models (or starts at `--g03-start`; the log prints the models' bounds and the start), and is put back there when he falls 30 units below them. The other Pac-N-Roll switches work with it (`--g03-fly`, `--g03-jump`, `--g03-show-hit`, `--g03-no-damage` ...) |
| `--g03-explore-anim ENTRY[,ENTRY...]` | (with `--g03-explore`) animation entries of the same archive played on every model: bone (CHR0), texture scroll (SRT0), texture swap (PAT0), colour (CLR0), visibility (VIS0), e.g. `CHA_BG_1,TSA_BG_1,CLA_BG_1`. Animated models are solid as they move |
| `--fullscreen` | start in borderless fullscreen (also `MEGAMIX_FULLSCREEN=1`). **F11** or **Alt+Enter** toggle it at any time; the 640x480 game keeps its black bars and the dev window (`--devui`) takes its width from the screen instead of widening the window |
| `--g03-gray FILE` | developer switch: load the graybox primitives of FILE (a `graybox.json` written by the editor's Graybox page, see `--devui`) into every Pac-N-Roll stage; a round with its own list in the editor keeps that |
| `--g03-gray-only` | developer switch: Pac-N-Roll stages are made of the graybox primitives only: the stage's models, collision and placement data (items, ghosts, gates, camera areas ...) are not made; Pac-Man starts above the first primitive and is put back there when he falls 30 units below all of them. Starts the Pac-N-Roll game job when no `--job` is given |
| `--g03-explore-scale S` | (with `--g03-explore`) scale of the models and their placement (default 1); Pac-Man keeps his size, so this sets how big the world feels |
| `--g03-cookies N` | developer switch (not 1:1): Pac-N-Roll starts with N Pac-Dots in hand, to test gates (also `MEGAMIX_G03_COOKIES=N`) |
| `--g03-kill N` | developer switch (not 1:1): Pac-N-Roll deals lethal damage to Pac-Man at game frame N, to test the miss / retry / game over flow (also `MEGAMIX_G03_KILL=N`) |
| `--g03-start X Y Z` | developer switch (not 1:1): Pac-N-Roll starts Pac-Man at this stage position (also `MEGAMIX_G03_START=x,y,z`) |
| `--tap-every N` | inject an A press every N frames (testing) |
| `--no-audio` | run without sound: no audio device is opened (movie and sound effects silent) |
| `--audio-dump FILE` | developer switch: no audio device; the sound mix is rendered offline, one game frame of audio per frame run, and written to FILE (48 kHz stereo 16-bit WAV) when the run ends. Same length whatever the host speed, so runs can be compared (`MEGAMIX_SND_LOG=1` prints each cue) |
| `--data DIR` | the game data: the extracted disc's `files` folder (the one holding `museum/`). A Dolphin "Extract Entire Disc" folder works as `DIR`, `DIR/files` or `DIR/DATA/files`. Overrides `data_root` and `MEGAMIX_DATA`. When no game data is found the game shows a message (in the window and on stderr) and exits with code 2 |
| `--dol FILE` | the game executable `main.dol`. Overrides `dol_path` and `MEGAMIX_DOL`. Optional: without it the debug font (debug menus) and the engine textures depth / ramp16 (some shadows) are missing |
| `-h`, `--help` | print the option summary and exit |
| `--arcade NAME` | developer switch (work in progress): runs a classic arcade game of the collection instead of the Megamix boot, read from the game data (`museum/<language>/rso_original/nmwii2.rso`). Only `pacman`, `rallyx` and `newrallyx` so far (PAC-MAN is also reachable in the debug build's menus: job 174, row RSO 09). A or PLUS between games opens the game's START menu (1P START / 2P START, a ROUND SELECT row under each once a round beyond the first was reached (left / right pick the round), OPTIONS, RETURN TO ARCADE): the D-pad moves the cursor, 2 / A / PLUS choose, MINUS / 1 go back to the attract mode (so does 30 seconds without input), RETURN TO ARCADE stops the game. OPTIONS opens the game's options (EXTEND, LIVES, SOUND TEST, SCREEN ADJUSTMENT, DEFAULT, EXIT): up / down move the cursor, left / right change a value, 2 plays the sound test number / resets the defaults / exits, A / PLUS leave the options; SCREEN ADJUSTMENT does nothing yet. The options apply to the next game. HOME = stop the game. A or PLUS during a game opens the game's PAUSE menu (CONTINUE / SCREEN ADJUSTMENT / QUIT GAME): the D-pad moves the cursor, 2 chooses the item, A / PLUS / MINUS / 1 close the menu (= continue), as in the original; SCREEN ADJUSTMENT does nothing yet, QUIT GAME returns to the game's attract mode. The remote is held sideways as in the original (the D-pad is turned by the game's own control table; a Nunchuk stick works too). Hi scores are only kept within the run (the save file is written when the game is left through the debug build's menus, job 174). With sound (the board's wave sound chip; silent with `--no-audio` or no audio device). Works with `--shot`, `--press`, `--hold` and `--dump-tree N` (which then just runs N frames) |
| `--view-model ARCHIVE ENTRY [MODEL]` | orbit viewer for a BRRES model, e.g. `--view-model g10_menu/g10title.lzs TITLE_PACMAN_MDL`. Left drag = orbit, wheel = zoom |
| `--chr ENTRY` / `--srt ENTRY` / `--pat ENTRY` | (with `--view-model`) play a bone / texture-scroll / texture-swap animation entry of the same archive |
| `--at FRAME` | (with `--view-model`) freeze the animations at that frame instead of playing them |

Example: `MEGAMIX_SAVE=/tmp/save ./build/megamix --dump-tree 130 --job 0x08900000 --press 100 a --shot 120 title.png`
(use a scratch save folder for test runs: a run that reaches a result screen writes the save).

Flags that take no value (`--preview-all`, `--devui`, `--g01-invincible`, `--g01-autoaim`, `--g03-standin`) are only seen when another argument follows
them; `--no-smooth`, `--no-nunchuk` and `--no-audio` work anywhere.

## 3. `megamix.cfg` (`key = value`, `#` starts a comment)

| key | default | meaning |
|---|---|---|
| `data_root` | `data` in the working directory if it exists, else `~/Games/Wii/_nmm/DATA/files` (the original developer's folder) | extracted disc `files` directory (the one holding `museum/`); its parent, or a Dolphin `DATA` folder above it, is accepted too; `~/` is expanded |
| `language` | `en` | folder under `museum/` to load. The US disc (SNMEAF) has only `en`; other values (`jp`, `de`, `fr`, `es`, `it`, `nl`, `kr`) only change the language the game reports (`SCGetLanguage`) and need a disc with that folder. Only `en` is tested |
| `save_dir` | `save` | folder standing in for the game's NAND save directory |
| `dol_path` | `main.dol` in the working directory; when that file does not exist, the disc's `sys/main.dol` next to the `files` folder | the game executable (engine textures depth / ramp16 and the debug font) |
| `audio` | `on` | `off` (or `0`, `false`, `no`) disables all sound, same as `--no-audio` |
| `volume` | `100` | master volume 0..100 for the sound effects and music |
| `remote_speaker` | `0` | the Wii's Wii Remote speaker volume setting, 0..127. Some sounds (menu decide, Rally-X brake / nitro / crash, Gator Panic hits) also play on the remote's speaker in the original; a PC has none, so with a value above 0 that copy is mixed into the normal output at value / 127 x 0.5 of full volume (Dolphin does the same). 0 = off |

Unknown keys are reported on stderr and ignored. The environment variables of section 6 override these keys.

## 4. `input.cfg` (controller bindings)

Format: `<input> = <source>, <source>, ...` (any listed source triggers the input). Delete the file to restore the defaults.

* **Pad semantics (from the DOL)**: the game sees the same button words as on the Wii (A = decide, MINUS = pause in Pac-N-Roll, HOME held = freeze); the title / dialogs accept **A** (not 1, 2 or Plus), a dialog accepts A, B or Z.
* **Inputs**: `Left Right Up Down Plus Minus Home 1 2 A B Z C Shake` (Wiimote / Nunchuk; `Shake` = swing the remote, default `Space` / gamepad R2: Pac-N-Roll's **dash** and hang escape and Gator Panic's **whack** need a swing, there is no accelerometer; in Gator Panic the Nunchuk stick picks the hole: up / left / right / centre), `StickUp StickDown StickLeft StickRight` (Nunchuk stick).
* **Sources**: `Key:<name>` keyboard (channel 1 only), `Pad:<button>` gamepad of the same channel (`South East West North L1 R1 L2 R2 Back Start Guide L3 R3 DUp DDown DLeft DRight`),
  `Pad2:<button>` / `Axis2:...` a specific gamepad (index from 0), `Axis:<axis>+/-` (`LeftX LeftY RightX RightY LTrigger RTrigger`), `Mouse:<button>` (`Left Right Middle`, channel 1 only).
* `Orientation = Sideways | Upright` (default `Sideways`): `Sideways` rotates the D-pad bindings as for a remote held sideways, `Upright` passes them
  through. It applies to the boot screens, title, attract movie and the debug menus. The hub and every game (they use the Nunchuk, so the
  remote is held upright) and the save dialogs (their Yes / No sit side by side: Left = Yes, Right = No) always use `Upright`, whatever the setting.

The IR pointer of Wiimote 1 is the mouse; Wiimotes 2..4 have a pointer moved by their gamepad's **right stick** (it starts at the screen centre; a PC gamepad has no IR camera, so this is a native stand-in, used to aim in Galaga multiplayer). The mouse pointer covers the 4:3 game screen centred in the window (outside the window = no pointer, the menus then use the D-pad). The cursor is hidden while HOME is held (HOME stands in for the Home Button Menu: while it is held every game task freezes, like the original with the HBM open). Click = A.

In Pac-N-Roll (as in the original) Pac-Man **dashes when you swing the remote (Space)**, is steered with the **Nunchuk stick** (`WASD` / the gamepad's left stick), brakes while **B or Z** is held, and the D-pad (arrows) turns the camera; the gamepad's **right stick** turns it too (a native addition: pushed past half travel it acts as the D-pad direction it points to).

Default keys: arrows = D-pad, `J` / Enter / left mouse = A, `K` / right mouse = B, `1` / `2` = the remote's 1 / 2 buttons, `=` = Plus, `-` = Minus, `H` = Home,
left Shift = Z, left Ctrl = C, Space = swing (Shake), `WASD` = Nunchuk stick.
Default gamepad: South = A, East = B, West = 1, North = 2, Start = Plus, Back = Minus, Guide = Home, L1 = Z, R1 = C, R2 = swing, D-pad = D-pad,
left stick = Nunchuk stick. Keyboard and mouse drive Wiimote 1; the first gamepad drives Wiimote 1, the second Wiimote 2, and so on.

## 5. Runtime keys

| key | meaning |
|---|---|
| `P` | print the task tree to the terminal |
| `F9` | show / hide the dev window (`--devui`). While it is open: the mouse over it does not reach the game; while one of its fields is being typed into, the game sees no keys and `Esc` cancels the field instead of closing the window |
| `Ctrl` + `=` / `-` / `0` | dev window: zoom in / out / back to 100 % |

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
| `MEGAMIX_G03_NO_DAMAGE=1` | same as `--g03-no-damage` |
| `MEGAMIX_G03_NO_GATES=1` | same as `--g03-no-gates` |
| `MEGAMIX_G03_FLY=1` | same as `--g03-fly` |
| `MEGAMIX_G03_SKIP_TITLE=1` | same as `--g03-skip-title` |
| `MEGAMIX_G03_SHOW_HIT=1` | same as `--g03-show-hit` |
| `MEGAMIX_G03_SHOW_GIMMICK_HIT=1` | same as `--g03-show-gimmick-hit` |
| `MEGAMIX_G03_BG_COLLISION=1` | same as `--g03-bg-collision` |
| `MEGAMIX_G03_NO_MENU_WARP=1` | same as `--g03-no-menu-warp` |
| `MEGAMIX_G03_FREECAM=1` | same as `--g03-freecam` |
| `MEGAMIX_G03_JUMP=S` | same as `--g03-jump-strength S` (`MEGAMIX_G03_JUMP=1` = strength 1) |
| `MEGAMIX_G03_COOKIES=N` | same as `--g03-cookies N` |
| `MEGAMIX_G03_KILL=N` | same as `--g03-kill N` |
| `MEGAMIX_PREVIEW_ALL=1` | same as `--preview-all` |
| `MEGAMIX_DEVUI=1` | same as `--devui` |
| `MEGAMIX_G07_EDITS=DIR` | where the dev window's Pac-Motos editor saves / loads edited maps (`DIR/stageNN/MAPOO_OBJ`; default `$XDG_DATA_HOME/megamix/g07_edits`, i.e. `~/.local/share/megamix/g07_edits`) |
| `MEGAMIX_G01_EDITS=DIR` | where the dev window's Galaga editor saves / loads edited stages (`DIR/stageNN/`: the scn, the .emy orbits, EnemyOrbit_sN.csv; default `$XDG_DATA_HOME/megamix/g01_edits`, i.e. `~/.local/share/megamix/g01_edits`) |
| `MEGAMIX_G03_EDITS=DIR` | where the dev window's Pac-N-Roll editor saves / loads edited stage files (`DIR/roundNN/`; default `$XDG_DATA_HOME/megamix/g03_edits`, i.e. `~/.local/share/megamix/g03_edits`) |
| `MEGAMIX_UI_THEME=FILE` | the dev window's theme file (default `$XDG_CONFIG_HOME/megamix/ui_theme.json`, i.e. `~/.config/megamix/ui_theme.json`) |
| `MEGAMIX_UI_FONT=FILE` | a .ttf font for the dev window's text (overrides the theme's `font` key; default `resources/fonts/Verdana.ttf` in the working directory when it is there, else raylib's font; `raylib` = raylib's pixel font) |
| `MEGAMIX_G01_STAGE=N` | same as `--g01-stage N` |
| `MEGAMIX_G01_INVINCIBLE=1` | same as `--g01-invincible` |
| `MEGAMIX_G01_AUTOAIM=1` | same as `--g01-autoaim` |
| `MEGAMIX_G02_STAGE=N` | same as `--g02-stage N` |
| `MEGAMIX_G04_TIME=S` | same as `--g04-time S` |
| `MEGAMIX_G04DBG=1` | Gator Panic game: print the app mode changes and, once a second, the timer, hits, score, the four gators' states and Pac-Man's state |
| `MEGAMIX_HIDDEN=1` | never show the window (scripted runs: `--shot` still works, nothing takes the keyboard). `tools/safe_run.sh` sets it |
| `MEGAMIX_G01_PLAYERS=N` | Galaga: with `--job 0x0b200000` (stage select) or `0x0b300000` (stage), play with N players (2..4, pads 0..N-1; empty names become PLAYERn) |
| `MEGAMIX_G02_PLAYERS=N` | Rally-X: with `--job 0x0c300000`, start the stage with N players (1..4); with `--g02-stage 25` (a battle stage) it is the multiplayer match |
| `MEGAMIX_G02_TIME=S` | Rally-X multiplayer stage: the match lasts S seconds instead of the menu's time (to see the winner / draw screens quickly) |
| `MEGAMIX_G11_PLAYERS=N` | Grobda: with `--job 0x15300000`, start the stage with N players (1..4; N > 1 = multiplayer rules) |
| `MEGAMIX_G01DBG=1` | Galaga stage: print player 1's score / kills / shots / hits and the life gauge to the terminal once a second, plus app mode changes, Pac-Man states, kills, bombs, fighters and course effects with their frame numbers (the frame `--shot` uses) |
| `MEGAMIX_LENSDBG=1` | Galaga stage: print the lens flare's light position on screen, its visibility and every sprite layer each frame |
| `MEGAMIX_SCENE_DEBUG=1` | 3D scene debug output: scene / camera state every frame, and at 3D frame 100 (or at frame N with `MEGAMIX_SCENE_DEBUG=N`, N > 1) the list of drawn models plus the loaded visible models no task drew that frame; in Pac-N-Roll also Pac-Man's position every 30 frames |
| `MEGAMIX_SCENE_REGISTERED=1` | debug: in Galaga, Pac-N-Roll, Pac-Motos and the hub, draw every loaded visible model (the port's old behaviour) instead of only the models the game drew that frame; for comparison shots |
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
* The text is the engine's own debug text (8x8 glyph font read from `main.dol` 0x80442bc0, see `dol_path`;
  without it the debug menus have no text).

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
* **Progress** (unlocked stages, records) is saved in `museum.bin` block 2, like the original. A `<save_dir>/pacnroll.bin` written by older port builds is imported once when block 2 is still empty (the file is left alone).
* **Not ported yet:** moving gimmicks (objects Pac-Man can stand on that move), water, hanging on ledges, the knight / wing forms, some round-specific
  behaviour (rounds 5, 9, 15, 22). Ghosts, bosses, gates, boxes, items, lives, pause, retry, game over and the staff roll are in.
