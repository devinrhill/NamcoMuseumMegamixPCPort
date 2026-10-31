#include "kingdom/game/g03/cTaskCharGhostClyde.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017de1c [audited: described by the 43rd-session pass] (SetParam 0x8015d2f8 + vtable +0x58 SetModel)
void cTaskCharGhostClyde::Init()
{
	SetParam();
	SetModel();
}

// DOL 0x80180d4c [audited: blr] blr
void cTaskCharGhostClyde::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostClyde::cTaskCharGhostClyde()
{
	Init();
}

// DOL 0x80180dec [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostClyde::GetKind()
{
	return cTaskChar::eKIND::eKIND_CLYDE;
}

// DOL 0x80180df4 [audited: constant getter vs disassembly]
string cTaskCharGhostClyde::GetModelKey()
{
	return "BIN_GOS_CLYD_MDL";
}

// DOL 0x80180e00 [audited: constant getter vs disassembly]
string cTaskCharGhostClyde::GetAnmKey()
{
	return "BIN_GOS_CLYD_CHA";
}

// DOL 0x80180e0c [audited: constant getter vs disassembly]
string cTaskCharGhostClyde::GetSRTAnmKey()
{
	return "BIN_GOS_CLYD_TSA";
}

// DOL 0x80180e18 [audited: constant getter vs disassembly]
uint cTaskCharGhostClyde::GetPushOutLevel()
{
	return 0u;
}

}
