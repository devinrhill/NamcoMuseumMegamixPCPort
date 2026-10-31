#include "kingdom/game/g02/cAppG02RallyXMatching.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/sMatchingData.hpp"

namespace kingdom::game::g02
{

// DOL 0x800c6a44 [audited] (job 199). The sys is made by platform/boot.cpp (G02Sys) before this runs.
void cAppG02RallyXMatching::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG02RallyXMatching());
}

// DOL 0x8019a7cc [audited]
cAppG02RallyXMatching::cAppG02RallyXMatching()
{
	RegistData("COMMON_ANITA_POINTER", "COMMON_ANITA_MATCHING", "COMMON_SCA_MATCHING_01", "COMMON_SET_MATCHING_01");
	nMenuJob() = job::RALLYX_MENU;         // +0x280
	nMultiJob() = job::RALLYX_MULTI_MENU;  // +0x284
}

// DOL 0x8019a828 [audited]: memcpy(sys (0x800c6428) + 0x64, matching data (+0x2b0), 0x154), whether confirmed or cancelled. NATIVE: the port's
// sMatchingData is the converted structure; its fields go into cG02RallyXSysExt (the multiplayer menu that reads them is not ported yet).
void cAppG02RallyXMatching::Free()
{
	if (const sMatchingData* m = pMatchingData()) cG02RallyXSysExt::SetMatching(*m);
	cAppGameBaseMenuMatching::Free();
}

}
