#include "kingdom/game/g03/cAppG03PacnRollGameOver.hpp"

#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g03
{

cAppG03PacnRollGameOver::cAppG03PacnRollGameOver()
{
	Init();
}

void cAppG03PacnRollGameOver::Free()
{
	cAppGameBaseGameOver::Free();
	Term();
}

void cAppG03PacnRollGameOver::Init()
{
	// archive entry name replaces the Unity asset path (g03_pacnroll/anita/console/ja/m03c_gameover01)
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("BIN_ANT_GAMEOVER"));
	pAnita(0u)->SetAnimNo(0);
	nAnita() = 0u;
	nNextJob() = 220200960u;
}

void cAppG03PacnRollGameOver::Term()
{
}

}
