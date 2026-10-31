#include "kingdom/game/g03/cTaskCharGhostUfo.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017ee84 [audited: SetModel / SetParam(kind) / SetEffect order vs disassembly (SetDrawShadow is a Unity stub)]
void cTaskCharGhostUfo::Init()
{
	SetModel();
	SetParam(9);
	SetEffect();
	nSndType() = 2u;
	SetDrawShadow(false);
}

// DOL 0x801807d8 [audited: blr / base Term]
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

// DOL 0x80180878 [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostUfo::GetKind()
{
	return cTaskChar::eKIND::eKIND_RIDE_UFO;
}

// DOL 0x80180880 [audited: constant getter vs disassembly]
string cTaskCharGhostUfo::GetModelKey()
{
	return "BIN_GOS_UFO_MDL";
}

// DOL 0x8018088c [audited: constant getter vs disassembly]
string cTaskCharGhostUfo::GetAnmKey()
{
	return "BIN_GOS_UFO_CHA";
}

// DOL 0x80180898 [audited: constant getter vs disassembly]
string cTaskCharGhostUfo::GetNodeKey0()
{
	return "ef_ufo01";
}

// DOL 0x801808a4 [audited: constant getter vs disassembly]
string cTaskCharGhostUfo::GetNodeKey1()
{
	return "ef_ufo01";
}

// DOL 0x801808b0 [audited: constant getter vs disassembly]
uint cTaskCharGhostUfo::GetEffectType0()
{
	return 25u;
}

// DOL 0x801808b8 [audited: constant getter vs disassembly]
uint cTaskCharGhostUfo::GetEffectType1()
{
	return 38u;
}

}
