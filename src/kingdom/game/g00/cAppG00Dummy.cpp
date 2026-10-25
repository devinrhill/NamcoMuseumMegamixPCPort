#include "kingdom/game/g00/cAppG00Dummy.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "gfx/DebugText.hpp"
#include "kingdom/KingdomJob.hpp"

namespace kingdom::game::g00
{

namespace
{
bool g_bSys = false;   // DAT_80479028: the 1-byte cG00DummySys (no members, empty ctor 0x800c6fb8 / dtor)

// the pointer in screen pixels: x = 256 * px + 320 (FLOAT_8047c97c / 8047c978), y = 256 * py + 200 (8047c980); the same in the game
// (FLOAT_8047b314 / 8047b310 / 8047b318). Pad 0 (get_pad1(0) +0x48 / +0x4c)
void Pointer(int& x, int& y)
{
	const GamePad::State& p = GamePad::Get(0);
	x = (int)(256.0f * p.pointerX + 320.0f);
	y = (int)(256.0f * p.pointerY + 200.0f);
}

// libc rand() as the DOL uses it (0x80211048: next = next * 0x41c64e6d + 0x3039, (next >> 16) & 0x7fff). Natively its own state (the DOL
// shares one with the particles), seeded 1
int Rand()
{
	static unsigned next = 1u;
	next = next * 0x41c64e6du + 0x3039u;
	return (int)((next >> 16) & 0x7fffu);
}
}

// DOL 0x800c6fc0
void cG00DummySys::JobInit()
{
	g_bSys = true;
	Kingdom_SetJobNo(0x0a100000u, false, 0);
}

// DOL 0x800c7010
void cG00DummySys::JobEnd()
{
	g_bSys = false;
	Kingdom_SetJobNo(0x08300000u, false, 0);
}

// ---- cAppG00DummyMenu ---------------------------------------------------------------------------------------------------------

// DOL 0x800c706c
void cAppG00DummyMenu::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG00DummyMenu());
}

// DOL 0x801915a0
cAppG00DummyMenu::cAppG00DummyMenu()
{
	SetName("g00::cAppG00DummyMenu");
	m_nState = 0;
	m_nPage = 0;
	m_nCount = 0;
}

// DOL 0x801915b4 (pad 0 trig: A 0x20, B 0x40, 2 0x80)
bool cAppG00DummyMenu::Execute(cJetTask* pParam)
{
	(void)pParam;
	Pointer(m_nPtrX, m_nPtrY);
	const uint trig = GamePad::Get(0).trig;
	auto go = [this](int state) { m_nPage = 0; m_nCount = 0; m_nState = state; };
	switch (m_nState)
	{
	case 0:
		if (m_nCount++ > 30) go(1);
		break;
	case 1:
		if (trig & GamePad::PAD_B) m_nPage = 3;
		if (m_nPage < 2)
		{
			if (m_nPage >= 0 && (trig & GamePad::PAD_A)) m_nPage++;
		}
		else if (m_nPage < 4)
			go(2);
		if (trig & GamePad::PAD_2) go(5);
		break;
	case 2:
		if (trig & GamePad::PAD_A) go(3);
		else if (trig & GamePad::PAD_2) go(5);
		break;
	case 3:
		if ((trig & GamePad::PAD_A) || (trig & GamePad::PAD_B)) go(4);
		if (trig & GamePad::PAD_2) go(2);
		break;
	case 4:
		if (m_nCount++ > 30) Kingdom_SetJobNo(0x0a200000u, false, 0);
		break;
	case 5:
		if (m_nCount++ > 30) Kingdom_SetJobNo(0x0af00000u, false, 0);
		break;
	default:
		break;
	}
	return true;
}

