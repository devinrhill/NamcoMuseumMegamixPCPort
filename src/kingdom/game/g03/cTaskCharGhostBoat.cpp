#include "kingdom/game/g03/cTaskCharGhostBoat.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostBoat::Init()
{
	SetModel();
	SetParam(10);
	SetEffect();
	SetDrawShadow(false);
}

void cTaskCharGhostBoat::Term()
{
	cTaskCharRouteGhost::Term();
}

cTaskCharGhostBoat::cTaskCharGhostBoat()
{
	nGhostRider() = 1u;
	Init();
}

cTaskCharGhostBoat::cTaskCharGhostBoat(uint type)
{
	nGhostRider() = type;
	Init();
}

cTaskChar::eKIND cTaskCharGhostBoat::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_BOAT;
}

string cTaskCharGhostBoat::GetModelKey()
{
	return "BIN_GOS_BOA_MDL";
}

string cTaskCharGhostBoat::GetAnmKey()
{
	return "BIN_GOS_BOA_CHA";
}

string cTaskCharGhostBoat::GetNodeKey0()
{
	return "ef_boa01";
}

string cTaskCharGhostBoat::GetNodeKey1()
{
	return "ef_boa02";
}

uint cTaskCharGhostBoat::GetEffectType0()
{
	return 27u;
}

uint cTaskCharGhostBoat::GetEffectType1()
{
	return 24u;
}

}
