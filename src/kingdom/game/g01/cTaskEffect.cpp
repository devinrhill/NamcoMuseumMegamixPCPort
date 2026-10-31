#include "kingdom/game/g01/cTaskEffect.hpp"

#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomPtkMan.hpp"

namespace kingdom::game::g01
{

// DOL 0x8037e248 [audited] (CRC32 of these names)
const char* const cTaskEffect::SLOT_NAME[eSLOT_MAX] = {
	"PTK_BOMB_SMALL01", "PTK_BOMB_SMALL02", "PTK_BOMB_SMALL03", "PTK_BOMB_SMALL04",
	"PTK_BOMB_MIDDLE01", "PTK_BOMB_MIDDLE02", "PTK_BOMB_MIDDLE03", "PTK_BOMB_MIDDLE04",
	"PTK_BOMB_BIG01", "PTK_COMET01", "PTK_COMET02", "PTK_COMET03",
	"PTK_BEAM01", "PTK_BEAM02", "PTK_BEAM03", "PTK_BEAM04",
	"PTK_PACMAN_HIT01", "PTK_CURE01", "PTK_SPIKE_HIT01", "PTK_ENEMY_ATTACK_SIGN01",
	"PTK_PRISONER_JET01", "PTK_ELV01", "PTK_SHINE01", "PTK_POWER01",
	"PTK_JET01", "PTK_LINE01", "PTK_JUMP01", "PTK_SPARK01",
	"PTK_M_COMET01", "PTK_STAR01", "PTK_GALAXY01", "PTK_GALAXY02",
	"PTK_SS01", "PTK_MAP_MIST01", "PTK_MAP_STONE01", "PTK_MIST01",
	"PTK_WATER04", "PTK_TRACTOR01", "PTK_ABDUCT01", "PTK_WATER01",
	"PTK_WATER02", "PTK_WATER03", "PTK_BOM01", "PTK_GOAL01",
	"PTK_MISSILE_JET01", "PTK_QUESTION01", "PTK_EXCLAMATION01", "PTK_THREAT_LINE01",
	"PTK_LB_BOMB01", "PTK_ROCKET_SMOKE01", "PTK_ROCKET_HIT01", "PTK_ROCKET_HIT02",
	"PTK_BARRIER01", "PTK_BITE01", "PTK_BREATH01", "PTK_VOMIT01",
	"PTK_BEAM05", "PTK_ROCKET_BOMB01", "PTK_THREAT01", "PTK_BARRIER_RELEASE01",
	"PTK_ANGRY01", "PTK_ROCKET_SMOKE02", "PTK_GATE_BOMB01", "PTK_WING01",
	"PTK_WARP01", "PTK_PAC_JUMPSTART01", "PTK_PAC_JUMPEND01", "PTK_BEAM06",
};

// DOL 0x801c1ec0 [audited] (and the ctor chain of new 0xd50 in cAppG01GalagaGame::Init 0x801bb258; FLOAT_8047ce28 = 25 at +0x351 not used here)
cTaskEffect::cTaskEffect()
{
	SetName("g01::cTaskEffect");
	get_KingdomPtkMan()->Create(eSLOT_MAX, 100u, 400u, 8000u);
	for (unsigned i = 0; i < eSLOT_MAX; i++) get_KingdomPtkMan()->RegistData(i, SLOT_NAME[i]);
}


// DOL 0x801c1f6c [audited]: the particles step (0x8008ad34 with the rate +0xd44) unless paused, Pac-Man's miss freeze (bUnk9) is on, the rate is 0 or
// pad 0 holds HOME (game bit 0x400). The rate is always 1.0 (only the ctors 0x800aeef0 / 0x80093df4 / 0x801b96f8 store it), so
// UpdateFrame's fixed step stands in for it.
bool cTaskEffect::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (KingdomGlobal::GalagaSys != nullptr && KingdomGlobal::GalagaSys->bUnk9()) return true;
	if ((GamePad::Get(0).hold & GamePad::PAD_HOME) != 0) return true;   // get_pad1(0)->hold & 0x400
	return cTaskPBPtkMan::Execute(pParam);
}

}
