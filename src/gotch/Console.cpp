#include "gotch/Console.hpp"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

#include "WiiPad.hpp"
#include "WiiSystem.hpp"
#include "gfx/Gfx.hpp"
#include "gotch/Draw.hpp"
#include "gotch/NewRallyX.hpp"
#include "gotch/PacMan.hpp"
#include "gotch/RallyX.hpp"
#include "gotch/Sound.hpp"

namespace gotch
{
namespace console
{
namespace
{
gfx::TexId g_tex = gfx::NO_TEX;
std::vector<std::uint8_t> g_rgba;
std::unique_ptr<CWiiSound> g_wiiSound;  // CConsole +0xc0
bool g_running = false;

// the ported machines behind one interface (every game namespace exports the same calls)
struct Game
{
	const char* name;
	bool portrait;   // the monitor is turned 90 degrees (PACMAN); RALLYX shows its picture as is
	void (*setSoundBufferClear)(void (*)());
	bool (*isSetup)();
	bool (*setup)();
	void (*hotReset)();
	const CDraw& (*draw)();
	void (*storageFrameUpdate)();
	int (*execute1V)();
	PrimaryStream* (*primary)();
	void (*exitRequest)();
	void (*exitMachine)();
};
const Game kGames[] = {
    {"PAC-MAN", true, pacman::SetSoundBufferClear, pacman::IsSetup, pacman::Setup, pacman::HotReset, pacman::Draw, pacman::StorageFrameUpdate,
     pacman::Execute1V, pacman::Primary, pacman::ExitRequest, pacman::ExitMachine},
    {"RALLY-X", false, rallyx::SetSoundBufferClear, rallyx::IsSetup, rallyx::Setup, rallyx::HotReset, rallyx::Draw, rallyx::StorageFrameUpdate,
     rallyx::Execute1V, rallyx::Primary, rallyx::ExitRequest, rallyx::ExitMachine},
    {"NEW RALLY-X", false, newrallyx::SetSoundBufferClear, newrallyx::IsSetup, newrallyx::Setup, newrallyx::HotReset, newrallyx::Draw,
     newrallyx::StorageFrameUpdate, newrallyx::Execute1V, newrallyx::Primary, newrallyx::ExitRequest, newrallyx::ExitMachine},
};
const Game* g_game = nullptr;   // vol 2 game number = index - 9
}

bool IsPorted(int gameIndex) { return gameIndex >= 9 && gameIndex <= 11; }

bool Start(int gameIndex)
{
	if (!IsPorted(gameIndex))
	{
		std::fprintf(stderr, "arcade: game index %d is not ported yet (ported: 9 = PAC-MAN, 10 = RALLY-X, 11 = NEW RALLY-X)\n", gameIndex);
		return false;
	}
	g_game = &kGames[gameIndex - 9];
	g_game->setSoundBufferClear([] {
		if (g_wiiSound) g_wiiSound->BufferClear();
	});
	if (!g_game->isSetup() && !g_game->setup()) return false;
	g_game->hotReset();
	const CDraw& d = g_game->draw();
	// PACMAN's monitor is turned (portrait): the picture is ScreenH wide and ScreenW high
	g_rgba.assign((std::size_t)d.ScreenW() * d.ScreenH() * 4, 0);
	// (one game per process: the texture keeps the first game's shape)
	if (g_tex == gfx::NO_TEX)
		g_tex = g_game->portrait ? gfx::CreateTexture(d.ScreenH(), d.ScreenW(), g_rgba.data()) : gfx::CreateTexture(d.ScreenW(), d.ScreenH(), g_rgba.data());
	// the console's sound (CConsole ctor RSO2 0x80d0022c): CWiiSound(0, 32000000, 60000), SetSoundOutMode(stereo: the console's sound setting,
	// UNVERIFIED which value it has natively), Play
	if (WiiSystem::AudioEnabled() && !g_wiiSound)
	{
		g_wiiSound = std::make_unique<CWiiSound>(32000000u, 60000u);
		g_wiiSound->SetSoundOutMode(1);
	}
	if (g_wiiSound) g_wiiSound->Play();
	g_running = g_tex != gfx::NO_TEX;
	return g_running;
}

bool Frame()
{
	if (!g_running) return false;
	// the pads go through CGAMEIO (ConvertIO); the start and pause menus are the module's (WII_MENU). Stand-in for the HOME menu, which is
	// not ported: HOME = its exit
	if (WiiPad::Pressed(0) & WiiPad::BTN_HOME) g_game->exitRequest();

	// the frame as the console runs it (UNVERIFIED order: PACMAN::Execute's loop is not decoded): OnWaitVSync, the machine, OnRender
	g_game->storageFrameUpdate();   // CDraw::WaitVSync RSO2 0x80d06d9c: CDataStorage::FrameUpdate, DEMODoneRender, OnWaitVSync
	if (g_wiiSound) Console_OnWaitVSync(*g_wiiSound);
	const int stopped = g_game->execute1V();   // 1 = the machine stopped (CGAMEIO::ExitCheck)
	if (g_wiiSound) Console_OnRender(*g_wiiSound, g_game->primary());
	if (stopped != 0)
	{
		std::fprintf(stderr, "arcade: the machine stopped (exit)\n");
		g_running = false;
	}
	return stopped == 0;
}

void Draw()
{
	if (g_tex == gfx::NO_TEX || g_game == nullptr) return;
	const CDraw& d = g_game->draw();
	const int w = d.ScreenW(), h = d.ScreenH();
	const std::vector<std::uint32_t>& f = d.Frame();
	if (g_game->portrait)
	{
		// landscape frame (x, y) -> portrait (H - 1 - y, x): the cabinet's monitor turned 90 degrees (UNVERIFIED direction)
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				const std::uint32_t c = f[(std::size_t)y * w + x];
				std::uint8_t* o = &g_rgba[((std::size_t)x * h + (h - 1 - y)) * 4];
				o[0] = (std::uint8_t)(c >> 16);
				o[1] = (std::uint8_t)(c >> 8);
				o[2] = (std::uint8_t)c;
				o[3] = 255;
			}
		}
		gfx::UpdateTexture(g_tex, h, w, g_rgba.data());
		// fit the portrait picture to the 480-high game rectangle (centre origin, y up)
		const float sy = 240.0f, sx = 240.0f * (float)h / (float)w;
		const float xy[8] = {-sx, sy, -sx, -sy, sx, sy, sx, -sy};
		const float uv[8] = {0, 0, 0, 1, 1, 0, 1, 1};
		gfx::DrawQuad(g_tex, xy, uv, 255, 255, 255, 255, gfx::BLEND_ALPHA, false);
		return;
	}
	for (std::size_t i = 0; i < f.size(); i++)
	{
		const std::uint32_t c = f[i];
		std::uint8_t* o = &g_rgba[i * 4];
		o[0] = (std::uint8_t)(c >> 16);
		o[1] = (std::uint8_t)(c >> 8);
		o[2] = (std::uint8_t)c;
		o[3] = 255;
	}
	gfx::UpdateTexture(g_tex, w, h, g_rgba.data());
	// landscape: the picture fills the whole 640 x 480 game rectangle (centre origin, y up); UNVERIFIED: the Wii's GX viewport stretches the 288 x 224
	// picture to the 4:3 screen
	const float sx = 320.0f, sy = 240.0f;
	const float xy[8] = {-sx, sy, -sx, -sy, sx, sy, sx, -sy};
	const float uv[8] = {0, 0, 0, 1, 1, 0, 1, 1};
	gfx::DrawQuad(g_tex, xy, uv, 255, 255, 255, 255, gfx::BLEND_ALPHA, false);
}

void Stop()
{
	if (g_game != nullptr && g_game->isSetup()) g_game->exitMachine();
	if (g_wiiSound) g_wiiSound->Stop();
	g_running = false;
}
}
}
