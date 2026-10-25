#include "kingdom/game/g11/cTaskGrobdaEffect.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

#include "kingdom/cKingdomPtkMan.hpp"

namespace kingdom::game::g11
{

// the CRCs / names of 0x80035b0c, resolved against the g11 archives (reversing/dol/g11_archives_crc.txt)
const char* const cTaskGrobdaEffect::SLOT_NAME[eSLOT_MAX] = {
	"BIN_EFF_SHIELD_L_PLAYER_01", "BIN_EFF_SHIELD_L_PLAYER_02", "BIN_EFF_SHIELD_L_PLAYER_03", "BIN_EFF_SHIELD_L_PLAYER_04",
	"BIN_EFF_SHIELD_M_PLAYER_01", "BIN_EFF_SHIELD_M_PLAYER_02", "BIN_EFF_SHIELD_M_PLAYER_03", "BIN_EFF_SHIELD_M_PLAYER_04",
	"BIN_EFF_SHIELD_S_PLAYER_01", "BIN_EFF_SHIELD_S_PLAYER_02", "BIN_EFF_SHIELD_S_PLAYER_03", "BIN_EFF_SHIELD_S_PLAYER_04",
	"BIN_EFF_SHOT_PLAYER_01", "BIN_EFF_SHOT_PLAYER_02", "BIN_EFF_SHOT_PLAYER_03", "BIN_EFF_SHOT_PLAYER_04",
	"BIN_EFF_SHOT_ENEMY_01", "BIN_EFF_SHOT_ENEMY_02", "BIN_EFF_SHOT_ENEMY_03", "BIN_EFF_SHOT_ENEMY_04",
	"BIN_EFF_SHOT_ENEMY_05", "BIN_EFF_SHOT_ENEMY_06", "BIN_EFF_SHOT_ENEMY_07", "BIN_EFF_HIT",
	"BIN_EFF_BOMB", "BIN_EFF_BOMB_GRENADE", "BIN_EFF_GRENADE_LOCUS_01", "BIN_EFF_GRENADE_LOCUS_02",
	"BIN_EFF_GRENADE_LOCUS_03", "BIN_EFF_GRENADE_LOCUS_04", "BIN_EFF_SHIELD_ENEMY", "m11e_24_fragment01.pto",
	"BIN_EFF_SHORT", "m11e_25_p1p01.pto", "m11e_26_p2p01.pto", "m11e_27_p3p01.pto",
	"m11e_28_p4p01.pto",
};

// DOL 0x80035b0c
cTaskGrobdaEffect::cTaskGrobdaEffect()
{
	SetName("cTaskGrobdaEffect");
	get_KingdomPtkMan()->Create(eSLOT_MAX, 0x100u, 0x800u, 0x4000u);
	for (unsigned i = 0; i < eSLOT_MAX; i++) get_KingdomPtkMan()->RegistData(i, SLOT_NAME[i]);
}

void cTaskGrobdaEffect::Spawn(uint slot, const float pos[3], const float rot[3])
{
	static kingdom::cKingdomPtkHnd* ring[64] = {};
	static int n = 0;
	kingdom::cKingdomPtkHnd*& h = ring[n++ & 63];
	if (h == nullptr) h = new kingdom::cKingdomPtkHnd();
	h->Generate(slot, true, true, true, 0.0f, Vector4(pos[0], pos[1], pos[2], 1.0f),
	            rot != nullptr ? Nullable<Vector4>(Vector4(rot[0], rot[1], rot[2], 1.0f)) : Nullable<Vector4>(nullptr));
}

}
