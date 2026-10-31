#include "kingdom/game/g07/cAppG07MotosGameOver.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cAppG07MotosGameOver::cAppG07MotosGameOver()
{
	Init();
}

void cAppG07MotosGameOver::Free()
{
	cAppGameBaseGameOver::Free();
	Term();
}

// DOL 0x8012bf00 [audited: per AUDIT_G03_G07 row] (shared GameOver init, ctor 0x800a2f9c; audited 44th: matches, next job MOTOS_MENU)
void cAppG07MotosGameOver::Init()
{
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_GAME_OVER"));
	pAnita(0u)->SetAnimNo(0);
	nAnita() = 0u;
	nNextJob() = kingdom::job::MOTOS_MENU;
}

void cAppG07MotosGameOver::Term()
{
}

}