// DOL 0x80191844
void cAppG00DummyMenu::Draw(int nStatus, cJetTask* pParam)
{
	(void)nStatus;
	(void)pParam;
	bool fadeIn = false, fadeOut = false, skip = false, goGame = false, surprise = false, halo = false, begin = false, start = false,
	     cancel = false, frame = false, norm = false, test = false;
	switch (m_nState)
	{
	case 0: fadeIn = true; frame = true; break;
	case 1:
		if (m_nPage == 0) surprise = true;
		else if (m_nPage == 1) halo = true;
		else if (m_nPage == 3) skip = true;
		frame = true;
		break;
	case 2: norm = true; frame = true; test = true; break;
	case 3: begin = true; start = true; cancel = true; break;
	case 4: fadeOut = true; goGame = true; break;
	case 5: fadeOut = true; break;
	default: break;
	}
	const uint W = dbgtext::DEFAULT_COLOR;
	if (fadeIn) dbgtext::Print(400, 400, W, "FADE IN");
	if (fadeOut) dbgtext::Print(400, 400, W, "FADE OUT");
	if (skip) dbgtext::Print(400, 0x1a4, W, "MESSAGE SKIP");
	if (goGame)
	{
		dbgtext::Print(400, 0x1b8, W, "GO TO GAME");
		dbgtext::Print(400, 0x1b8, W, "GO TO WORLD MAP");   // the DOL prints both at the same place
	}
	if (surprise) dbgtext::Print(0x3c, 0x140, W, "[Surprise!]");
	if (halo) dbgtext::Print(0x3c, 0x140, W, "[Halo]");
	if (begin) dbgtext::Print(300, 200, W, "Do You Begin?");
	if (start) dbgtext::Print(300, 0xdc, W, "A or B TO Start");
	if (cancel) dbgtext::Print(300, 0xf0, W, "2 TO Cancel");
	if (frame)
	{
		dbgtext::Print(0x28, 0x28, W, "BACK");
		dbgtext::Print(0x14, 0x14, W, "00000");
		dbgtext::Print(0x14, 0x3c, W, "00000");
	}
	if (norm) dbgtext::Print(0xf0, 200, W, "NORM SELECT");
	if (test) dbgtext::Print(0x104, 0xdc, W, "1P TEST 1");
	dbgtext::Print(m_nPtrX - 8, m_nPtrY - 8, 0x808080ffu, "+");
}

// ---- cAppG00DummyGame ---------------------------------------------------------------------------------------------------------

// DOL 0x800c7194
void cAppG00DummyGame::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG00DummyGame());
}

// DOL 0x800c72bc (the member initialisers zero the rest: +0x2a8 0x180 bytes, +0x428 0x100 bytes)
cAppG00DummyGame::cAppG00DummyGame()
{
	SetName("g00::cAppG00DummyGame");
}

// DOL 0x800c7320: 0 count 31 frames -> 1 play -> 2 count 31 frames -> job 0x0af00000
bool cAppG00DummyGame::Execute(cJetTask* pParam)
{
	(void)pParam;
	Pointer(m_nPtrX, m_nPtrY);
	if (m_nState == 1)
	{
		m_nTime++;
		Spawn();
		Move();
		Shoot();
		if (m_nTime > 0x707 || m_nScore > 999 || m_nBullet > 999 || (GamePad::Get(0).hold & GamePad::PAD_1) != 0) m_nState = 2;
	}
	else if (m_nState == 0)
	{
		if (m_nCount++ > 30)
		{
			m_nCount = 0;
			m_nState = 1;
		}
	}
	else if (m_nState == 2)
	{
		if (m_nCount++ > 30)
		{
			m_nCount = 0;
			Kingdom_SetJobNo(0x0af00000u, false, 0);
		}
	}
	return true;
}

// DOL 0x800c76e0: from the 22nd frame on, every frame up to rand() % 4 targets start (the loop stops once the count is used up, at the
// first slot already when it is 0); a target enters at the left (x -64) or right (x 640, flying left) edge, y 20 + rand() % 380, speed
// 2 + rand() % 8, vertical speed (rand() % 8) * 0.5 toward the middle (200)
void cAppG00DummyGame::Spawn()
{
	if (m_nSpawn++ <= 0x14) return;
	int n = Rand() % 4;
	for (int i = 0; i < 16; i++)
	{
		Target& t = m_Target[i];
		if (t.nState == 0)
		{
			t.nState = 1;
			n--;
			t.x = (float)(Rand() % 0x280);
			t.y = 20.0f + (float)(Rand() % 0x17c);
			t.vx = 2.0f + (float)(Rand() % 8);
			t.vy = (float)(Rand() % 8) * 0.5f;
			t.nTimer = 0;
			t.x = t.x <= 320.0f ? -64.0f : 640.0f;
			if (320.0f < t.x) t.vx = -t.vx;
			if (200.0f < t.y) t.vy = -t.vy;
		}
		if (n < 1) break;
	}
}

// DOL 0x800c7944: flying targets move and vanish outside (-64 .. 656) x (-16 .. 416); hit ones vanish after 61 frames
void cAppG00DummyGame::Move()
{
	for (Target& t : m_Target)
	{
		if (t.nState == 1)
		{
			t.x += t.vx;
			t.y += t.vy;
			if (t.x < -64.0f || 656.0f < t.x || t.y < -16.0f || 416.0f < t.y) t.nState = 0;
		}
		else if (t.nState == 2 && t.nTimer++ > 0x3c)
			t.nState = 0;
	}
}

