#pragma once

#include <vector>

#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include "kingdom/game/g01/cObject.hpp"

namespace wii { class Mdl0; }

// Galaga Remix (g01) goal area: kingdom::game::g01::cTaskGoal (0x240 bytes, vtable 0x8043b130; cTaskStage +0x228, made by 0x800ee050, set up
// by vtable +0x7c = 0x800eeedc). One model slot per goal gimmick m01g_s%x_g%02d (x = 1 before stage 7, else 7; the first skeleton node
// m01g_s%x_g%02d_ per index while the .brres exists, 0x800ee1e4), models 0x800ee9fc (fog 3), the a01 clips 0x800eeb30 (cha / cla / tsa 0.1
// before world 3, else 1.0; tpa / via 1.0). Stage 3 also registers its goal demo (cDemoGoalW1S3 0x801bd5ac: the a20 clips 0x800eecf0 and
// Demo_s3.csv) with cDemoMan. Stage 9 has cTaskGoalBoss (vtable 0x8043b228, cTaskGoalBoss.hpp) instead.
namespace kingdom::game::g01
{
class cTaskNodeEffect;

class cTaskGoal : public cObject, public kingdom::cTaskModel, public cDemoInterface
{
public:
	// stage 3: cDemoGoalW1S3 (0x801bd5ac), registered with cDemoMan; no opening part, no Term
	void DemoStart() override { m_bDemoDone = true; }   // DOL 0x800ef020 (state 0; cDemoGoalW1S3 has no opening part: done; natively at once)
	bool IsDemoDone() const override { return m_bDemoDone; }
	void DemoFinish() override { m_bDemoDone = true; m_bGoalDone = true; }
	void DemoGoalStart() override { m_nState = 1; m_nStep = 0; m_nGoalStep = 0; m_bGoalDone = false; }   // DOL 0x800ef038
	bool IsGoalDone() const override { return m_bGoalDone; }
	cTaskGoal();                                     // DOL 0x800ee050
	virtual void Setup(int nStage, const wii::Mdl0& skel);   // DOL 0x800eeedc (vtable +0x7c; + 0x800ee1e4 the placements)
	bool Execute(cJetTask* pParam) override;         // DOL 0x800eefa4

protected:
	struct Place { Vector4 t, r, s; int nSet, nIndex; };   // 0x40 bytes each (+0x228 vector): SRT, the x and the index of the name
	virtual void LoadModels();                       // DOL 0x800ee9fc (vtable +0x70)
	virtual void LoadGame();                         // DOL 0x800eeb30 (vtable +0x78)
	virtual void LoadDemo();                         // DOL 0x800eecf0 (vtable +0x74): the a20 clips, the longest one checked
	virtual void GoalInit();                         // cDemoGoalW1S3::Start 0x801bd904
	virtual bool GoalCheck();                        // cDemoGoalW1S3::Execute 0x801bd9ac
	bool ClipCheck();                                // cAnmCheck: the longest clip still runs (native: a wrap ends it too)
	int m_nStage{};                                  // +0x224
	std::vector<Place> m_Place;
	bool m_bDemoDone{}, m_bGoalDone{};
	int m_nGoalStep{};
	int m_nChkModel{-1}, m_nChkKind{};               // cAnmCheck
	float m_fChkMax{}, m_fChkLast{};
	// Demo_s3.csv: e_power01 +0x4c, e_jet01 +0x44, e_spark01 +0x48, s_seridasi01 +0x50, s_catapult01 +0x54, s_booster01 +0x58,
	// s_catapult_start01 +0x5c
	float m_fGoalTime[7]{-1, -1, -1, -1, -1, -1, -1};
	cTaskNodeEffect* m_pEffect[3]{};
};
}
