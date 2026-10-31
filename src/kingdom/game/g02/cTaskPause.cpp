#include "kingdom/game/g02/cTaskPause.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"

namespace kingdom::game::g02
{

// DOL 0x80123e84 [audited] (inlined cTaskGameBasePause ctor + Init) + 0x8007a244. The multiplayer registration passes the sys matching block (sys +0x64)
cTaskPause::cTaskPause()
{
	SetName("g02::cTaskPause");
	const bool multi = KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsMulti();   // 0x800c5ca0
	sMatchingData* data = nullptr;
	if (multi)
	{
		m_Matching.nNum = (uint)cG02RallyXSysExt::nMatchNum;
		for (uint i = 0; i < 4; i++)
		{
			m_Info[i].Name = cG02RallyXSysExt::szMatchName[i];
			m_Info[i].nCtrl = cG02RallyXSysExt::nMatchCtrl[i];
			m_Matching.Info[i] = &m_Info[i];
		}
		data = &m_Matching;
	}
	RegistData("COMMON_ANITA_PAUSE", "COMMON_ANITA_POINTER", data);   // 0x80147a64
	nMenuJob() = multi ? job::RALLYX_MULTI_MENU : job::RALLYX_MENU;                      // +0x1c8
}

// DOL 0x8012734c [audited]
cTaskPause::~cTaskPause()
{
	Free();
}

}
