#include "kingdom/game/g01/cAppG01GalagaStaffRoll.hpp"

#include <EBGMKind.hpp>

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"

namespace kingdom::game::g01
{

// DOL 0x800b150c (job 183)
void cAppG01GalagaStaffRoll::JobInit()
{
	KingdomGlobal::SysCore->RegistMainTask(new cAppG01GalagaStaffRoll());
}

// DOL 0x800b3490 (staffroll.lzs is attached by the base 0x801442a8 too). +0x288 = 0x0b500000 is set as the next job (UNVERIFIED: that it
// is the field nSetNextJobNo writes; the Pac-Motos staff roll sets its own end job the same way)
cAppG01GalagaStaffRoll::cAppG01GalagaStaffRoll()
{
	pSetBgmName(EBGMKind::GALAGA_SND_BGM07_ENDING);   // DAT_8047aff8 m01bgm_Ending (+0x28c)
	nSetNextJobNo(0x0b500000u);
	RegistData();
}

}
