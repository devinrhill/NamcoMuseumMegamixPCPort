#include "kingdom/game/g11/cAppG11GrobdaMatching.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/sMatchingData.hpp"

namespace kingdom::game::g11
{

// DOL 0x8000fa64 (job 342)
void cAppG11GrobdaMatching::JobInit()
{
	if (KingdomGlobal::GrobdaSys == nullptr) KingdomGlobal::GrobdaSys = new cG11GrobdaSys();   // native: a direct --job entry has no sys yet
	KingdomGlobal::SysCore->RegistMainTask(new cAppG11GrobdaMatching());
}

// DOL 0x8014a450
cAppG11GrobdaMatching::cAppG11GrobdaMatching()
{
	RegistData("BIN_COMMON_ANT_CURSOR", "BIN_COMMON_ANT_MATCHING", "BIN_COMMON_CAMERA_MATCHING", "BIN_COMMON_MDL_MATCHING");
	nMenuJob() = 0x15200000u;    // +0x280
	nMultiJob() = 0x15200000u;   // +0x284
}

// DOL 0x8014a4a8: +0x29a (confirmed) -> the matching data into the sys (memcpy 0x154 at sys +4, port: player count and the pads' controller types), sys +0x20 = 1;
// else 1 player (FUN_8000ee50(sys, 1)) and sys +0x20 = 0
void cAppG11GrobdaMatching::Free()
{
	cG11GrobdaSys* sys = KingdomGlobal::GrobdaSys;
	if (sys != nullptr)
	{
		const sMatchingData* m = pMatchingData();
		if (bConfirmDecide() && m != nullptr && m->nNum >= 2)
		{
			sys->nPlayer((int)m->nNum > 4 ? 4 : (int)m->nNum);
			sys->nStage(1);
		}
		else
		{
			sys->nPlayer(1);
			sys->nStage(0);
		}
	}
	cAppGameBaseMenuMatching::Free();
}

}
