#pragma once

#include "common.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/cKingdomSys.hpp"

// Game-specific types are only named by pointer here; the boot path must not pull in any minigame.
namespace kingdom::game::g01 { class cAppG01GalagaGame; class cG01GalagaSys; }
namespace kingdom::game::g02 { class cG02RallyXSys; }
namespace kingdom::game::g11 { class cG11GrobdaSys; }
namespace kingdom::game::g04 { class cG04WaniWaniSys; }
namespace kingdom::game::g03 { class cAppG03PacnRollGame; class cG03PacnRollSys; }
namespace kingdom::game::g07 { class cAppG07MotosGame; class cG07MotosSys; class cMotosGameSys; }

class KingdomGlobal
{
public:
	static cSysCoreStructure* SysCore;

	static kingdom::cKingdomSys* KingSys;

	static kingdom::game::g01::cG01GalagaSys* GalagaSys;        // DOL r13 - 0x5760

	static kingdom::game::g01::cAppG01GalagaGame* g_pGalagaGame;

	static kingdom::game::g11::cG11GrobdaSys* GrobdaSys;         // DOL r13 - 0x57d8
	static kingdom::game::g02::cG02RallyXSys* RallyXSys;         // DOL r13 - 0x5700 (DAT_80479020)
	static kingdom::game::g04::cG04WaniWaniSys* WaniWaniSys;     // DOL r13 - 0x5618 (DAT_80479108)

	static kingdom::game::g03::cG03PacnRollSys* PacnRollSys;

	static kingdom::game::g03::cAppG03PacnRollGame* g_pRollGame;

	/*
	static kingdom::game::g04::cG04WaniWaniSys* WaniWaniSys;

	static kingdom::game::g04::cAppG04WaniWaniGame* g_pWaniWaniGame;
	*/

	static kingdom::game::g07::cG07MotosSys* MotosSys;

	static kingdom::game::g07::cMotosGameSys* pMotosGameSys;

	static kingdom::game::g07::cAppG07MotosGame* g_pMotosGame;

	/*
	static kingdom::game::g11::cG11GrobdaSys* GrobdaSys;

	static kingdom::game::g11::cAppG11GrobdaGame* g_pGrobdaGame;
	*/

	static float ShaderInGameTime;
};
