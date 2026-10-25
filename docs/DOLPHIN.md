# Comparing the port with the real game in Dolphin

`tools/dolphin/` runs the retail game (SNMEAF) in Dolphin, drives it from the shell and takes screenshots of it, so that a
screen of the port can be put next to the same screen of the original. The real game in Dolphin is the reference: when the
port and Dolphin disagree, Dolphin is right.

## Setup (once)

* Dolphin (`dolphin-emu`, tested with 2609) on an X11 desktop, ImageMagick (`magick`, `import`), `xprop`.
* The disc image: `~/Games/Wii/_nmm.wbfs` by default (`dolphin.sh start OTHER.wbfs` to use another one).
* For job jumps and memory reads: `~/Code/proj/unpac/tools/jbc.py` and its `jobs.json` (Megamix = language `mm`).

The script uses its own Dolphin user folder, `build/dolphin_user` (a copy of your Dolphin config with both Wiimotes and the
Nunchuk mapped to input pipes, background input on, cheats off). Your own Dolphin settings, saves and any Dolphin window you
have open are never touched: `stop` only closes the Dolphin that `start` opened (its pid is in `build/dolphin_user/pid`).

## Commands

| command | does |
|---|---|
| `tools/dolphin/dolphin.sh start [disc]` | boot the game in a new Dolphin window |
| `tools/dolphin/dolphin.sh stop` | close that Dolphin |
| `tools/dolphin/dolphin.sh press BTN [SEC] [PAD]` | hold a Wiimote button for SEC seconds (default 0.15) on Wiimote PAD (1 or 2). BTN: `a b 1 2 minus plus home up down left right z c` |
| `tools/dolphin/dolphin.sh stick X Y [PAD]` | Nunchuk stick, 0..1 per axis (`0.5 0.5` = centre, `0.5 1` = up) |
| `tools/dolphin/dolphin.sh swing [SEC] [PAD]` | swing the Wiimote down for SEC seconds (default 0.2): Gator Panic's hammer. `build/dolphin_user/Config/WiimoteNew.ini` maps `Swing/Down` to the pipe's `L` axis (added in the 27th session; restart Dolphin after changing that file) |
| `tools/dolphin/dolphin.sh ir X Y [PAD]` | pointer, 0..1 per axis (`0.5 0.5` = screen centre, larger Y = higher) |
| `tools/dolphin/dolphin.sh shot NAME` | Dolphin's own screenshot (screenshot hotkey on a third pipe; works while the window is minimized or covered) -> `build/pic/dolphin_NAME.png` |
| `tools/dolphin/dolphin.sh cur` | print the current job number |
| `tools/dolphin/dolphin.sh job JOB [force]` | jump to a job (`jbc.py goto mm JOB`), e.g. `0x11000000` = Pac-Motos |
| `tools/dolphin/dolphin.sh jbc ARGS...` | any other `jbc.py` command against this Dolphin (`jbc listdeep mm`, `jbc permago mm 0x11300000`, ...) |
| `tools/dolphin/dolphin.sh py SCRIPT [ARGS]` | run a Python script that imports `dolphin_memory_engine` (bound to this Dolphin) |
| `tools/dolphin/compare.sh DOLPHIN.png PORT.png OUT.png` | side-by-side picture, both cut to the 4:3 game area and scaled to 640x480 |

Example (Pac-Motos stage select, Dolphin left, port right):

    tools/dolphin/dolphin.sh start            # wait for the game to boot
    tools/dolphin/dolphin.sh job 0x11000000 1
    tools/dolphin/dolphin.sh shot g07menu
    ./build/megamix --dump-tree 260 --job 0x11000000 --shot 250 build/pic/port_g07menu.png
    tools/dolphin/compare.sh build/pic/dolphin_g07menu.png build/pic/port_g07menu.png build/pic/cmp_g07menu.png

## Notes

* `dolphin_memory_engine` (the real module) attaches to the first Dolphin it finds, which may be yours. The scripts here use a
  stand-in with the same functions (`tools/dolphin/pydme/dolphin_memory_engine.py`) that only reads and writes the Dolphin
  started by `dolphin.sh` (through `/proc/<pid>/mem`). `jbc.py` runs unchanged with it.
* The Megamix JobHandler is the `mm` entry of `jobs.json` (0x80b037c0), not `en`.
* Input goes through named pipes (`build/dolphin_user/Pipes/wm1`, `wm2`, and `hk` for the screenshot hotkey); background input is on and
  `[General] HotkeysRequireFocus = False`, so the window doesn't need focus. You can minimize it.
* `tools/winshot.sh SECONDS OUT.png -- ./build/megamix ...` grabs the port's real window (after smoothing and the overlay text, unlike `--shot`).
* Animated screens differ from frame to frame: compare at the same moment of an animation, or compare things that stand still.
