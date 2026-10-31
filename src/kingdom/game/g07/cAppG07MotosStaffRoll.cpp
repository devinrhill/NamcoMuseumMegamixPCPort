#include "kingdom/game/g07/cAppG07MotosStaffRoll.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include <EBGMKind.hpp>
#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g07
{

// DOL 0x800c5348 [audited: per AUDIT_G03_G07 row] (job init 0x800a311c): AttachArcData("staffroll.lzs", 1), BGM "m07bgm_Ending" (+0x28c), next job 0x11500000 (+0x288), RegistCamera
void cAppG07MotosStaffRoll::Init()
{
	pSetBgmName(EBGMKind::MOTOS_SND_BGM07_ENDING);
	nSetNextJobNo(kingdom::job::MOTOS_GAMEOVER);
	RegistData();
}

// DOL 0x800c5398 [audited: per AUDIT_G03_G07 row] (dtor body): DetachArcData("staffroll.lzs")
void cAppG07MotosStaffRoll::Term()
{
	KingdomGlobal::KingSys->DetachArcData("staffroll.lzs");
}

cAppG07MotosStaffRoll::cAppG07MotosStaffRoll()
{
	Init();
}

void cAppG07MotosStaffRoll::Free()
{
	Term();
	cAppGameBaseStaffRoll::Free();
}

}
