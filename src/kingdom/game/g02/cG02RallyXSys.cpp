#include "kingdom/MslRand.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"

#include <cstdlib>

#include "KingdomGlobal.hpp"
#include "kingdom/KingdomJob.hpp"

namespace kingdom::game::g02
{

const char* const cG02RallyXSys::STAGE_ARC[31] = {
	"g02_rallyx/g02wxxsxx.lzs", "g02_rallyx/g02w01s01.lzs", "g02_rallyx/g02w01s02.lzs", "g02_rallyx/g02w01s03.lzs", "g02_rallyx/g02w01s04.lzs",
	"g02_rallyx/g02w01s05.lzs", "g02_rallyx/g02w01s06.lzs", "g02_rallyx/g02w02s01.lzs", "g02_rallyx/g02w02s02.lzs", "g02_rallyx/g02w02s03.lzs",
	"g02_rallyx/g02w02s04.lzs", "g02_rallyx/g02w02s05.lzs", "g02_rallyx/g02w02s06.lzs", "g02_rallyx/g02w03s01.lzs", "g02_rallyx/g02w03s02.lzs",
	"g02_rallyx/g02w03s03.lzs", "g02_rallyx/g02w03s04.lzs", "g02_rallyx/g02w03s05.lzs", "g02_rallyx/g02w03s06.lzs", "g02_rallyx/g02w04s01.lzs",
	"g02_rallyx/g02w04s02.lzs", "g02_rallyx/g02w04s03.lzs", "g02_rallyx/g02w04s04.lzs", "g02_rallyx/g02w04s05.lzs", "g02_rallyx/g02w04s06.lzs",
	"g02_rallyx/g02w11s01.lzs", "g02_rallyx/g02w11s02.lzs", "g02_rallyx/g02w11s03.lzs", "g02_rallyx/g02w11s04.lzs", "g02_rallyx/g02w11s05.lzs",
	"g02_rallyx/g02w11s06.lzs"};

const int cG02RallyXSys::FLAG_COUNT[6] = {1, 5, 10, 15, 20, 0};            // DOL 0x80371160 [audited]
const int cG02RallyXSys::TIME_MS[4] = {60000, 180000, 300000, 0};          // DOL 0x80371178 [audited]

// DOL 0x800c54e0 [audited: the SE group GROUP_rallyx_SE (0x80137648) is not loaded natively]. The save block 1 and cParamDefaultRecord live in cG02RallyXRecord (loaded on first use); the block
// (0x250 bytes, version 0x20070824 at +4, reset by FUN_800c6024).
cG02RallyXSys::cG02RallyXSys()
{
	KingdomGlobal::KingSys->AttachArcData("g02_rallyx/g02resident.lzs", true);
	for (int i = 0; i < 4; i++)   // block i at +0x14 + 0x14 * i
	{
		m_nPlayerWork[i][0] = 2;
		m_nPlayerWork[i][1] = 6;
		m_nPlayerWork[i][2] = i;
		m_nPlayerWork[i][3] = 6;
		m_nPlayerWork[i][4] = kingdom::MslRand() % 6;
	}
}

// DOL 0x800c5690 [audited] (UNVERIFIED body: detaches the resident archive)
cG02RallyXSys::~cG02RallyXSys()
{
	KingdomGlobal::KingSys->DetachArcData("g02_rallyx/g02resident.lzs");
}

// DOL 0x800c6444 [audited]
void cG02RallyXSys::JobInit()
{
	delete KingdomGlobal::RallyXSys;
	KingdomGlobal::RallyXSys = new cG02RallyXSys();
	Kingdom_SetJobNo(job::RALLYX_MENU, false, 0);
}

// DOL 0x800c64a8 [audited]
void cG02RallyXSys::JobEnd()
{
	delete KingdomGlobal::RallyXSys;
	KingdomGlobal::RallyXSys = nullptr;
	Kingdom_SetJobNo(job::MENU, false, 0);
}

// DOL 0x800c5ba8 [audited]
bool cG02RallyXSys::IsBossStage() const
{
	const int s = m_nStageIdx == 0 ? 0 : (m_nStageIdx - 1) % 6 + 1;
	return s == 6;
}

// DOL 0x800c5788 [audited]
int cG02RallyXSys::nWorld() const
{
	const int i = m_nStageIdx;
	if (i > 0x12) return i > 0x1c ? 0 : (i < 0x19 ? 4 : 0xb);
	if (i > 6) return i < 0xd ? 2 : 3;
	return i < 1 ? 0 : 1;
}

// DOL 0x800c5808 [audited]
int cG02RallyXSys::nStageInWorld() const
{
	return m_nStageIdx == 0 ? 0 : (m_nStageIdx - 1) % 6 + 1;
}

// DOL 0x800c5a1c [audited]
void cG02RallyXSys::AttachStageArc() const
{
	KingdomGlobal::KingSys->AttachArcData("g02_rallyx/g02game.lzs", true);
	if ((unsigned)m_nStageIdx < 31) KingdomGlobal::KingSys->AttachArcData(STAGE_ARC[m_nStageIdx], true);
}

void cG02RallyXSys::DetachStageArc() const
{
	KingdomGlobal::KingSys->DetachArcData("g02_rallyx/g02game.lzs");
	if ((unsigned)m_nStageIdx < 31) KingdomGlobal::KingSys->DetachArcData(STAGE_ARC[m_nStageIdx]);
}

}
