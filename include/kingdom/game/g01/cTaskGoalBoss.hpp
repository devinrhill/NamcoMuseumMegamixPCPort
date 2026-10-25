#pragma once

#include "kingdom/game/g01/cTaskGoal.hpp"

// Galaga Remix (g01) the goal of stage 9: kingdom::game::g01::cTaskGoalBoss (vtable 0x8043b228, a cTaskGoal). One model m01g_g09.brres per
// skeleton node containing m01g_g09_ (0x800ef130), light set 0, fog 3 (0x800ef8d4); the a01 clips m01g_g09_a01_* (cha / tsa 0.1, the others
// 1.0; 0x800efa00); for its goal demo (cDemoGoalW3S3 0x801bdde0, registered with cDemoMan) the a40 clips at 1.0, the longest one timed
// (0x800efb88). The demo object's own part (cDemoGoalW3S3: ctor 0x801bdde0, Start 0x801be350, Execute 0x801be36c): Demo_s9.csv frames for the
// boss's cues, the bite effect at ef_scissors, the three ANT_LINE letterbox bars and the boss music. Its End (+0x20 = 0x801be940 -> +0x78,
// the game clips) has no caller in the DOL (cDemoMan's Finish calls Term +0x18, which does nothing here).
namespace kingdom { class cTaskAnita; }

namespace kingdom::game::g01
{
class cTaskGoalBoss : public cTaskGoal
{
public:
	cTaskGoalBoss();
	void Setup(int nStage, const wii::Mdl0& skel) override;   // DOL 0x800efd6c (+ the placements 0x800ef130)
	bool Execute(cJetTask* pParam) override;                  // DOL 0x800efe1c

protected:
	void LoadModels() override;                // DOL 0x800ef8d4
	void LoadGame() override;                  // DOL 0x800efa00
	void LoadDemo() override;                  // DOL 0x800efb88
	void GoalInit() override { LoadDemo(); m_fChkLast = 0.0f; }   // cDemoGoalW3S3::Start 0x801be350 -> +0x74: the a40 clips
	bool GoalCheck() override;                                    // cDemoGoalW3S3::Execute 0x801be36c
	// the demo interface: +0x28 0x800efe98 (state 0) / +0x2c 0x800efeb0 (state 1) = cTaskGoal's DemoStart / DemoGoalStart

private:
	// Demo_s9.csv: e_bite01 +0x24, s_appear01, s_boss_voice1_01, s_boss_voice2_01, s_boss_trance_01, s_quake_01, s_ensyutu_01, a_line01,
	// a_line02, a_line03, b_boss01 +0x4c (-1 = done)
	float m_fDemo[11]{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	kingdom::cTaskAnita* m_pLine{};                               // +0x50 ANT_LINE x3 (priority 0x8000000a), hidden
};
}
