#include "kingdom/game/g03/cTaskCharGhostUfo.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostUfo::Init()
{
	SetModel();
	SetParam(9);
	SetEffect();
	nSndType() = 2u;
	SetDrawShadow(false);
}

void cTaskCharGhostUfo::Term()
{
	cTaskCharRouteGhost::Term();
}

cTaskCharGhostUfo::cTaskCharGhostUfo()
{
	nGhostRider() = 1u;
	Init();
}

cTaskCharGhostUfo::cTaskCharGhostUfo(uint type)
{
	nGhostRider() = type;
	Init();
}

cTaskChar::eKIND cTaskCharGhostUfo::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_UFO;
}

string cTaskCharGhostUfo::GetModelKey()
{
	return "BIN_GOS_UFO_MDL";
}

string cTaskCharGhostUfo::GetAnmKey()
{
	return "BIN_GOS_UFO_CHA";
}

string cTaskCharGhostUfo::GetNodeKey0()
{
	return "ef_ufo01";
}

string cTaskCharGhostUfo::GetNodeKey1()
{
	return "ef_ufo01";
}

uint cTaskCharGhostUfo::GetEffectType0()
{
	return 25u;
}

uint cTaskCharGhostUfo::GetEffectType1()
{
	return 38u;
}

}
