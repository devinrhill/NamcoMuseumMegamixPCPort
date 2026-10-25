#pragma once

// Developer-only switches of the native build. NOT part of the 1:1 game: nothing here exists in main.dol. The game code reaches these through a few
// marked one-line hooks ("devtools:" comments) so they are easy to find and remove. Everything is off by default and never writes to the save.
namespace devtools
{
// --preview-all / MEGAMIX_PREVIEW_ALL=1: every Pac-N-Roll / Pac-Motos / Galaga stage reads as unlocked (all worlds, all stages, the arrows to switch worlds), so each
// menu world can be looked at. Progress is not written while this is on (cSaveLoadMan::SaveData is skipped), the save is untouched.
bool PreviewAll();
// --g03-start X Y Z / MEGAMIX_G03_START="x,y,z": Pac-N-Roll starts Pac-Man at this stage position (to test a part of a stage without rolling there)
bool G03StartOverride(float out[3]);
void SetG03Start(float x, float y, float z);
void SetPreviewAll(bool on);
// --g03-kill N / MEGAMIX_G03_KILL=N: Pac-N-Roll deals lethal damage to Pac-Man when the game frame counter reaches N (tests the miss / game over flow); -1 = off
int G03KillFrame();
// Extra lives Pac-Man starts a Pac-N-Roll stage with (-1 = the save's value)
int G03Life();
// Pac-Dots Pac-Man already carries when a Pac-N-Roll stage starts (-1 = none)
int G03Cookies();
void SetG03Cookies(int n);
void SetG03Life(int lives);
void SetG03KillFrame(int frame);
// --g01-stage N / MEGAMIX_G01_STAGE=N: a Galaga stage job entered directly (--job 0x0b300000) plays stage N (1..10) instead of 1; -1 = off
int G01Stage();
void SetG01Stage(int n);
// --g01-invincible / MEGAMIX_G01_INVINCIBLE=1: Galaga's Pac-Man takes no damage (to test a whole course)
bool G01Invincible();
void SetG01Invincible(bool on);
// --g01-autoaim / MEGAMIX_G01_AUTOAIM=1: Galaga shots aim at the nearest enemy in front of the camera (to test a whole course)
bool G01AutoAim();
// the number of the frame being run (0 = the first; the same count --shot / --press use): debug prints only
int FrameNo();
void TickFrame();
void SetG01AutoAim(bool on);
// --g02-stage N / MEGAMIX_G02_STAGE=N: the Rally-X stage job entered directly (--job 0x0c300000) plays stage index N (1..30, 0 = multiplayer)
int G02Stage();
void SetG02Stage(int n);
// --g04-time S / MEGAMIX_G04_TIME=S: Gator Panic's play time in seconds instead of CSV_GAME's 120 (tests the time up / result / retry quickly)
int G04Time();
void SetG04Time(int n);
}
