#pragma once

// Developer-only switches of the native build. NOT part of the 1:1 game: nothing here exists in main.dol. The game code reaches these through a few
// marked one-line hooks ("devtools:" comments) so they are easy to find and remove. Everything is off by default and never writes to the save.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace kingdom::game::g03 { class cTaskCharPacman; class cTaskBg; class cTaskBgHit; class cTaskStage; }

namespace devtools
{
// the dev window's Jobs tab (devui, --devui / F9): switch to any job of the master table at run time (src/platform/devtools_jobs.cpp)
void RegisterJobsPanel();
// the dev window's Menu tab, shown while a game's menu job runs: unlock stages, start any stage (src/platform/devtools_menu.cpp)
void RegisterMenuPanel();
void RegisterSavePanel();          // the dev window's Save tab: edit a copy of the save data (devtools_save.cpp)
// the dev window's Editor tab, shown in a Pac-N-Roll stage (job PACNROLL_GAME): a stage placement editor (port of unpac's pacnroll editor,
// src/platform/devtools_g03edit.cpp). G03EditOverride swaps an archive entry for the edited copy (cKingdomSys::SearchDataEx), G03EditCamera
// replaces the camera with the editor's orbit camera (NativeScene::SetCamera), G03EditDraw3D draws the handles (inside the 3D pass)
void RegisterG03EditPanel();
void G03EditOverride(const std::string& key, const std::uint8_t*& data, std::size_t& size);
void G03EditCamera(float eye[3], float target[3], float& farZ);
// the editor's "Draw everything": cTaskModelEx::Draw skips the game camera's view-volume test (every item / gimmick draws) while the Editor tab is shown
bool G03EditNoCull();
// the editor's test start (editor only, never saved): Pac-Man starts there in the edited round (read by G03StartOverride before --g03-start)
bool G03EditStart(float out[3]);
void G03EditDraw3D();
// the editor's "Hide the HUD" checkbox: the stage's consoles (cTaskConsoleBase::Draw) draw nothing while it is ticked and the stage runs
bool G03HideHud();
// --preview-all / MEGAMIX_PREVIEW_ALL=1: every Pac-N-Roll / Pac-Motos / Galaga / Rally-X stage reads as unlocked (all worlds, all stages, the arrows to switch worlds), so each
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
// --g03-no-gates / MEGAMIX_G03_NO_GATES=1: Pac-N-Roll stages spawn none of their Golvis (Pac-Dot) gates, so the whole course is open
bool G03NoGates();
// --g03-no-damage / MEGAMIX_G03_NO_DAMAGE=1: Pac-N-Roll's Pac-Man loses no HP from hits (the hit sound / knockback stay; falling off the stage still misses)
bool G03NoDamage();
void SetG03NoDamage(bool on);
// --g03-show-hit / MEGAMIX_G03_SHOW_HIT=1: Pac-N-Roll draws the stage's HIT geometry (the collision faces of the BG, not the gimmicks) as wireframe,
// coloured by face kind (see docs/OPTIONS.md for the colours)
bool G03ShowHit();
void SetG03ShowHit(bool on);
// --g03-show-gimmick-hit / MEGAMIX_G03_SHOW_GIMMICK_HIT=1: Pac-N-Roll draws only the gimmicks' collision faces and the boxes / spheres their own
// damage / area tests use (white = hurts Pac-Man)
bool G03ShowGimmickHit();
void SetG03ShowGimmickHit(bool on);
// a world-space box / sphere a gimmick tested this frame (recorded where the test runs, drawn by DrawOverlay3D); rgba = 0xRRGGBBAA
void G03HitBox(const float mn[3], const float mx[3], unsigned rgba);
void G03HitSphere(const float c[3], float r, unsigned rgba);
// called inside the 3D pass (NativeScene::Flush): draws the hit overlays that are switched on (src/platform/devtools_g03.cpp)
void DrawOverlay3D();
// --g03-fly / MEGAMIX_G03_FLY=1: Pac-N-Roll's Pac-Man flies instead of rolling: no gravity, no collision, no fall-off miss. Nunchuk stick = move
// on X / Z relative to the camera, C = up, Z = down, B held = 3x speed, A held = 2x (stacks). G03FlyStep replaces his control / gravity / collision for one play frame
// Wiimote 1 switches between flying and normal rolling (starts flying); G03Fly = flying this frame, G03FlyEnabled = --g03-fly given
bool G03Fly();
bool G03FlyEnabled();
void SetG03Fly(bool on);
void G03FlyStep(kingdom::game::g03::cTaskCharPacman* pac);
// --g03-skip-title / MEGAMIX_G03_SKIP_TITLE=1: a Pac-N-Roll stage starts without the stage name card (the game's own retry path:
// TitleStart(false), straight to the start); the opening camera still plays
bool G03SkipTitle();
void SetG03SkipTitle(bool on);
// --g03-bg-collision / MEGAMIX_G03_BG_COLLISION=1: Pac-N-Roll's collision comes from the models that are DRAWN instead of the hidden hit models:
// the stage's "hit" models are not loaded, every "bg" model gets a collision object built from its own triangles, and every gimmick / box / gate
// that has a hit model takes its faces from its visual model instead (its hit model is still loaded: gimmicks read node positions from it).
// Animated models are re-skinned every frame. Faces are only floor (normal up to 60 degrees from up) or wall: no bounce / dash / jump / magma /
// poison / water / grass / camera faces. Translucent (DrawXlu) and billboard draws are skipped. src/platform/devtools_g03.cpp
bool G03BgCollision();
// --g03-no-menu-warp / MEGAMIX_G03_NO_MENU_WARP=1: the hub (g03 cAppMenu, the top menu) never warps to a game: rolling onto a door's floor does
// nothing (cAppMenu::Select reads no door), so the whole garden can be rolled around
bool G03NoMenuWarp();
// --g03-jump / --g03-jump-strength S / MEGAMIX_G03_JUMP=S: Pac-N-Roll's Pac-Man jumps with C (Nunchuk) while he is on a floor: the upward speed S
// (default 0.35) goes through the jump vector the game's bounce pads use; B held = 3x, A held = 2x (both = 6x), as --g03-fly's speed. G03JumpTrigger
// returns the speed to jump with this frame, 0 = no jump
bool G03Jump();
void SetG03Jump(bool on);
void SetG03JumpStrength(float s);
float G03JumpTrigger(bool onFloor);
float G03JumpStrengthValue();
// --g03-freecam / MEGAMIX_G03_FREECAM=1: the Pac-N-Roll camera turns with no limits (no camera-area ranges, no wall checks, no wall escape: the
// "no camera control" sign never shows; the camera may go through walls); right stick Y (or D-pad up / down) raises / lowers the look-at point.
// G03FreecamTargetY(distance) = the look-at height offset, updated once per frame.
bool G03Freecam();
void SetG03Freecam(bool on);
float G03FreecamTargetY(float distance);
// --g03-explore ARCHIVE ENTRY[,ENTRY...] (+ --g03-explore-anim ENTRY[,...], --g03-explore-scale S): roll Pac-N-Roll's Pac-Man around arbitrary models
// (any game's archive). The Pac-N-Roll game job runs (round from --round, default W1S1) but its stage is replaced: the models are placed at the
// origin (a Pac-Motos archive's MDL_BG_n where its CSV_BG puts them) as BG models; ENTRY+ANIM+ANIM gives a model its own animations, --g03-explore-anim
// gives every model the same ones. Collision is built from them as --g03-bg-collision does, and the stage's placement data (items, ghosts, gates,
// boxes, checkpoints, water, wind, fall areas, camera areas, goal) is hidden. Pac-Man starts on the highest floor under the models' centre (or --g03-start) and is
// put back there when he falls 30 units below the models. The other g03 devtools work.
bool G03Explore();
void SetG03Explore(const std::string& archive, const std::string& entries);
void SetG03ExploreAnims(const std::string& entries);
void SetG03ExploreScale(float s);
// cAppG03PacnRollGame::Init, before the stage archive is attached (the stage's entries win over the explore archive's)
void G03ExploreAttach();
// cKingdomSys::SearchDataEx: true = act as if the entry were missing (the stage placement data)
bool G03ExploreHidesData(const std::string& key);
// cAppG03PacnRollGame::Dispose: the explore models instead of the stage's bg / hit / obj nodes
void G03ExploreStage(kingdom::game::g03::cTaskStage* stage, std::vector<kingdom::game::g03::cTaskBg*>& bgs, std::vector<kingdom::game::g03::cTaskBgHit*>& hits);
// cTaskCharPacman play step: true = he fell below the models and was put back at the start this frame
bool G03ExploreFell(kingdom::game::g03::cTaskCharPacman* pac);
void SetG03NoMenuWarp(bool on);
void SetG03BgCollision(bool on);
// cAppG03PacnRollGame::Dispose: one collision object per BG model (connected to `stage`, appended to `out`) instead of the "hit" models
void G03BgCollisionStage(kingdom::game::g03::cTaskStage* stage, const std::vector<kingdom::game::g03::cTaskBg*>& bgs, std::vector<kingdom::game::g03::cTaskBgHit*>& out);
// cTaskBgHit::RegistHitData: the object's faces from its own visual model (only when it found a hit model)
void G03BgCollisionReplace(kingdom::game::g03::cTaskBgHit* obj);
// cTaskBgHit::UpdateMatrix: re-skins the faces of an object whose model is posed by a CHR clip
void G03BgCollisionRefresh(kingdom::game::g03::cTaskBgHit* obj);
void SetG03NoGates(bool on);
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
// --arcade NAME: runs one of the arcade collection's classic games (gotch, src/gotch/) in the window instead of the Megamix boot; only "pacman" so far.
// The game's own Wii menu is not ported: PLUS = 1P start, MINUS = 2P start (UNVERIFIED stand-in), the D-pad (physical directions) = the joystick
const std::string& Arcade();
void SetArcade(const std::string& name);
bool ArcadeInit();   // false when the game is unknown or its data is missing from the player's module
void ArcadeFrame();  // one video frame of the machine, drawn into the 640 x 480 game rectangle
}
