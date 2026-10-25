#include "kingdom/game/g02/cTaskPause.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"

namespace kingdom::game::g02
{

// DOL 0x80123e84 (inlined cTaskGameBasePause ctor + Init) + 0x8007a244. The multiplayer registration passes the sys matching block
// (sys +0x64) as sMatchingData: NOT PORTED (the port's sys keeps those bytes raw), nullptr is passed instead.
cTaskPause::cTaskPause()
{
	SetName("g02::cTaskPause");
	const bool multi = KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsMulti();   // 0x800c5ca0
	RegistData("COMMON_ANITA_PAUSE", "COMMON_ANITA_POINTER", nullptr);   // 0x80147a64
	nMenuJob() = multi ? 0x0c800000u : 0x0c200000u;                      // +0x1c8
}

// DOL 0x8012734c
cTaskPause::~cTaskPause()
{
	Free();
}

}
