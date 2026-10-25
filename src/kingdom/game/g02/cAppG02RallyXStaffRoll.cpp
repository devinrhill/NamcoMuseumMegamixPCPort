#include "kingdom/game/g02/cAppG02RallyXStaffRoll.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"

namespace kingdom::game::g02
{

// DOL 0x800c6b80 (job 206)
void cAppG02RallyXStaffRoll::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG02RallyXStaffRoll());
}

// DOL 0x80112b44. The second AttachArcData("staffroll.lzs") of the DOL (PTR 0x8047bb4c; the base 0x801442a8 attaches it too) and its
// detach in the dtor (0x80112b94) are left out natively (as cAppG01GalagaStaffRoll). RegistCamera + the rest = the base RegistData
// (UNVERIFIED that the Rally-X ctor needs more than RegistCamera, as for Galaga).
// NOT PORTED: the BGM m02bgm_Ending (PTR 0x8047bb48 -> +0x28c): EBGMKind / SoundManager have no Rally-X entries (not this agent's files);
// the base default (BGM_001) plays nothing.
cAppG02RallyXStaffRoll::cAppG02RallyXStaffRoll()
{
	nSetNextJobNo(0x0c600000u);   // +0x288: the game over, which goes back to the stage select
	RegistData();
}

}
