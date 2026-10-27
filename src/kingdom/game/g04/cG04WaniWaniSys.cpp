#include "kingdom/game/g04/cG04WaniWaniSys.hpp"

#include <cstring>

#include "KingdomGlobal.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g04
{

namespace
{
const char* const ARC[3] = {"g04_waniwani/g04resident.lzs", "g04_waniwani/g04resident2.lzs", "g04_waniwani/g04resident3.lzs"};   // DAT_8047ca44..4c
}

// DOL 0x80199a7c: the three resident archives, the SE group GROUP_wani_SE (KingSys +0xa54, 0x80137648: NOT PORTED, sounds play by name), the
// matching block (1 player; names DAT_8037c188, +0x44 = 6, index / work = the player number)
cG04WaniWaniSys::cG04WaniWaniSys()
{
	for (const char* a : ARC) KingdomGlobal::KingSys->AttachArcData(a, true);
	m_b164 = false;
	m_nMode = 1;
	m_nHighScore = 0;
	std::memset(&m_Matching, 0, sizeof m_Matching);
	m_Matching.nPlayer = 1;
	for (int i = 0; i < 4; i++)
	{
		std::strcpy(m_Matching.player[i].szName, "test");   // DAT_8037c188
		m_Matching.player[i].n44 = 6;
		m_Matching.player[i].nIndex = i;
		m_Matching.player[i].nWork = i;
		m_Matching.player[i].n50 = 0;
	}
	m_n160 = 0;
	m_n00 = 1;
	m_b165 = false;
}

// DOL 0x80199b64: the SE group (0x801376d8, not ported) and the three archives
cG04WaniWaniSys::~cG04WaniWaniSys()
{
	for (const char* a : ARC) KingdomGlobal::KingSys->DetachArcData(a);
}

// DOL 0x8019a07c: new sys, then the Nunchuk card (job 231)
void cG04WaniWaniSys::JobInit()
{
	delete KingdomGlobal::WaniWaniSys;
	KingdomGlobal::WaniWaniSys = new cG04WaniWaniSys();
	Kingdom_SetJobNo(job::WANIWANI_NUNCHUK, false, 0);
}

// DOL 0x8019a0e0: delete the sys, back to the hub
void cG04WaniWaniSys::JobEnd()
{
	delete KingdomGlobal::WaniWaniSys;
	KingdomGlobal::WaniWaniSys = nullptr;
	Kingdom_SetJobNo(job::MENU, false, 0);
}

// DOL 0x80199dfc: game data block 3 = {flag, version, score}; an empty or old block starts at 0
void cG04WaniWaniSys::LoadRecord()
{
	u8 be[12] = {};   // DAT_8048a730..38, big-endian as on the Wii
	KingdomGlobal::KingSys->LoadGameData(3u, be, sizeof be);
	s32 rec[3];
	for (int i = 0; i < 3; i++) rec[i] = (s32)((u32)be[4 * i] << 24 | (u32)be[4 * i + 1] << 16 | (u32)be[4 * i + 2] << 8 | be[4 * i + 3]);
	if (rec[0] == 0 || rec[1] != 0x20070723)
	{
		rec[2] = 0;
		rec[0] = 1;
		rec[1] = 0x20070723;
	}
	m_nHighScore = rec[2];
}

}

namespace kingdom::game::g04
{

// DOL 0x80199db4: the high score into game data block 3 ({1, 0x20070723, score}, big-endian)
void cG04WaniWaniSys::SaveRecord()
{
	const s32 rec[3] = {1, 0x20070723, m_nHighScore};
	u8 be[12];
	for (int i = 0; i < 3; i++)
	{
		be[4 * i] = (u8)((u32)rec[i] >> 24);
		be[4 * i + 1] = (u8)((u32)rec[i] >> 16);
		be[4 * i + 2] = (u8)((u32)rec[i] >> 8);
		be[4 * i + 3] = (u8)rec[i];
	}
	KingdomGlobal::KingSys->SaveGameData(3u, be, sizeof be);
}

}
