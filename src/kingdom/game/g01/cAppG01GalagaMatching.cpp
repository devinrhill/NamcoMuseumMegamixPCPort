#include "kingdom/game/g01/cAppG01GalagaMatching.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include <cstring>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/sMatchingData.hpp"

namespace kingdom::game::g01
{

// DOL 0x800b13d0 [audited] (job 182)
void cAppG01GalagaMatching::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG01GalagaMatching());
}

// DOL 0x801059f4 [audited] (the menu archive is already attached by the stage select)
cAppG01GalagaMatching::cAppG01GalagaMatching()
{
	cG01GalagaSys::AttachMenuArc();   // native: the archive of ANT_MATCHING when the job is entered directly (UNVERIFIED: not in 0x801059f4)
	RegistData("ANT_MENU_POINTER", "ANT_MATCHING", "MATCHING_CAMERA_SCA_01", "MATCHING_SELECT_SET_MDL");
	nMenuJob() = job::GALAGA_MENU;    // +0x280
	nMultiJob() = job::GALAGA_MENU;   // +0x284
}

// DOL 0x80105a4c [audited]: +0x29a set -> the matching data (0x154 bytes) into cG01GalagaSys +4, else back to 1 player. The port's sMatchingData is
// the converted structure (name / controller / work per player): mapped field by field (UNVERIFIED mapping; +0x29a taken as bConfirmDecide).
void cAppG01GalagaMatching::Free()
{
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	if (sys != nullptr)
	{
		const sMatchingData* m = pMatchingData();
		if (!bConfirmDecide() || m == nullptr)
		{
			sys->nPlayer(1);
		}
		else
		{
			sGalagaMatching* g = sys->pMatchingData();
			g->nPlayer = (s32)m->nNum;
			for (uint i = 0; i < 4 && i < m->nNum; i++)
			{
				if (m->Info[i] == nullptr) continue;
				std::strncpy(g->player[i].szName, m->Info[i]->Name.c_str(), sizeof g->player[i].szName - 1);
				g->player[i].nIndex = (s32)i;
				g->player[i].nWork = (s32)m->Info[i]->nCtrl;
			}
		}
	}
	cG01GalagaSys::DetachMenuArc();
	cAppGameBaseMenuMatching::Free();
}

}
