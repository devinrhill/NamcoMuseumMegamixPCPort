#pragma once

#include <algorithm>

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cObject.hpp"

namespace kingdom { class cTaskCursor; }

// Galaga Remix (g01) player: kingdom::game::g01::cTaskConsolePlayer (vtable 0x80439918; one per player under cTaskGameConsoleMan). Besides its
// HUD panel (score digits ported; the life / kill counters and the rest of cTaskGameConsoleBase are not) it is the shooter: while A or B is held and the remote points at the screen it fires
// beams (PlayerParam.csv BEAM_MAX shots, BEAM_BLANK_FRAME apart, then BEAM_WAIT_FRAME), each a ray from the camera through the aim point,
// tested by the collision pass (it is the object of kind 1) against the enemies' world AABBs; the first (nearest) hit takes the shot.
namespace kingdom::game::g01
{
// DOL kingdom::game::g01::cTaskBeam (vtable 0x80438850): the laser flash of one shot. Init 0x800c7ff8 (ANT_BEAM 0x1f3e9d31, laser anim
// DAT_80371360[player] = 1 + 2 * player), Execute 0x800c80a4 (state 1: play the anim, free (+9 = 0) at its end; the anita sits at the aim
// point moved down by 240), Draw 0x800c821c (its own 640 x 480 ortho projection). The console player keeps BEAM_MAX of them.
class cTaskBeam : public cObject, public kingdom::cTaskAnita
{
public:
	explicit cTaskBeam(int nPlayer);           // DOL 0x800c7ff8
	bool IsFree() const { return !m_b09; }
	void Start(const Vector4& aim);            // the anita part of 0x800c8310
	bool Execute(cJetTask* pParam) override;   // DOL 0x800c80a4

private:
	int m_nPlayer{};                           // +0x218
	Vector4 m_vAim{};                          // +0x230 (nOldState(0) in the old tree)
};

class cTaskConsolePlayer : public cObject, public kingdom::cTaskAnita
{
public:
	cTaskConsolePlayer(int nPlayer, int nPad, int nPlayers);   // DOL 0x800d665c (Init: console mode = player count, player, matching data)
	bool Execute(cJetTask* pParam) override;     // DOL 0x800d6080 -> 0x800d60c4 (the shot part)
	void Draw(int nStatus, cJetTask* pParam) override;   // cTaskGameConsoleBase::Draw 0x800eb548: only while active (+8) and shown (+9)
	bool HitCheck(cObject* other) override;      // DOL 0x800d7198
	void HitCallback(cObject* other) override;   // DOL 0x800d7344
	void AddScore(int n) { m_nScore = std::clamp(m_nScore + n, 0, 999999); }   // DOL 0x800d73cc: +0x244 += n, clamped to 0..999999
	void AddKill() { m_nKill++; }                // the enemy's HitCallback increments +0x234
	void SetEnable(bool b) { m_bEnable = b; }    // +0x24c (cTaskGameConsoleBase 0x800ec2cc: on in READY / GAME)
	void Appear() { m_nPanelStep = 0; m_bPanel = true; }   // starts 0x800d572c (the console's READY call, vtable 28; UNVERIFIED wiring)
	void StartResult() { m_bPanel = false; m_bResult = true; m_nResultStep = 0; }   // console manager +0x80 -> 0x800eb6c4 (state 4)
	bool IsResultEnd() const { return !m_bResult; }   // state 5
	int nScore() const { return m_nScore; }
	int nKill() const { return m_nKill; }
	int nShot() const { return m_nShot; }
	int nHit() const { return m_nHit; }
	int nRateCarry() const { return m_nRateCarry; }   // +0x240: -1, or (stage 10) the hit rate of stage 9 in 1/1000
	int nPlayer() const { return m_nPlayer; }
	const float* vHitPos() const { return m_vHitPos; }   // beam +0x260: where this frame's shot met the last box it was tested against (0x800c8764)

protected:
	virtual bool ResultExec();                   // DOL 0x800d6444: the panel and digits go out (vtable slot 0x800d6444; cTaskConsoleMultiPlayer: 0x800d740c)
	int m_nResultStep{};                         // +0x220 in state 4

private:
	void Fire();                                 // the beam pool part of 0x800d60c4 + cTaskBeam 0x800c8310 (the ray)
	void LoadCarry();                            // DOL 0x800d65cc (vtable +0x88): stage 10 starts from stage 9's numbers
	void PanelExec();                            // DOL 0x800d572c: panel appear, then the score digits; then 0x800d60c4's panel part
	void DigitExec(int kind);                    // DOL 0x800d5aa0: 1 = the changed digits pop (anitas 1..6 + 7..12), 2 = all go out
	int m_nPlayer{};                             // +0x228
	int m_nPad{};                                // +0x22c
	int m_nScore{};                              // +0x244
	int m_nScorePrev{};                          // +0x248 the score the digits last showed
	int m_nKill{};                               // +0x234
	int m_nHit{};                                // +0x238
	int m_nShot{};                               // +0x23c
	int m_nRateCarry{-1};                        // +0x240
	bool m_bEnable{};                            // +0x24c
	bool m_bShotHit{};                           // +0x24d this frame's shot already hit
	bool m_bShot{};                              // +0x258 != 0: a beam was fired this frame
	float m_vRayOrg[3]{}, m_vRayDir[3]{};        // beam +0x240 / +0x24c
	float m_vHitPos[3]{};                        // beam +0x260
	int m_nBurst{};                              // +0x260
	float m_fWait{};                             // +0x264
	int m_nBeamMax{12}, m_nBlank{6}, m_nWaitFrame{33};   // PlayerParam.csv (cPlayerParam, +0x25c)
	kingdom::cTaskCursor* m_pAim{};              // +0x250 cTaskAim (ANT_BEAM crosshair, anim 2 * player)
	cTaskBeam* m_pBeam[16]{};                     // +0x254 BEAM_MAX beams
	int m_nPanel{};                              // +0x230 panel kind: the player, 4 when alone
	int m_nPanelStep{};                          // +0x220
	bool m_bPanel{};
	bool m_bResult{};
};
}
