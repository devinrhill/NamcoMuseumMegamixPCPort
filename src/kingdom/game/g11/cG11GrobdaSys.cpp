#include "kingdom/game/g11/cG11GrobdaSys.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g11
{

// DOL 0x8000e944 [audited: SE group native, cG11SaveLoadManager is LoadRecord / SaveRecord here, test names not ported] -> 0x8000e9e0. Not ported yet: the SE group "GROUP_grobda_SE", cG11SaveLoadManager (+0x24), the 0x154-byte matching block
// (+0x04, test names "test_%02d"), the per-player blocks.
cG11GrobdaSys::cG11GrobdaSys()
{
	KingdomGlobal::KingSys->AttachArcData("g11_grobda/g11resident.lzs", true);   // DAT_80476820
	LoadRecord();
}

// DOL 0x8000eee4 [audited]: block 5, 8 bytes {flag, best}; any other flag value resets the record
void cG11GrobdaSys::LoadRecord()
{
	u8 b[8] = {};
	KingdomGlobal::KingSys->LoadGameData(5, b, 8);
	const u32 flag = (u32)b[0] << 24 | (u32)b[1] << 16 | (u32)b[2] << 8 | b[3];
	const u32 best = (u32)b[4] << 24 | (u32)b[5] << 16 | (u32)b[6] << 8 | b[7];
	m_nBest = flag == 1u ? (int)best : 0;
}

// DOL 0x8000eea0 [audited]
void cG11GrobdaSys::SaveRecord()
{
	const u32 best = (u32)m_nBest;
	const u8 b[8] = {0, 0, 0, 1, (u8)(best >> 24), (u8)(best >> 16), (u8)(best >> 8), (u8)best};
	KingdomGlobal::KingSys->SaveGameData(5, b, 8);
}

// DOL 0x8000ee28 [audited]
void cG11GrobdaSys::ResetScores()
{
	for (int& n : m_nScore) n = 0;
}

// DOL 0x8000edbc [audited]
void cG11GrobdaSys::RecordSingle(int score)
{
	if (m_nBest < score) m_nBest = score;
	SaveRecord();
}

// DOL 0x8000e980 [audited] -> 0x8000ec28: delete the save manager, unload the SE group (FUN_801376d8), the blocks, detach the archive
cG11GrobdaSys::~cG11GrobdaSys()
{
	KingdomGlobal::KingSys->DetachArcData("g11_grobda/g11resident.lzs");
}

cG11GrobdaSys* cG11GrobdaSys::Instance()
{
	return KingdomGlobal::GrobdaSys;
}

// DOL 0x8000f0c8 [audited]
void cG11GrobdaSys::JobInit()
{
	delete KingdomGlobal::GrobdaSys;
	KingdomGlobal::GrobdaSys = new cG11GrobdaSys();
	Kingdom_SetJobNo(job::GROBDA_MENU, false, 0);
}

// DOL 0x8000f10c [audited]
void cG11GrobdaSys::JobEnd()
{
	delete KingdomGlobal::GrobdaSys;
	KingdomGlobal::GrobdaSys = nullptr;
	Kingdom_SetJobNo(job::MENU, false, 0);
}

// DOL 0x8000ed64 [audited] (table 0x80476828: stage01, stage02; the words after them are other data)
const char* cG11GrobdaSys::StageArc(int n)
{
	static const char* const ARC[2] = {"g11_grobda/g11stage01.lzs", "g11_grobda/g11stage02.lzs"};
	return n >= 0 && n < 2 ? ARC[n] : ARC[0];
}

}
