#include "kingdom/game/g11/cG11GrobdaSys.hpp"
#include "kingdom/game/g11/cTaskGrobdaPause.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"

namespace kingdom::game::g11
{

// DOL 0x80037378 [audited] (FUN_80147a64 = RegistData with the matching block sys +4)
cTaskGrobdaPause::cTaskGrobdaPause()
{
	SetName("g11::cTaskGrobdaPause");
	sMatchingData* data = nullptr;
	cG11GrobdaSys* sys = KingdomGlobal::GrobdaSys;
	if (sys != nullptr && sys->IsMulti())
	{
		m_Matching.nNum = (uint)sys->nPlayer();
		for (uint i = 0; i < 4; i++)
		{
			m_Info[i].nCtrl = (uint)sys->nPad((int)i);
			m_Matching.Info[i] = &m_Info[i];
		}
		data = &m_Matching;
	}
	RegistData("BIN_COMMON_ANT_PAUSE", "BIN_COMMON_ANT_CURSOR", data);   // crc 0x6e3ccd30 / 0x06ca4270 (g11resident.lzs)
	nMenuJob() = job::GROBDA_MENU;   // +0x1c8
}

}
