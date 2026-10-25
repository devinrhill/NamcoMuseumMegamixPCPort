#include "kingdom/game/g03/cTaskCharGhostBlinky.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostBlinky::Init()
{
	SetParam();
	SetModel();
}

void cTaskCharGhostBlinky::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostBlinky::cTaskCharGhostBlinky()
{
	Init();
}

cTaskChar::eKIND cTaskCharGhostBlinky::GetKind()
{
	return cTaskChar::eKIND::eKIND_BLINKY;
}

string cTaskCharGhostBlinky::GetModelKey()
{
	return "BIN_GOS_BLIN_MDL";
}

string cTaskCharGhostBlinky::GetAnmKey()
{
	return "BIN_GOS_BLIN_CHA";
}

string cTaskCharGhostBlinky::GetSRTAnmKey()
{
	return "BIN_GOS_BLIN_TSA";
}

uint cTaskCharGhostBlinky::GetPushOutLevel()
{
	return 0u;
}

}
