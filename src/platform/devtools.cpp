#include "platform/devtools.hpp"
#include "platform/devtools_g03gray.hpp"

#include <cstdio>
#include <cstdlib>

namespace devtools
{
namespace
{
bool g_previewAll = std::getenv("MEGAMIX_PREVIEW_ALL") != nullptr;
}

bool PreviewAll() { return g_previewAll; }

namespace
{
bool g_hasStart = false;
float g_start[3] = {};
}
void SetG03Start(float x, float y, float z) { g_hasStart = true; g_start[0] = x; g_start[1] = y; g_start[2] = z; }
bool G03StartOverride(float out[3])
{
	if (G03EditStart(out)) return true;   // the dev window editor's test start wins
	if (G03GrayStart(out)) return true;   // a graybox stage starts on its first primitive
	static bool envRead = false;
	if (!envRead)
	{
		envRead = true;
		if (const char* e = std::getenv("MEGAMIX_G03_START")) { float x, y, z; if (std::sscanf(e, "%f,%f,%f", &x, &y, &z) == 3) SetG03Start(x, y, z); }
	}
	if (!g_hasStart) return false;
	for (int i = 0; i < 3; i++) out[i] = g_start[i];
	return true;
}
void SetPreviewAll(bool on) { g_previewAll = on; }
namespace
{
int g_kill = -2;   // -2: environment not read yet
}
void SetG03KillFrame(int frame) { g_kill = frame; }
int G03KillFrame()
{
	if (g_kill == -2)
	{
		const char* e = std::getenv("MEGAMIX_G03_KILL");
		g_kill = e ? std::atoi(e) : -1;
	}
	return g_kill;
}
namespace
{
int g_life = -2;
}
void SetG03Life(int lives) { g_life = lives; }
int G03Life()
{
	if (g_life == -2)
	{
		const char* e = std::getenv("MEGAMIX_G03_LIFE");
		g_life = e ? std::atoi(e) : -1;
	}
	return g_life;
}
namespace
{
int g_cookies = -2;
}
void SetG03Cookies(int n) { g_cookies = n; }
namespace
{
bool g_noGates = std::getenv("MEGAMIX_G03_NO_GATES") != nullptr;
}
static bool g_noDamage = std::getenv("MEGAMIX_G03_NO_DAMAGE") != nullptr;
bool G03NoDamage() { return g_noDamage; }
void SetG03NoDamage(bool on) { g_noDamage = on; }
static bool g_showHit = std::getenv("MEGAMIX_G03_SHOW_HIT") != nullptr;
static bool g_showGimmickHit = std::getenv("MEGAMIX_G03_SHOW_GIMMICK_HIT") != nullptr;
static bool g_fly = std::getenv("MEGAMIX_G03_FLY") != nullptr;
static bool g_skipTitle = std::getenv("MEGAMIX_G03_SKIP_TITLE") != nullptr;
static bool g_bgCollision = std::getenv("MEGAMIX_G03_BG_COLLISION") != nullptr;
bool G03BgCollision() { return g_bgCollision; }
static bool g_noMenuWarp = std::getenv("MEGAMIX_G03_NO_MENU_WARP") != nullptr;
bool G03NoMenuWarp() { return g_noMenuWarp; }
static bool g_jump = std::getenv("MEGAMIX_G03_JUMP") != nullptr;
static float g_jumpStrength = std::getenv("MEGAMIX_G03_JUMP") && std::atof(std::getenv("MEGAMIX_G03_JUMP")) > 0.0 ? (float)std::atof(std::getenv("MEGAMIX_G03_JUMP")) : 0.35f;
bool G03Jump() { return g_jump; }
float G03JumpStrengthValue() { return g_jumpStrength; }
void SetG03Jump(bool on) { g_jump = on; }
void SetG03JumpStrength(float s) { g_jump = true; if (s > 0.0f) g_jumpStrength = s; }
static bool g_freecam = std::getenv("MEGAMIX_G03_FREECAM") != nullptr;
bool G03Freecam() { return g_freecam; }
void SetG03Freecam(bool on) { g_freecam = on; }
void SetG03NoMenuWarp(bool on) { g_noMenuWarp = on; }
void SetG03BgCollision(bool on) { g_bgCollision = on; }
bool G03SkipTitle() { return g_skipTitle; }
void SetG03SkipTitle(bool on) { g_skipTitle = on; }
bool G03FlyEnabled() { return g_fly; }
void SetG03Fly(bool on) { g_fly = on; }
bool G03ShowHit() { return g_showHit; }
void SetG03ShowHit(bool on) { g_showHit = on; }
bool G03ShowGimmickHit() { return g_showGimmickHit; }
void SetG03ShowGimmickHit(bool on) { g_showGimmickHit = on; }
bool G03NoGates() { return g_noGates; }
void SetG03NoGates(bool on) { g_noGates = on; }
int G03Cookies()
{
	if (g_cookies == -2)
	{
		const char* e = std::getenv("MEGAMIX_G03_COOKIES");
		g_cookies = e ? std::atoi(e) : -1;
	}
	return g_cookies;
}

namespace
{
int g_g01Stage = std::getenv("MEGAMIX_G01_STAGE") ? std::atoi(std::getenv("MEGAMIX_G01_STAGE")) : -1;
}
int G01Stage() { return g_g01Stage; }
void SetG01Stage(int n) { g_g01Stage = n; }
bool g_g01Invincible = std::getenv("MEGAMIX_G01_INVINCIBLE") != nullptr;
bool G01Invincible() { return g_g01Invincible; }
void SetG01Invincible(bool on) { g_g01Invincible = on; }
bool g_g01AutoAim = std::getenv("MEGAMIX_G01_AUTOAIM") != nullptr;
bool G01AutoAim() { return g_g01AutoAim; }
static int g_frameNo = 0;
int FrameNo() { return g_frameNo; }
void TickFrame() { g_frameNo++; }
void SetG01AutoAim(bool on) { g_g01AutoAim = on; }
int g_g02Stage = std::getenv("MEGAMIX_G02_STAGE") ? std::atoi(std::getenv("MEGAMIX_G02_STAGE")) : -1;
int G02Stage() { return g_g02Stage; }
void SetG02Stage(int n) { g_g02Stage = n; }
int g_g04Time = std::getenv("MEGAMIX_G04_TIME") ? std::atoi(std::getenv("MEGAMIX_G04_TIME")) : -1;
int G04Time() { return g_g04Time; }
void SetG04Time(int n) { g_g04Time = n; }
}
