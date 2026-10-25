#include "kingdom/game/g03/cTaskCharGhostCar.hpp"

namespace kingdom::game::g03
{

void cTaskCharGhostCar::Init()
{
	SetModel();
	SetParam(8);
	SetEffect();
	nSndType() = 1u;
	SetDrawShadow(false);
}

void cTaskCharGhostCar::Term()
{
	cTaskCharRouteGhost::Term();
}

cTaskCharGhostCar::cTaskCharGhostCar()
{
	nGhostRider() = 1u;
	Init();
}

cTaskCharGhostCar::cTaskCharGhostCar(uint type)
{
	nGhostRider() = type;
	Init();
}

cTaskChar::eKIND cTaskCharGhostCar::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_CAR;
}

string cTaskCharGhostCar::GetModelKey()
{
	return "BIN_GOS_CAR_MDL";
}

string cTaskCharGhostCar::GetAnmKey()
{
	return "BIN_GOS_CAR_CHA";
}

string cTaskCharGhostCar::GetNodeKey0()
{
	return "ef_car01";
}

string cTaskCharGhostCar::GetNodeKey1()
{
	return string();
}

uint cTaskCharGhostCar::GetEffectType0()
{
	return 24u;
}

uint cTaskCharGhostCar::GetEffectType1()
{
	return 38u;
}

}