// DOL 0x800c7ba4: B pressed (pad trig 0x40), or A held with B repeating (+0x8), fires: one more shot, a burst at the pointer (first free
// slot), and every flying target with 0 > x - pointer x > -96 and 0 > y - pointer y > -16 is hit (+10)
void cAppG00DummyGame::Shoot()
{
	const GamePad::State& p = GamePad::Get(0);
	const bool fire = ((p.hold & GamePad::PAD_A) && (p.repeat & GamePad::PAD_B)) || (p.trig & GamePad::PAD_B);
	if (!fire) return;
	m_nBullet++;
	for (Burst& b : m_Burst)
		if (b.bActive == 0)
		{
			b.bActive = 1;
			b.x = m_nPtrX;
			b.y = m_nPtrY;
			b.nCount = 0;
			break;
		}
	for (Target& t : m_Target)
	{
		if (t.nState != 1) continue;
		const float dx = t.x - (float)m_nPtrX, dy = t.y - (float)m_nPtrY;
		if (dx < 0.0f && -96.0f < dx && dy < 0.0f && -16.0f < dy)
		{
			t.nState = 2;
			m_nScore += 10;
		}
	}
}

// DOL 0x800c7a6c: flying "\(^o^)/"; hit "/(^o^)\" green blinking (hidden every 4th frame) for 20 frames, then red blinking (every 2nd)
// until 40, then gone
void cAppG00DummyGame::DrawTargets()
{
	for (const Target& t : m_Target)
	{
		if (t.nState == 1)
			dbgtext::Print((int)t.x, (int)t.y, dbgtext::DEFAULT_COLOR, "\\(^o^)/");
		else if (t.nState == 2)
		{
			bool show = true;
			uint color;
			if (t.nTimer < 0x14)
			{
				color = 0x80ff80ffu;
				if (t.nTimer % 4 == 0) show = false;
			}
			else if (t.nTimer < 0x28)
			{
				color = 0xff8080ffu;
				if (t.nTimer % 2 == 0) show = false;
			}
			else
			{
				color = 0x80000080u;
				show = false;
			}
			if (show) dbgtext::Print((int)t.x, (int)t.y, color, "/(^o^)\\");
		}
	}
}

// DOL 0x800c7480 (the bursts advance here, in Draw, as in the DOL)
void cAppG00DummyGame::Draw(int nStatus, cJetTask* pParam)
{
	(void)nStatus;
	(void)pParam;
	const uint W = dbgtext::DEFAULT_COLOR;
	dbgtext::Print(400, 10, 0x8080ffffu, "SCORE  %6d", m_nScore);
	dbgtext::Print(400, 0x1e, 0x8080ffffu, "TIME   %6d", m_nTime);
	dbgtext::Print(400, 0x32, 0x8080ffffu, "BULLET %6d", m_nBullet);
	dbgtext::Print(m_nPtrX - 8, m_nPtrY - 8, 0x808080ffu, "+");
	if (m_nState == 0)
		dbgtext::Print(0x14, 0x14, W, "FADE %d", m_nCount);
	else if (m_nState == 1)
	{
		DrawTargets();
		for (Burst& b : m_Burst)
		{
			if (b.bActive == 0) continue;
			const int d = b.nCount * 2 - 8;
			dbgtext::Print(b.x - d, b.y - d, W, "\\");
			dbgtext::Print(d + b.x + 2, d + b.y + 2, W, "\\");
			dbgtext::Print(d + b.x, b.y - d, W, "/");
			dbgtext::Print(b.x - d + 2, d + b.y + 2, W, "/");
			if (b.nCount++ > 8) b.bActive = 0;
		}
	}
	else if (m_nState == 2)
	{
		dbgtext::Print(100, 0xb4, W, "0000 000 0   0 000  00 0  0");
		dbgtext::Print(100, 0xc4, W, "0     0  00  0  0  0   0  0");
		dbgtext::Print(100, 0xd4, W, "000   0  0 0 0  0   0  0000");
		dbgtext::Print(100, 0xe4, W, "0     0  0  00  0    0 0  0");
		dbgtext::Print(100, 0xf4, W, "0    000 0   0 000 00  0  0");
		dbgtext::Print(0x14, 0x14, W, "FADE %d", m_nCount);
	}
}

}
