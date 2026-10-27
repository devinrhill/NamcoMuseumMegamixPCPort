Namco Museum Megamix - native PC port (version @VERSION@, Windows x86_64)
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
  a) Edit megamix.cfg (Notepad works) and set
         data_root = C:\Games\Megamix\DATA\files
  b) Start the game from a command prompt with
         megamix.exe --data C:\Games\Megamix\DATA\files
  c) Put (or copy) the "files" folder next to megamix.exe, named "data".

You may also give the DATA folder itself, or the folder above it; the game
finds "files/museum" below it. If the data is not found, the game shows a
message saying where it looked.


3. Play
-------
  megamix.exe             (double-click it; a console window opens beside
                          the game and shows messages)
  megamix.exe --help      all options (from a command prompt)

Run it from its own folder: settings, controls and saves are read from the
folder the game is started in.

Saves go to the "save" folder next to megamix.exe (the game's own Wii save
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
  Windows 10 or 11, 64-bit, with an OpenGL 3.3 graphics driver (any GPU
  from the last ten years with its vendor's driver). Nothing else to
  install: raylib, libjpeg-turbo and the C++ runtime are inside
  megamix.exe. The Windows build is new and has mostly been tested under
  Wine on Linux; please report problems.


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
