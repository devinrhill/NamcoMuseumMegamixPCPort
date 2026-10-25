#include "kingdom/game/g03/cTaskCharGhostHover.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostHover::Init()
{
	SetModel();
	SetParam(12);
	SetEffect();
	SetDrawShadow(false);
}

void cTaskCharGhostHover::Term()
{
	cTaskCharRouteGhost::Term();
}

cTaskCharGhostHover::cTaskCharGhostHover()
{
	nGhostRider() = 1u;
	Init();
}

cTaskCharGhostHover::cTaskCharGhostHover(uint type)
{
	nGhostRider() = type;
	Init();
}

cTaskChar::eKIND cTaskCharGhostHover::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_HOVER;
}

string cTaskCharGhostHover::GetModelKey()
{
	return "BIN_GOS_HOV_MDL";
}

string cTaskCharGhostHover::GetAnmKey()
{
	return "BIN_GOS_HOV_CHA";
}

string cTaskCharGhostHover::GetNodeKey0()
{
	return "ef_hov01";
}

string cTaskCharGhostHover::GetNodeKey1()
{
	return "ef_hov02";
}

uint cTaskCharGhostHover::GetEffectType0()
{
	return 26u;
}

uint cTaskCharGhostHover::GetEffectType1()
{
	return 24u;
}

}
