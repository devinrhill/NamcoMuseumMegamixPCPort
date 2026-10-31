#include "kingdom/game/g03/cTaskCharGhostBlinky.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017dea4 [audited: described by the 43rd-session pass] (SetParam 0x8015d2f8 + vtable +0x58 SetModel)
void cTaskCharGhostBlinky::Init()
{
	SetParam();
	SetModel();
}

// DOL 0x80180ba4 [audited: blr] blr
void cTaskCharGhostBlinky::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostBlinky::cTaskCharGhostBlinky()
{
	Init();
}

// DOL 0x80180c44 [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostBlinky::GetKind()
{
	return cTaskChar::eKIND::eKIND_BLINKY;
}

// DOL 0x80180c4c [audited: constant getter vs disassembly]
string cTaskCharGhostBlinky::GetModelKey()
{
	return "BIN_GOS_BLIN_MDL";
}

// DOL 0x80180c58 [audited: constant getter vs disassembly]
string cTaskCharGhostBlinky::GetAnmKey()
{
	return "BIN_GOS_BLIN_CHA";
}

// DOL 0x80180c64 [audited: constant getter vs disassembly]
string cTaskCharGhostBlinky::GetSRTAnmKey()
{
	return "BIN_GOS_BLIN_TSA";
}

// DOL 0x80180c70 [audited: constant getter vs disassembly]
uint cTaskCharGhostBlinky::GetPushOutLevel()
{
	return 0u;
}

}
