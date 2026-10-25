#include "kingdom/game/g03/cTaskCharGhostPlane.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostPlane::Init()
{
	SetModel();
	SetParam(11);
	SetEffect();
	nSndType() = 1u;
	SetDrawShadow(false);
}

void cTaskCharGhostPlane::Term()
{
	cTaskCharRouteGhost::Term();
}

cTaskCharGhostPlane::cTaskCharGhostPlane()
{
	nGhostRider() = 1u;
	Init();
}

cTaskCharGhostPlane::cTaskCharGhostPlane(uint type)
{
	nGhostRider() = type;
	Init();
}

cTaskChar::eKIND cTaskCharGhostPlane::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_PLANE;
}

string cTaskCharGhostPlane::GetModelKey()
{
	return "BIN_GOS_AIR_MDL";
}

string cTaskCharGhostPlane::GetAnmKey()
{
	return "BIN_GOS_AIR_CHA";
}

string cTaskCharGhostPlane::GetNodeKey0()
{
	return "ef_air01";
}

string cTaskCharGhostPlane::GetNodeKey1()
{
	return string();
}

uint cTaskCharGhostPlane::GetEffectType0()
{
	return 28u;
}

uint cTaskCharGhostPlane::GetEffectType1()
{
	return 38u;
}

}
