#include "kingdom/game/g04/cAppG04WaniWaniMatching.hpp"

#include <cstring>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"
#include "kingdom/sMatchingData.hpp"

namespace kingdom::game::g04
{

// DOL 0x8019a470 (job 229)
void cAppG04WaniWaniMatching::JobInit()
{
	if (KingdomGlobal::WaniWaniSys == nullptr) KingdomGlobal::WaniWaniSys = new cG04WaniWaniSys();   // native: a direct --job entry has no sys yet
	KingdomGlobal::SysCore->RegistMainTask(new cAppG04WaniWaniMatching());
}

// DOL 0x80095044
cAppG04WaniWaniMatching::cAppG04WaniWaniMatching()
{
	RegistData("COMMON_ANITA_POINTER", "COMMON_ANITA_MATCHING", "COMMON_CAM_MATCHING_SCA", "COMMON_SET_MATCHING_MDL");
	nMenuJob() = 0x0e700000u;    // +0x280
	nMultiJob() = 0x0e700000u;   // +0x284
}

// DOL 0x8009509c: confirmed (+0x29a) -> sys +0x15c = the player count and the 0x154 byte matching data into sys +4; else sys +0x15c = 1.
// The port's sMatchingData is the converted structure: mapped field by field as Galaga's (UNVERIFIED mapping)
void cAppG04WaniWaniMatching::Free()
{
	cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	if (sys != nullptr)
	{
		const sMatchingData* m = pMatchingData();
		if (bConfirmDecide() && m != nullptr)
		{
			sys->SetPlayers((int)m->nNum);
			sWaniMatching* g = sys->pMatchingData();
			g->nPlayer = (s32)m->nNum;
			for (uint i = 0; i < 4 && i < m->nNum; i++)
			{
				if (m->Info[i] == nullptr) continue;
				std::strncpy(g->player[i].szName, m->Info[i]->Name.c_str(), sizeof g->player[i].szName - 1);
				g->player[i].nIndex = (s32)i;
				g->player[i].nWork = (s32)m->Info[i]->nCtrl;
			}
		}
		else
			sys->SetPlayers(1);
	}
	cAppGameBaseMenuMatching::Free();
}

}
