#pragma once

#include "kingdom/game/g01/cTaskEnemy.hpp"

// Galaga Remix (g01) the rescued fighter and its cookie.
//
// kingdom::game::g01::cTaskFighter (0x3d0 bytes, vtable 0x8043a9e0, a cTaskEnemy of type 10 = CHAR_FIGHTER_MDL, kind 5; ctor 0x800e9f60).
// The enemy manager keeps a pool of 2 (+0x1e8, tWorkObject<cTaskFighter>, table 0x80373138 column 7). When a scn record's p2 bits 9-10 are
// set, Spawn (0x800e6d24) puts a fighter on the spawned enemy's orbit (0x800ea304): hidden, it waits FighterParam.csv WAIT_MAX_FRAME frames,
// then shows and flies the orbit with that enemy (its "leader"). If the leader is shot down the fighter is freed: it homes in on Pac-Man's
// mouth point (cTaskPacman +0x290, node boss_bite) at SPEED, turning at most ROT_LIMIT_INIT rad per frame (+0.02 every frame, 0x800ea614);
// if the leader just left, so does the fighter. When it reaches Pac-Man (its last step's segment against a 0.35 sphere, 0x800ea4c4) it lets
// its cookie go (0x800ea550), plays CHAR_FIGHTER_ANM_CHA_01 and climbs away (turning toward +y) until 150 units from the camera.
//
// kingdom::game::g01::cTaskCookie (0x370 bytes, vtable 0x8043ab1c, kind 4, CHAR_COOKIE_MDL; made by the fighter's ctor, a child of it, registered
// for collision): rides on the fighter's node ef_cooky (+0x390) hidden; once let go it shows and falls toward Pac-Man at COOKIE_FALL_SPEED
// (moving along with him, 0x800e9d4c). When it touches him (0x800e9c48) it is gone and Pac-Man eats it (cTaskPacman::HitCallback kind 4:
// m01se_23kiraan, effect 0x11, the life gauge heals one step).
namespace kingdom::game::g01
{
class cTaskNodeEffect;
class cTaskFighter;

class cTaskCookie : public cObjectAABB, public kingdom::cTaskModel
{
public:
	explicit cTaskCookie(cTaskFighter* pFighter);  // DOL 0x800e9f60 (the cookie part) + vtable +0xa4 = 0x800e9c04
	void Attach(float fFallSpeed);                 // the cookie part of 0x800ea304
	void Release() { m_bAttached = false; m_b09 = true; }   // the fighter's hit callback 0x800ea550
	bool Execute(cJetTask* pParam) override;       // DOL 0x800e9d4c
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x800e9f3c (active and shown)
	bool HitCheck(cObject* other) override;        // DOL 0x800e9c48
	void HitCallback(cObject* other) override;     // DOL 0x800e9d0c

private:
	cTaskFighter* m_pFighter{};                    // +0x328
	float m_fFallSpeed{};                          // +0x330
	bool m_bAttached{true};                        // +0x334
	float m_vLastPac[3]{};                         // +0x340 Pac-Man's position last frame
	float m_vSeg[6]{};                             // +0x350 this frame's segment (from, to)
};

class cTaskFighter : public cTaskEnemy
{
public:
	cTaskFighter();                                // DOL 0x800e9f60
	void Setup(const sScnRecord* pRec, int nOrbit, cTaskEnemy* pLeader);   // DOL 0x800ea304
	bool Execute(cJetTask* pParam) override;       // DOL 0x800e4704 -> vtable +0xa0 = 0x800eaa3c
	bool HitCheck(cObject* other) override;        // DOL 0x800ea4c4
	void HitCallback(cObject* other) override;     // DOL 0x800ea550
	const float* vCookiePoint() const { return m_vCookie; }   // +0x390 the world position of node ef_cooky

private:
	void LeaveStep();                              // DOL 0x800ea9a8 (vtable +0x84)
	void Turn(float fStep, const float to[3]);     // DOL 0x800ea614 (vtable +0xcc)
	bool NodeWorld(const char* node, float out[3]);
	struct sParam { float fWaitMax{}, fSpeed{}, fRotLimitInit{}, fCookieFallSpeed{}; };   // FighterParam.csv (0x14 bytes, vtable 0x8043ab10)
	sParam m_Param;                                // +0x364
	cTaskEnemy* m_pLeader{};                       // +0x358
	cTaskCookie* m_pCookie{};                      // +0x35c
	bool m_bRiding{true};                          // +0x368 on the leader's orbit
	bool m_bDelivered{};                           // +0x369 the cookie is let go: climb away
	float m_fSpeed{};                              // +0x36c
	float m_vDir[3]{0, 0, 1};                      // +0x370 flight direction once freed
	float m_vPrev[3]{};                            // +0x380 the position before this frame
	float m_vCookie[3]{};                          // +0x390
	float m_fWait{};                               // +0x3a0
	float m_fRotLimit{};                           // +0x3a4
	float m_vSeg[6]{};                             // +0x3a8 this frame's segment (0x800eb06c)
	cTaskNodeEffect* m_pBoost[2]{};                // +0x3c0 / +0x3c4 ef_l_boost / ef_r_boost (slot 0x14, looping)
};
}
