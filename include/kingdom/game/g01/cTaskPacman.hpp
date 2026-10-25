#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) Pac-Man: kingdom::game::g01::cTaskPacman (0x470 bytes, vtable 0x8043b750; cObjectAABB at +0, cTaskModel at +0x88).
// He rides the course: model 0 is the skeleton CHAR_PACMAN_ORBIT_MDL (nodes position / pac_orbit / pac_move) animated by the stage's
// m01m_s%x_pac_ob_cha clip in step with the course camera, with CHAR_PACMAN_MDL expanded at "pac_orbit" (DAT_8047b6a0).
// Ported from main.dol; the old hand-reversed tree (reversing/dol/g01_ghidra/cTaskPacman.cpp) was the guide.
namespace kingdom { class cKingdomPtkHnd; }

namespace wii { struct Mdl0Node; }

namespace kingdom::game::g01
{
class cTaskScene;
class cTaskConsoleLife;
class cTaskNodeEffect;

class cTaskPacman : public cObjectAABB, public kingdom::cTaskModel, public cDemoInterface
{
public:
	// stage 1 opening demo (cDemoPacmanW1S1, 0x800f3eec) and the goal demos of stages 3 / 6 / 9 / 10
	void DemoStart() override { m_nState = 0; m_nStep = 0; m_bDemoDone = false; m_nDemoStep = 0; }   // DOL 0x800f3d28
	bool IsDemoDone() const override { return m_bDemoDone; }
	void DemoFinish() override;   // cDemoPacmanW1S1::Term 0x801beefc: done, vtable +0xd0 (the start clip 0x800f5ab0)
	void DemoGoalStart() override { m_nState = 1; m_nStep = 0; m_nGoalStep = 0; m_bGoalDone = false; }
	bool IsGoalDone() const override { return m_bGoalDone; }
	// DOL 0x800f40f0 (after new 0x470): stage, scene, enemy manager / console player (natively the globals cTaskEnemyMan::s_pInstance and SetLife)
	cTaskPacman(int nStage, cTaskScene* pScene);
	~cTaskPacman() override { if (s_pInstance == this) s_pInstance = nullptr; }
	bool Execute(cJetTask* pParam) override;    // DOL 0x800f5e4c
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x800f600c: only while active (+8) and shown (+9)

	void ReadyStart();                          // DOL 0x800f3e2c (state 3) + the start clip (vtable +0xd0 = 0x800f5ab0)
	bool IsReady() const { return m_nState == 4; }     // DOL 0x800f3e18
	void GameStart() { m_nState = 5; m_nStep = 0; }    // DOL 0x800f3e40
	bool IsGoal() const { return m_nState == 9; }      // DOL 0x800f3ebc
	bool IsGoalRun() const { return m_nState == 8; }   // DOL 0x800f3e94 (the shots leave, 0x800d24b0)
	const float* vAimPoint() const { return m_vAimPoint; }   // +0x2f0 (0x800f4388): the dummy shots' target
	const float* vBite() const { return m_vBite; }           // +0x290 (0x800f4388): node boss_bite (the boss's bite, the fighter's target)
	bool IsMissEnd() const { return m_nState == 7; }   // DOL 0x800f3e68
	void MissStart() { m_nState = 6; m_nStep = 0; }    // DOL 0x800f3e54
	void SetLife(cTaskConsoleLife* p) { m_pLife = p; } // +0x3a8 (the console life the ctor 0x800f40f0 gets)
	void HitCallback(cObject* other) override;         // DOL 0x800f4f20
	bool bJumping() const override { return m_nJump == 1 || m_nJump == 2; }   // DOL 0x800f4bc4 (+0x2b4 in {1, 2})
	void RequestJump();                                // DOL 0x800f3ed0 (vtable +0xf8): unless the scene forbids it (+0x240)
	static cTaskPacman* s_pInstance;                   // the global Pac-Man (r13 - 0x56e0)

protected:
	void LoadModel();          // DOL 0x800f41d4 (vtable +0xac)
	void SetStartClip();       // DOL 0x800f5ab0
	virtual void SetCourseClip();   // DOL 0x800f5bf0 (vtable +0xd4)
	virtual void HitClip();         // DOL 0x800f5d94 (vtable +0xa0)
	virtual void HitEnd();          // DOL 0x800f5e00 (vtable +0xa4; cTaskBossPacman 0x800d0f20)
	virtual void PreGameStep() {}   // cTaskBossPacman: the start of its game step (0x800d0dd0: the course segment changed)
	void SyncCourse();         // DOL 0x800f42c8 (vtable +0xd8): frame and rates from the scene
	void GameStep();           // DOL 0x800f53cc (vtable +0x7c)
	void SetDemoClip(int n);   // DOL 0x800f520c (vtable +0xc8, n = 1) / 0x800f5860 (vtable +0xcc, n = 2..4)
	void GoalInit();           // cDemoPacmanW1S3::Start 0x801bf138
	bool GoalCheck();          // cDemoPacmanW1S3::Execute 0x801bf294
	bool NodeWorld(const char* node, float out[3]);
	bool NodeMtx(const char* node, float out[16]);   // the same as a whole matrix (the goal effects, 0x800e11fc)
	void JumpStep();           // DOL 0x800f4be8 (vtable +0xbc)
	void MissStep();           // DOL 0x800f5590 (vtable +0x80, state 6)
	void MissClip();           // DOL 0x800f5ca4 (vtable +0x9c): CHAR_PACMAN_ANM_CHA_03 + TPA_03 at rate 1
	bool IsMissClipEnd();      // DOL 0x800f5d30 (vtable +0xa8)
	// the goal run (state 8, 0x800f5708): along the goal skeleton's pac_goal_%03d nodes, then the arrival clips; then state 9
	void GoalRun();            // DOL 0x800f5708 (vtable +0x84)
	bool GoalStop();           // DOL 0x800f30f4 (vtable +0x88)
	bool GoalNext();           // DOL 0x800f3394 (vtable +0x8c): true when no next node
	bool GoalMove();           // DOL 0x800f386c (vtable +0x90): true at the node
	void GoalClip();           // DOL 0x800f3b44 (vtable +0x94)
	bool GoalBlend();          // DOL 0x800f3c14 (vtable +0x98): true when the arrival clip ended

