#include "kingdom/game/g03/cTaskCharGhostPinky.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017de60 [audited: described by the 43rd-session pass] (SetParam 0x8015d2f8 + vtable +0x58 SetModel)
void cTaskCharGhostPinky::Init()
{
	SetParam();
	SetModel();
}

// DOL 0x80180c78 [audited: blr] blr
void cTaskCharGhostPinky::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostPinky::cTaskCharGhostPinky()
{
	Init();
}

// DOL 0x80180d18 [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostPinky::GetKind()
{
	return cTaskChar::eKIND::eKIND_PINKY;
}

// DOL 0x80180d20 [audited: constant getter vs disassembly]
string cTaskCharGhostPinky::GetModelKey()
{
	return "BIN_GOS_PINK_MDL";
}

// DOL 0x80180d2c [audited: constant getter vs disassembly]
string cTaskCharGhostPinky::GetAnmKey()
{
	return "BIN_GOS_PINK_CHA";
}

// DOL 0x80180d38 [audited: constant getter vs disassembly]
string cTaskCharGhostPinky::GetSRTAnmKey()
{
	return "BIN_GOS_PINK_TSA";
}

// DOL 0x80180d44 [audited: constant getter vs disassembly]
uint cTaskCharGhostPinky::GetPushOutLevel()
{
	return 0u;
}

}
