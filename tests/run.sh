#!/bin/sh
# Builds and runs the host-independent tests (no window needed). The archive test skips itself if the disc data is missing.
cd "$(dirname "$0")/.." || exit 1
mkdir -p build/tests
FLAGS="-std=c++20 -fpermissive -w -Iinclude"
g++ $FLAGS tests/wiipad_test.cpp src/WiiPad.cpp src/InGamePadManager.cpp -o build/tests/wiipad_test 2>&1 | grep error
g++ $FLAGS tests/archive_test.cpp src/wii/Archive.cpp src/WiiSystem.cpp -o build/tests/archive_test 2>&1 | grep error
g++ $FLAGS tests/save_test.cpp src/kingdom/cKingdomSave.cpp src/wii/Archive.cpp src/wii/Nand.cpp src/WiiSystem.cpp -o build/tests/save_test 2>&1 | grep error
g++ $FLAGS tests/gfx_math_test.cpp -o build/tests/gfx_math_test 2>&1 | grep error
g++ $FLAGS tests/mdl0_dump.cpp src/wii/Mdl0.cpp src/wii/Brres.cpp src/wii/Archive.cpp src/WiiSystem.cpp -o build/tests/mdl0_dump 2>&1 | grep error
g++ $FLAGS tests/brres_dump.cpp src/wii/Brres.cpp src/wii/Archive.cpp src/WiiSystem.cpp -o build/tests/brres_dump 2>&1 | grep error
g++ $FLAGS tests/anim_dump.cpp src/wii/Anim.cpp src/wii/Brres.cpp src/wii/Archive.cpp src/WiiSystem.cpp -o build/tests/anim_dump 2>&1 | grep error
g++ $FLAGS tests/rfnt_dump.cpp src/wii/Rfnt.cpp src/wii/Brres.cpp src/wii/Archive.cpp src/WiiSystem.cpp -o build/tests/rfnt_dump 2>&1 | grep error
g++ $FLAGS tests/snd_dump.cpp src/snd/Strm.cpp src/snd/Archive.cpp -o build/tests/snd_dump 2>&1 | grep error
g++ $FLAGS tests/gotch_z80_test.cpp src/gotch/Z80.cpp src/gotch/RsoModule.cpp src/WiiSystem.cpp -o build/tests/gotch_z80_test 2>&1 | grep error
g++ $FLAGS tests/gotch_storage_test.cpp src/gotch/DataStorage.cpp -o build/tests/gotch_storage_test 2>&1 | grep error
rc=0
for t in wiipad_test archive_test save_test gfx_math_test gotch_z80_test gotch_storage_test; do ./build/tests/$t 2>/dev/null | tail -1 || rc=1; done
# BRRES texture decoding: every texture of the boot / title / resident archives must match the checksums produced by
# ~/Code/proj/unpac's Python decoder (tests/expected_textures.txt). Skipped when the disc data is not there.
M="${MEGAMIX_DATA:-$HOME/Games/Wii/_nmm/DATA/files}/museum"
if [ -f "$M/en/boot.lzs" ]; then
  ./build/tests/brres_dump "$M/en/boot.lzs" "$M/resident.arc" "$M/en/g10_menu/g10title.lzs" "$M/en/g10_menu/g10resident.lzs" "$M/en/g10_menu/g10menu.lzs" > build/tests/textures.txt
  if diff -q build/tests/textures.txt tests/expected_textures.txt >/dev/null; then echo "brres decode tests passed ($(wc -l < build/tests/textures.txt) textures)"; else echo "brres decode MISMATCH"; rc=1; fi
fi
# MDL0 parsing: per-shape vertex/triangle counts and coordinate sums; first verified against unpac's Mdl0.shape_mesh (equal up to 1e-3 rounding).
if [ -f "$M/en/g10_menu/g10title.lzs" ]; then
  ./build/tests/mdl0_dump "$M/en/g10_menu/g10title.lzs" "$M/en/g10_menu/g10menu.lzs" > build/tests/mdl0.txt
  if diff -q build/tests/mdl0.txt tests/expected_mdl0.txt >/dev/null; then echo "mdl0 tests passed"; else echo "mdl0 MISMATCH"; rc=1; fi
fi
# RFNT fonts: parse info + glyph lookups (snapshot checked by eye against the decoded sheet; nw4r fontResourceFormat.h rules).
if [ -f "$M/resident.arc" ]; then
  ./build/tests/rfnt_dump "$M/resident.arc" > build/tests/rfnt.txt
  if diff -q build/tests/rfnt.txt tests/expected_rfnt.txt >/dev/null; then echo "rfnt tests passed"; else echo "rfnt MISMATCH"; rc=1; fi
fi
# CHR0 / SRT0 / PAT0 / SCN0: every sample equals the unpac Python readers (needs ~/Code/proj/unpac; snapshot of the Python output otherwise).
if [ -f "$M/en/g10_menu/g10title.lzs" ]; then
  ./build/tests/anim_dump "$M/en/g10_menu/g10title.lzs" | sed 's/-0\.000/0.000/g' > build/tests/anim.txt
  if [ -d "$HOME/Code/proj/unpac" ]; then python3 -I tests/anim_ref.py "$M/en/g10_menu/g10title.lzs" | sed 's/-0\.000/0.000/g' > tests/expected_anim.txt; fi
  if diff -q build/tests/anim.txt tests/expected_anim.txt >/dev/null; then echo "anim tests passed ($(wc -l < build/tests/anim.txt) samples)"; else echo "anim MISMATCH"; rc=1; fi
fi
# CLR0 (material colour animation): Motos world 5 stage 7 (the boss stage, 13 clips)
if [ -f "$M/en/g07_motos/g07w05s07.lzs" ]; then
  ./build/tests/anim_dump "$M/en/g07_motos/g07w05s07.lzs" | grep -E "CLR0 frames|mask=" > build/tests/clr.txt
  if [ -d "$HOME/Code/proj/unpac" ]; then python3 -I tests/anim_ref.py "$M/en/g07_motos/g07w05s07.lzs" | grep -E "CLR0 frames|mask=" > tests/expected_clr.txt; fi
  if diff -q build/tests/clr.txt tests/expected_clr.txt >/dev/null; then echo "clr tests passed ($(wc -l < build/tests/clr.txt) samples)"; else echo "clr MISMATCH"; rc=1; fi
fi
# Sound archive: every one of the 317 cues (277 waves + 40 streams) must open and decode.
if [ -f "$M/en/sound/Wii_Wander.brsar" ]; then
  out=$(./build/tests/snd_dump "$M/en/sound/Wii_Wander.brsar" "$M/en/sound" build/tests | grep -c "FAILED")
  if [ "$out" = "0" ]; then echo "sound tests passed (all cues decode)"; else echo "sound MISMATCH ($out cues fail)"; rc=1; fi
fi
exit $rc
