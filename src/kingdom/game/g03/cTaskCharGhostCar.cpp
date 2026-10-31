#include "kingdom/game/g03/cTaskCharGhostCar.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017ee28 [audited: SetModel / SetParam(kind) / SetEffect order vs disassembly (SetDrawShadow is a Unity stub)]
void cTaskCharGhostCar::Init()
{
	SetModel();
	SetParam(8);
	SetEffect();
	nSndType() = 1u;
	SetDrawShadow(false);
}

// DOL 0x801808c0 [audited: blr / base Term]
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

// DOL 0x80180960 [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostCar::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_CAR;
}

// DOL 0x80180968 [audited: constant getter vs disassembly]
string cTaskCharGhostCar::GetModelKey()
{
	return "BIN_GOS_CAR_MDL";
}

// DOL 0x80180974 [audited: constant getter vs disassembly]
string cTaskCharGhostCar::GetAnmKey()
{
	return "BIN_GOS_CAR_CHA";
}

// DOL 0x80180980 [audited: constant getter vs disassembly]
string cTaskCharGhostCar::GetNodeKey0()
{
	return "ef_car01";
}

// DOL 0x8018098c [audited: li 0 (empty key)]
string cTaskCharGhostCar::GetNodeKey1()
{
	return string();
}

// DOL 0x80180994 [audited: constant getter vs disassembly]
uint cTaskCharGhostCar::GetEffectType0()
{
	return 24u;
}

// DOL 0x8018099c [audited: constant getter vs disassembly]
uint cTaskCharGhostCar::GetEffectType1()
{
	return 38u;
}

}
