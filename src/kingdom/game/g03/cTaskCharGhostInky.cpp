#include "kingdom/game/g03/cTaskCharGhostInky.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostInky::Init()
{
	SetParam();
	SetModel();
}

void cTaskCharGhostInky::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostInky::cTaskCharGhostInky()
{
	Init();
}

cTaskChar::eKIND cTaskCharGhostInky::GetKind()
{
	return cTaskChar::eKIND::eKIND_INKY;
}

string cTaskCharGhostInky::GetModelKey()
{
	return "BIN_GOS_INKY_MDL";
}

string cTaskCharGhostInky::GetAnmKey()
{
	return "BIN_GOS_INKY_CHA";
}

string cTaskCharGhostInky::GetSRTAnmKey()
{
	return "BIN_GOS_INKY_TSA";
}

uint cTaskCharGhostInky::GetPushOutLevel()
{
	return 0u;
}

}
