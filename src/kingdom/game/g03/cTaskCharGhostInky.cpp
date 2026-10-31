#include "kingdom/game/g03/cTaskCharGhostInky.hpp"

namespace kingdom::game::g03
{

// DOL 0x8017dee8 [audited: described by the 43rd-session pass] (SetParam 0x8015d2f8 + vtable +0x58 SetModel)
void cTaskCharGhostInky::Init()
{
	SetParam();
	SetModel();
}

// DOL 0x80180ad0 [audited: blr] blr
void cTaskCharGhostInky::Term()
{
	cTaskCharAreaGhost::Term();
}

cTaskCharGhostInky::cTaskCharGhostInky()
{
	Init();
}

// DOL 0x80180b70 [audited: constant getter vs disassembly]
cTaskChar::eKIND cTaskCharGhostInky::GetKind()
{
	return cTaskChar::eKIND::eKIND_INKY;
}

// DOL 0x80180b78 [audited: constant getter vs disassembly]
string cTaskCharGhostInky::GetModelKey()
{
	return "BIN_GOS_INKY_MDL";
}

// DOL 0x80180b84 [audited: constant getter vs disassembly]
string cTaskCharGhostInky::GetAnmKey()
{
	return "BIN_GOS_INKY_CHA";
}

// DOL 0x80180b90 [audited: constant getter vs disassembly]
string cTaskCharGhostInky::GetSRTAnmKey()
{
	return "BIN_GOS_INKY_TSA";
}

// DOL 0x80180b9c [audited: constant getter vs disassembly]
uint cTaskCharGhostInky::GetPushOutLevel()
{
	return 0u;
}

}
