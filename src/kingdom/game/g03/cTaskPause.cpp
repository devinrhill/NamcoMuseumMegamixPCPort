#include "kingdom/game/g03/cTaskPause.hpp"

#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g03
{

cTaskPause::cTaskPause()
{
	SetName("cTaskPause");
	Init();
}

cTaskPause::~cTaskPause()
{
	Term();
}

void cTaskPause::Init()
{
	RegistData(ResourceDatabase::GetPath("BIN_COMMON_ANT_PAUSE"), ResourceDatabase::GetPath("BIN_COMMON_ANT_CURSOR"));
	nMenuJob() = 220200960u;
}

void cTaskPause::Term()
{
}

}
