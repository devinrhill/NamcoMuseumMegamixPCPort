#include "kingdom/game/g03/cTaskCharNonRideGhost.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017edd8 [audited: SetParam(13) order]
void cTaskCharNonRideGhost::Init()
{
	SetModel();
	SetParam(13);
	SetEffect();
}

// DOL 0x801809a4 [audited]
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

// DOL 0x80180a44 [audited: rider 1..4 switch]
cTaskChar::eKIND cTaskCharNonRideGhost::GetKind()
{
	return ((nGhostRider() == 1u) ? cTaskChar::eKIND::eKIND_CLYDE : ((nGhostRider() == 2u) ? cTaskChar::eKIND::eKIND_PINKY : ((nGhostRider() == 3u) ? cTaskChar::eKIND::eKIND_BLINKY : ((nGhostRider() == 4u) ? cTaskChar::eKIND::eKIND_INKY : (cTaskChar::eKIND::eKIND_CLYDE)))));
}

// DOL 0x80180a98 [audited: constant getter vs disassembly]
string cTaskCharNonRideGhost::GetModelKey()
{
	return "BIN_GOS_CAR_MDL";
}

// DOL 0x80180aa4 [audited: constant getter vs disassembly]
string cTaskCharNonRideGhost::GetAnmKey()
{
	return "BIN_GOS_CAR_CHA";
}

// DOL 0x80180ab0 [audited]
string cTaskCharNonRideGhost::GetNodeKey0()
{
	return string();
}

// DOL 0x80180ab8 [audited]
string cTaskCharNonRideGhost::GetNodeKey1()
{
	return string();
}

// DOL 0x80180ac0 [audited: constant getter vs disassembly]
uint cTaskCharNonRideGhost::GetEffectType0()
{
	return 38u;
}

// DOL 0x80180ac8 [audited: constant getter vs disassembly]
uint cTaskCharNonRideGhost::GetEffectType1()
{
	return 38u;
}

}
