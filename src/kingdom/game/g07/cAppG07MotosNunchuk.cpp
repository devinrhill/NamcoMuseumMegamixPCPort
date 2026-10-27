#include "kingdom/game/g07/cAppG07MotosNunchuk.hpp"
#include "kingdom/KingdomJobTable.hpp"

#include "InGamePadManager.hpp"
#include "KingdomGlobal.hpp"
#include "WiiPad.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

namespace kingdom::game::g07
{

// DOL 0x801c5574: attach g07_motos/g07menu.lzs (0x800a0834), layout crc 0x21d4da2a = ANT_NUNCHUK (g07menu.lzs), next job 0x11300000 (the game);
// then 0x800a0888 starts the asynchronous load of the stage archive (g07_motos/g07wXXsXX.lzs): the native game job loads its stage itself.
cAppG07MotosNunchuk::cAppG07MotosNunchuk()
{
	KingdomGlobal::KingSys->AttachArcData("g07_motos/g07menu.lzs");
	RegistLayout("ANT_NUNCHUK");
	nNextJob() = job::MOTOS_GAME;
	SetName("cAppG07MotosNunchuk");
}

// DOL 0x801c55d4: on a multiplayer stage (index 0x24..0x2d) every matched player's pad needs a Nunchuk, else pad 0
bool cAppG07MotosNunchuk::IsSkip()
{
	auto* sys = KingdomGlobal::MotosSys;
	if (sys != nullptr && sys->nStageIdx() - 0x24u < 10u && sys->pGetMatchingData() != nullptr)
	{
		const sMatchingData* m = sys->pGetMatchingData();
		for (uint i = 0; i < m->nNum; i++)
			if (!WiiPad::NunchukConnected(InGamePadManager::GetPadNum((int)m->Info[i]->nCtrl))) return false;
		return true;
	}
	return WiiPad::NunchukConnected(0);
}

}
