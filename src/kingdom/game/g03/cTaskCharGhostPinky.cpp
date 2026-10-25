#include "kingdom/game/g03/cTaskCharGhostPinky.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostPinky::Init()
{
	SetParam();
	SetModel();
}

void cTaskCharGhostPinky::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostPinky::cTaskCharGhostPinky()
{
	Init();
}

cTaskChar::eKIND cTaskCharGhostPinky::GetKind()
{
	return cTaskChar::eKIND::eKIND_PINKY;
}

string cTaskCharGhostPinky::GetModelKey()
{
	return "BIN_GOS_PINK_MDL";
}

string cTaskCharGhostPinky::GetAnmKey()
{
	return "BIN_GOS_PINK_CHA";
}

string cTaskCharGhostPinky::GetSRTAnmKey()
{
	return "BIN_GOS_PINK_TSA";
}

uint cTaskCharGhostPinky::GetPushOutLevel()
{
	return 0u;
}

}
