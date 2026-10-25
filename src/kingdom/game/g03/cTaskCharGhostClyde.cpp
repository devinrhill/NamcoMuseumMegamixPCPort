#include "kingdom/game/g03/cTaskCharGhostClyde.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostClyde::Init()
{
	SetParam();
	SetModel();
}

void cTaskCharGhostClyde::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostClyde::cTaskCharGhostClyde()
{
	Init();
}

cTaskChar::eKIND cTaskCharGhostClyde::GetKind()
{
	return cTaskChar::eKIND::eKIND_CLYDE;
}

string cTaskCharGhostClyde::GetModelKey()
{
	return "BIN_GOS_CLYD_MDL";
}

string cTaskCharGhostClyde::GetAnmKey()
{
	return "BIN_GOS_CLYD_CHA";
}

string cTaskCharGhostClyde::GetSRTAnmKey()
{
	return "BIN_GOS_CLYD_TSA";
}

uint cTaskCharGhostClyde::GetPushOutLevel()
{
	return 0u;
}

}
