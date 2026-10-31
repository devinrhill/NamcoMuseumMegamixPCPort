#include "kingdom/game/g03/cTaskCharGhostHover.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017ef8c [audited: SetModel / SetParam(kind) / SetEffect order vs disassembly (SetDrawShadow is a Unity stub)]
void cTaskCharGhostHover::Init()
{
	SetModel();
	SetParam(12);
	SetEffect();
	SetDrawShadow(false);
}

// DOL 0x80180524 [audited: blr / base Term]
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

// DOL 0x801805c4 [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostHover::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_HOVER;
}

// DOL 0x801805cc [audited: constant getter vs disassembly]
string cTaskCharGhostHover::GetModelKey()
{
	return "BIN_GOS_HOV_MDL";
}

// DOL 0x801805d8 [audited: constant getter vs disassembly]
string cTaskCharGhostHover::GetAnmKey()
{
	return "BIN_GOS_HOV_CHA";
}

// DOL 0x801805e4 [audited: constant getter vs disassembly]
string cTaskCharGhostHover::GetNodeKey0()
{
	return "ef_hov01";
}

// DOL 0x801805f0 [audited: constant getter vs disassembly]
string cTaskCharGhostHover::GetNodeKey1()
{
	return "ef_hov02";
}

// DOL 0x801805fc [audited: constant getter vs disassembly]
uint cTaskCharGhostHover::GetEffectType0()
{
	return 26u;
}

// DOL 0x80180604 [audited: constant getter vs disassembly]
uint cTaskCharGhostHover::GetEffectType1()
{
	return 24u;
}

}
