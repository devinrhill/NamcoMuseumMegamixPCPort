#include "kingdom/game/g07/cAppG07MotosGameOver.hpp"

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

void cAppG07MotosGameOver::Init()
{
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_GAME_OVER"));
	pAnita(0u)->SetAnimNo(0);
	nAnita() = 0u;
	nNextJob() = 287309824u;
}

void cAppG07MotosGameOver::Term()
{
}

}
