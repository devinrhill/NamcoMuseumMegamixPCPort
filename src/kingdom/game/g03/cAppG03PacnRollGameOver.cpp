#include "kingdom/game/g03/cAppG03PacnRollGameOver.hpp"

#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g03
{

// DOL 0x8012b698 [audited: per AUDIT_G03_G07 row] (job init: new 0x2b0, base ctor 0x80142e4c, Init 0x800fc214, RegistMainTask)
cAppG03PacnRollGameOver::cAppG03PacnRollGameOver()
{
	Init();
}

void cAppG03PacnRollGameOver::Free()
{
	cAppGameBaseGameOver::Free();
	Term();
}

// DOL 0x800fc214 [audited: per AUDIT_G03_G07 row] (RegistData BIN_ANT_GAMEOVER, SetAnimNo(0, true, 0.0), +0x294 = 0, +0x298 next job = 0x0d200000)
void cAppG03PacnRollGameOver::Init()
{
	// archive entry name replaces the Unity asset path (g03_pacnroll/anita/console/ja/m03c_gameover01)
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("BIN_ANT_GAMEOVER"));
	pAnita(0u)->SetAnimNo(0);
	nAnita() = 0u;
	nNextJob() = 220200960u;
}

// DOL 0x8012bdb4 [audited: per AUDIT_G03_G07 row] (the destructor chain only; no own Term code)
void cAppG03PacnRollGameOver::Term()
{
}

}
