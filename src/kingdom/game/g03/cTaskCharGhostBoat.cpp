#include "kingdom/game/g03/cTaskCharGhostBoat.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017eee0 [audited: SetModel / SetParam(kind) / SetEffect order vs disassembly (SetDrawShadow is a Unity stub)]
void cTaskCharGhostBoat::Init()
{
	SetModel();
	SetParam(10);
	SetEffect();
	SetDrawShadow(false);
}

// DOL 0x801806f0 [audited: blr / base Term]
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

// DOL 0x80180790 [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostBoat::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_BOAT;
}

// DOL 0x80180798 [audited: constant getter vs disassembly]
string cTaskCharGhostBoat::GetModelKey()
{
	return "BIN_GOS_BOA_MDL";
}

// DOL 0x801807a4 [audited: constant getter vs disassembly]
string cTaskCharGhostBoat::GetAnmKey()
{
	return "BIN_GOS_BOA_CHA";
}

// DOL 0x801807b0 [audited: constant getter vs disassembly]
string cTaskCharGhostBoat::GetNodeKey0()
{
	return "ef_boa01";
}

// DOL 0x801807bc [audited: constant getter vs disassembly]
string cTaskCharGhostBoat::GetNodeKey1()
{
	return "ef_boa02";
}

// DOL 0x801807c8 [audited: constant getter vs disassembly]
uint cTaskCharGhostBoat::GetEffectType0()
{
	return 27u;
}

// DOL 0x801807d0 [audited: constant getter vs disassembly]
uint cTaskCharGhostBoat::GetEffectType1()
{
	return 24u;
}

}
