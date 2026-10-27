Namco Museum Megamix - native PC port (version @VERSION@, @SYSTEM@)
=====================================================================

A from-scratch PC rebuild of the Wii game "Namco Museum Megamix" that runs
the ORIGINAL game's data. No game data is included: you need your own copy
of the disc.


1. Get the game data
--------------------
Extract your disc with Dolphin:
  right-click the game > Properties > Filesystem >
  right-click the disc at the top > Extract Entire Disc...

That gives a folder with "files" and "sys" inside (Dolphin calls it DATA).
"files" holds the "museum" folder the port needs. "sys/main.dol" is used
for a few extras when it is there.


2. Tell the game where the data is (any ONE of these)
-----------------------------------------------------
  a) Edit megamix.cfg and set
         data_root = /path/to/DATA/files
  b) Start the game with
         ./megamix.sh --data /path/to/DATA/files
  c) Put (or symlink) the "files" folder next to megamix.sh, named "data":
         ln -s /path/to/DATA/files data

You may also give the DATA folder itself, or the folder above it; the game
finds "files/museum" below it. If the data is not found, the game shows a
message saying where it looked.


3. Play
-------
  ./megamix.sh            (from a terminal, or double-click it)
  ./megamix.sh --help     all options

Saves go to the "save" folder next to megamix.sh (the game's own Wii save
format). Settings: megamix.cfg. Controls: input.cfg (written with the
defaults the first time the game runs; delete it to get them back).

Default controls (a Wiimote + Nunchuk on the keyboard / mouse / gamepad):
  arrows          D-pad              J / Enter / left mouse   A
  K / right mouse B                  1 / 2                    1 / 2
  =  / -          Plus / Minus       H                        Home
  Left Shift      Z                  Left Ctrl                C
  W A S D         Nunchuk stick      Space                    swing the remote
  mouse           pointer (IR)
A gamepad works too (South = A, East = B, Start = Plus, Back = Minus, left
stick = Nunchuk, R2 = swing).


4. Requirements
---------------
  Linux @ARCH@ with OpenGL 3.3 (libOpenGL / libGLX from the system's graphics
  driver), X11 or XWayland, and ALSA/PulseAudio/PipeWire for sound. The C
  runtime (glibc, libstdc++) and the X11 libraries are bundled in lib/ and
  megamix.sh uses them, so older systems (e.g. a Chromebook's Linux) work.
  Start the game with megamix.sh, not bin/megamix directly.


5. What works
-------------
Boot, save / load, title, the main menu hub and these games in single
player: Pac-N-Roll Remix, Pac-Motos, Galaga Remix, Gator Panic Remix,
Rally-X Remix, Grobda Remix. Multiplayer: Gator Panic, Galaga, Pac-Motos,
Rally-X; Grobda partly. The classic arcade hall: PAC-MAN only (it is read
from your disc's own module; the other cabinets return at once).

Known problems:
  - Pac-Motos: the board is see-through.
  - The debug menu's RSO row leaves an empty screen (restart the game).
Missing: Mii faces, the Home Button menu (HOME only freezes the game),
widescreen, some shadows, 3D sound positioning.
Details: docs/JOBS.md (every screen and its status) and docs/OPTIONS.md.

Options marked "developer switch" / "dev:" in --help and docs/OPTIONS.md
are testing aids and are not part of the original game.
