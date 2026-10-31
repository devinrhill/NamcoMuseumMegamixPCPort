#pragma once

#include "common.hpp"
#include "WiiSnd.hpp"
#include "kingdom/game/g11/cTaskGrobdaModel.hpp"

namespace kingdom { class cKingdomPtkHnd; }

// Grobda Remix (g11) player tank: kingdom::game::g11::cTaskGrobdaPlayer (0xc80 bytes, vtable 0x80449230; ctor 0x80194d0c(p, 0x10000001 + player), Init 0x80194e74, Execute 0x80195a0c,
// Resolve 0x80198454 -> damage 0x80197d30, steering 0x80196124 / input 0x80196054, control 0x801987c8, grenade 0x80197508, energy 0x80197894, shield effects 0x80197ae4,
// respawn 0x80199094, end 0x80199478). States (+0xbb8): 0 wait for the stage, 1 control, 2 destroyed (explosion, RECV_WAIT frames), 3 respawn (drops from RECV_HEIGHT), 4 stage over
// (settles on the ground). The object flag word (cGrobdaObject +0x93c): 2 shield held, 4 moving, 8 / 0x10 recovering (blinking, invulnerable).
// PlayerParam.csv rows (0-based): 0 SPEED_MIN 1 SPEED_MAX 2 SPEED_DEBRIS 3 ACCELE 4 GRAVITY 5 SHOT_SPEED 6 SHOT_INTERVAL 7 CANNON_TURN 8 BODY_TURN 9 EN_MAX 10 EN_RECV_STOP 11 EN_RECV_MOVE
// 12 EN_SHOT 13 EN_SHIELD 14 EN_DEFENSE 15 EN_GRENADE 16 SHIELD_EN_MIDDLE 17 SHIELD_EN_SMALL 18 GRE_SWING 19 GRE_SPEED 20 GRE_ANGLE 21 GRE_GRAVITY 22 RECV_WAIT 23 RECV_HEIGHT
// 24 RECV_INVINCIBLE 25 STEAL_SCORE_LASER 26 STEAL_SCORE_BOMB. One energy bar pays for the laser (per frame while firing), the shield (per frame) and the grenade (all of it).
namespace kingdom::game::g11
{
class cTaskGrobdaShotPlayer;
class cTaskGrobdaShotGrenade;

class cTaskGrobdaPlayer : public cTaskGrobdaModel
{
public:
	explicit cTaskGrobdaPlayer(int player);           // DOL 0x80194d0c + Init 0x80194e74
	void SetStart(float x, float z, float angle);     // DOL 0x8003822c + 0x80199818
	~cTaskGrobdaPlayer() override;
	bool Execute(cJetTask* pParam) override;          // DOL 0x80195a0c
	void Resolve(int a, int b) override;              // DOL 0x80198454
	bool IsFiring() const { return m_bFiring; }       // +0xbbe: a shot left this frame (the Frossors dodge it)
	const float* Aim() const { return m_vAim; }       // +0xbf0

private:
	void Steer();                                     // DOL 0x80196124
	void Fire();                                      // DOL 0x801970d0
	void Control();                                   // DOL 0x801987c8
	void Grenade();                                   // DOL 0x80197508
	void Energy();                                    // DOL 0x80197894
	void ShieldEffects();                             // DOL 0x80197ae4
	void Damage(std::vector<const sGrobdaHit*>& hits);   // DOL 0x80197d30
	void Respawn();                                   // DOL 0x80199094
	void Settle();                                    // DOL 0x80199478
	void SetAnim(int anim);                           // the clip pair m11m_c01_aNN / m11m_p01_aNN (+0xbc8 = the anim id)
	bool AnimDone();                                  // the clip ran out (frame past its end, or wrapped)
	bool Shielding() const { return HasFlag(2); }

	int m_nPlayer{};
	int m_nState{0};                                  // +0xbb8
	bool m_bEnter{true};                              // +0xbbc
	bool m_bOnDebris{};                               // +0xbbd
	bool m_bFiring{};                                 // +0xbbe
	int m_nTimer{0};                                  // +0xbc0 (RECV_WAIT, then the invulnerable frames)
	int m_nBlink{0};                                  // +0xbc4
	int m_nAnim{-1};                                  // +0xbc8: 0 idle a01, 1 move a02, 2 grenade a03, 3 shield a04, 4 death a06, 5 turn a07, 6 turn a08
	float m_fFrom{0.0f}, m_fTo{0.0f};                 // +0xbcc / +0xbd0
	int m_nShotWait{0};                               // +0xbd4
	float m_fEnergy{0.0f};                            // +0xbd8
	float m_vPos[4]{};
	float m_vSpawn[4]{};                              // +0xbe0
	float m_vAim[4]{0.0f, 0.0f, 1.0f, 0.0f};          // +0xbf0 (cannon direction)
	float m_vBody[4]{0.0f, 0.0f, 1.0f, 0.0f};         // +0xc00 (body direction)
	float m_fSpeed{};                                 // +0xc10
	// PlayerParam.csv (+0xc14 ..)
	float m_fSpeedMin{0.05f}, m_fSpeedMax{0.25f}, m_fSpeedDebris{0.03f}, m_fAccele{0.005f}, m_fGravity{0.05f}, m_fShotSpeed{0.6f};
	int m_nShotInterval{5};
	float m_fCannonTurn{0.1396f}, m_fBodyTurn{0.0873f};   // radians per frame
	float m_fEnMax{2000.0f};
	int m_nEnRecvStop{5}, m_nEnRecvMove{2}, m_nEnShot{4}, m_nEnShield{5}, m_nEnDefense{4}, m_nEnGrenade{2000};
	float m_fShieldMiddle{0.5f}, m_fShieldSmall{0.25f}, m_fGreSwing{3.0f}, m_fGreSpeed{0.5f}, m_fGreAngle{1.309f}, m_fGreGravity{0.01f};
	int m_nRecvWait{120};
	float m_fRecvHeight{10.0f};
	int m_nRecvInvincible{120};
	int m_nStealLaser{1000}, m_nStealBomb{3000};
	cTaskGrobdaShotPlayer* m_pShot[16]{};             // +0xb5c
	cTaskGrobdaShotGrenade* m_pGrenade{};             // +0xb9c
	kingdom::cKingdomPtkHnd* m_pShield[3]{};          // +0xb50 (L / M / S)
	kingdom::cKingdomPtkHnd* m_pMarker{};             // +0xb4c (multiplayer only)
	WiiSnd::Handle m_ShieldSnd, m_AlarmSnd;           // +0xba0 / +0xbac
};
}
