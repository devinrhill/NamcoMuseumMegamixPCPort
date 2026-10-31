#include "kingdom/game/g03/cTaskCharGhostPlane.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017ef30 [audited: SetModel / SetParam(kind) / SetEffect order vs disassembly (SetDrawShadow is a Unity stub)]
void cTaskCharGhostPlane::Init()
{
	SetModel();
	SetParam(11);
	SetEffect();
	nSndType() = 1u;
	SetDrawShadow(false);
}

// DOL 0x8018060c [audited: blr / base Term]
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

// DOL 0x801806ac [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostPlane::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_PLANE;
}

// DOL 0x801806b4 [audited: constant getter vs disassembly]
string cTaskCharGhostPlane::GetModelKey()
{
	return "BIN_GOS_AIR_MDL";
}

// DOL 0x801806c0 [audited: constant getter vs disassembly]
string cTaskCharGhostPlane::GetAnmKey()
{
	return "BIN_GOS_AIR_CHA";
}

// DOL 0x801806cc [audited: constant getter vs disassembly]
string cTaskCharGhostPlane::GetNodeKey0()
{
	return "ef_air01";
}

// DOL 0x801806d8 [audited: li 0 (empty key)]
string cTaskCharGhostPlane::GetNodeKey1()
{
	return string();
}

// DOL 0x801806e0 [audited: constant getter vs disassembly]
uint cTaskCharGhostPlane::GetEffectType0()
{
	return 28u;
}

// DOL 0x801806e8 [audited: constant getter vs disassembly]
uint cTaskCharGhostPlane::GetEffectType1()
{
	return 38u;
}

}