	int m_nStage{};            // +0x260
	int m_nWorld{};            // +0x264
	cTaskScene* m_pScene{};    // +0x3a0
	float m_fLastFrame{-1.0f}; // native: the scene's "frame was set" flag (+0x1fa) is seen as a jump of the frame
	cTaskConsoleLife* m_pLife{};   // +0x3a8
	float m_fInvincible{};     // +0x2e4 frames left (PacmanParam.csv MAX_INVINCIBLE, +0x25c +0xc)
	float m_fMaxInvincible{30.0f};
	kingdom::cKingdomPtkHnd* m_pHitEffect{};   // +0x300
	kingdom::cKingdomPtkHnd* m_pCookieEffect{};   // +0x350
	kingdom::cKingdomPtkHnd* m_pGoalPtk{};        // +0x410 PTK_GOAL01 (0x800f3b44)
	float m_vOrbitRot[3]{};    // +0x280 the CHR "pac_orbit" rotation in radians (0x800f4388)
	int m_nMissTimer{};
	bool m_bDemoDone{};
	int m_nDemoStep{};
	float m_fDemoMax{};        // cAnmCheck +8 (the longest a10 clip)
	int m_nDemoKind{};         // cAnmCheck +4: 0 cha, 3 tpa, 4 via
	float m_fDemoLast{};
	float m_fDemoLastOrbit{};
	// the swing jump (0x800f4be8); PacmanParam.csv GRAVITY / JUMP_START_SPEED / JUMP_MAX_RIGIDITY (+0x25c +0 / +4 / +8)
	bool m_bJumpReq{};         // +0x2b0 (cleared every frame by Execute)
	int m_nJump{};             // +0x2b4 0 ground, 1 start, 2 air, 3 landed (rigidity)
	float m_fJumpVel{};        // +0x2bc
	float m_fJumpWait{};       // +0x2c0
	float m_fJumpH{};          // +0x2c4
	float m_vUp[3]{0, 1, 0};   // +0x2a0 Pac-Man's up (0x800f4388)
	float m_vJumpOfs[3]{};     // +0x2d0 height * up
	float m_vBite[3]{};        // +0x290
	float m_vBiteNode[3]{};    // the orbit model's node boss_bite (bind translate)
	float m_vAimPoint[3]{};    // +0x2f0 position + (-3, -3, -3) * normalize(R(pac_orbit) * (0, 0, 1)) (DAT_80374e00 / DAT_80374e80)
	float m_fGravity{0.065f}, m_fJumpStart{0.6f}, m_fRigidity{10.0f};
	float m_fLift{0.3873f};    // -pac_move y
	// the goal demo (cDemoPacmanW1S3 ...)
	int m_nGoalStep{};
	bool m_bGoalDone{};
	float m_fGoalLast{};
	float m_fGoalTime[6]{-1, -1, -1, -1, -1, -1};   // e_jump01 +0x24, e_shine01 +0x28, e_line01 +0x2c, s_shine01 +0x3c, s_jump01 +0x40, s_set01 +0x44
	cTaskNodeEffect* m_pGoalEffect[4]{};       // W1S3 +0x30 / +0x34 / +0x38; W2S3 +0x3c (ef_body) / +0x40 (the boss's ef_mouth); W3S3 +0x5c..+0x68 (ef_top)
	// cDemoPacmanW2S3 (stage 6, Demo_s6.csv): e_abduct01 +0x24, e_tractor01 +0x28, s_fly01 +0x2c, s_tractor01 +0x30, s_appear01 +0x34;
	// cDemoPacmanW3S3 (stage 9, Demo_s9.csv): e_question01 / 02, e_exclamation01 / 02 (+0x24..+0x30), s_pac_bound_01..10 (+0x34..+0x58)
	float m_fGoalTime2[14]{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int m_nGoalSub{};                          // the demo object's +4 (W2S3: the boss model made)
	// the goal run
	int m_nGoalNode{1};                        // +0x2e0 the next pac_goal_%03d index
	const wii::Mdl0Node* m_pGoalNode[2]{};     // +0x3fc / +0x400 from / to
	float m_fGoalPos{};                        // +0x3f4 frames run (also the arrival blend's counter)
	float m_fGoalFrames{1.0f};                 // +0x3f8 the to node's user data pac_trans_frame
	bool m_bGoalRoll{};                        // +0x404 the to node's pac_roll_anm (the roll clip keeps playing)
	float m_mGoalRot[9]{1, 0, 0, 0, 1, 0, 0, 0, 1};   // +0x3b0 Rz Ry Rx of pac_orbit at the stop (row major 3x3)
};
}
