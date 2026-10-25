#include "kingdom/game/g03/cTaskCharNonRideGhost.hpp"

namespace kingdom::game::g03
{

void cTaskCharNonRideGhost::Init()
{
	SetModel();
	SetParam(13);
	SetEffect();
}

void cTaskCharNonRideGhost::Term()
{
	cTaskCharRouteGhost::Term();
}

cTaskCharNonRideGhost::cTaskCharNonRideGhost()
{
	nGhostRider() = 1u;
	Init();
}

cTaskCharNonRideGhost::cTaskCharNonRideGhost(uint type)
{
	nGhostRider() = type;
	Init();
}

cTaskChar::eKIND cTaskCharNonRideGhost::GetKind()
{
	return ((nGhostRider() == 1u) ? cTaskChar::eKIND::eKIND_CLYDE : ((nGhostRider() == 2u) ? cTaskChar::eKIND::eKIND_PINKY : ((nGhostRider() == 3u) ? cTaskChar::eKIND::eKIND_BLINKY : ((nGhostRider() == 4u) ? cTaskChar::eKIND::eKIND_INKY : (cTaskChar::eKIND::eKIND_CLYDE)))));
}

string cTaskCharNonRideGhost::GetModelKey()
{
	return "BIN_GOS_CAR_MDL";
}

string cTaskCharNonRideGhost::GetAnmKey()
{
	return "BIN_GOS_CAR_CHA";
}

string cTaskCharNonRideGhost::GetNodeKey0()
{
	return string();
}

string cTaskCharNonRideGhost::GetNodeKey1()
{
	return string();
}

uint cTaskCharNonRideGhost::GetEffectType0()
{
	return 38u;
}

uint cTaskCharNonRideGhost::GetEffectType1()
{
	return 38u;
}

}
