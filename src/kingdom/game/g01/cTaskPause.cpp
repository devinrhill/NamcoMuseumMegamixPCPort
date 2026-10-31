#include "KingdomGlobal.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskPause.hpp"
#include "kingdom/KingdomJobTable.hpp"

namespace kingdom::game::g01
{

// DOL 0x801b92cc [audited] + Init DOL 0x800f6060 [audited]: the sys matching block (+4): one cursor per player on that player's pad. Alone the block
// holds 1 player, the same as nullptr natively
cTaskPause::cTaskPause()
{
	sMatchingData* data = nullptr;
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	if (sys != nullptr && sys->IsMulti())
	{
		const sGalagaMatching* g = sys->pMatchingData();
		m_Matching.nNum = (uint)g->nPlayer;
		for (uint i = 0; i < 4; i++)
		{
			m_Info[i].Name = g->player[i].szName;
			m_Info[i].nCtrl = (uint)g->player[i].nWork;
			m_Matching.Info[i] = &m_Info[i];
		}
		data = &m_Matching;
	}
	RegistData("ANT_PAUSE", "ANT_MENU_POINTER", data);   // 0x80147a64(this, 0xdf5903f3, 0xf63671cb, matching)
	nMenuJob() = job::GALAGA_MENU;
}

cTaskPause::~cTaskPause()
{
	Free();
}

}
