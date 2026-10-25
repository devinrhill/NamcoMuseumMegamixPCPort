#pragma once

#include <string>
#include <vector>

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include "kingdom/game/g01/cObject.hpp"

namespace wii { struct Mdl0Node; class Mdl0; }
namespace kingdom { class cKingdomPtkHnd; }

// Galaga Remix (g01) stage scenery. The stage archive holds a skeleton model m01b_s<N>_bg_s whose nodes are the placement table: a node named
// <model>_<NNN> puts an instance of <model> at the node's translate / rotate / scale (cObject +0x20 / +0x30 / +0x40, see cTaskBg 0x800c8b78).
namespace kingdom::game::g01
{
// DOL kingdom::game::g01::cTaskBg (0x230 bytes, vtable 0x804388f8): one background model instance (node "m01b_s%x_bg_")
class cTaskBg : public cObject, public kingdom::cTaskModel, public cDemoInterface
{
public:
	cTaskBg() = default;
	// stage 1 opening demo (cDemoBgW1S1, 0x800c89dc) and stage 10's goal demo (cDemoBgBoss: no opening part, IsWaitEnd 0x800c962c = 0)
	void DemoStart() override { m_nState = 0; m_nStep = 0; m_bDemoDone = false; m_nDemoStep = 0; }   // DOL 0x800c95e0 (vtable +0x84)
	bool IsDemoDone() const override { return m_bDemoDone; }
	void DemoFinish() override;   // cDemoBgW1S1::Term 0x801bcf44: done, vtable +0x78 (0x800c8ddc, the a01 clips again); cDemoBgBoss: nothing
	void DemoGoalStart() override { m_nState = 1; m_nStep = 0; m_nDemoStep = 0; m_bGoalDone = false; }   // DOL 0x800c95f8 (vtable +0x88)
	bool IsGoalDone() const override { return m_bGoalDone; }
	// DOL 0x800c8b78 (stage, node) -> vtable +0x74 = 0x800c89dc (demo object for stages 1 / 10: the bg's states 0 / 1) -> +0x78 = 0x800c8ddc
	void Setup(const wii::Mdl0Node& node, int nStage);
	bool Execute(cJetTask* pParam) override;   // DOL 0x800c9388

private:
	void LoadModel();   // DOL 0x800c8ddc: m01b_s%x_bg.brres + the a01 animations
	void LoadDemoModel();   // DOL 0x800c8fe8 (vtable +0x7c): the a10 clips of the stage 1 demo; the longest one is the demo's length
	void LoadGoalModel();   // DOL 0x800c91a0 (vtable +0x80, cDemoBgBoss::Start 0x801bcf8c): the model again + m01b_s%x_bg_a50_cha
	int m_nStage{};     // +0x224
	Vector4 m_vT{}, m_vR{}, m_vS{};   // cObject +0x20 / +0x30 / +0x40 (the node's SRT, copied to every new model)
	bool m_bDemoDone{};
	bool m_bGoalDone{true};
	int m_nDemoStep{};
	float m_fDemoMax{};   // cAnmCheck +8
	float m_fDemoLast{};
};

// DOL kingdom::game::g01::cTaskCourse (0x2f0 bytes, vtable 0x80439e18): the course model (node "m01b_s%x_cou_")
class cTaskCourse : public cObject, public kingdom::cTaskModel
{
public:
	cTaskCourse() = default;
	void Setup(const wii::Mdl0Node& node, int nStage);   // DOL 0x800dfd4c
	bool Execute(cJetTask* pParam) override;
};


// DOL kingdom::game::g01::cTaskStage (0x240 bytes, vtable 0x8043bcc8), created by cAppG01GalagaGame::Init, vtable +0x58 (slot 22) = Setup
class cTaskStage : public cObject, public kingdom::cTaskModel
{
public:
	cTaskStage();                              // DOL 0x800f9484 (Init)
	void Setup(int nStage);                    // DOL 0x800fa80c
	bool Execute(cJetTask* pParam) override;   // DOL 0x800fad60
	void SetState(int n) { m_nState = n; m_nStep = 0; }   // the app writes +0 / +4 directly (0 demo, 1 game)

private:
	void SetupEffect(const wii::Mdl0Node& node);   // one node of 0x800f94ec
	void SetupGimmick(const wii::Mdl0& skel, int nStage);   // DOL 0x800f9e8c
	void SetupMComet(const wii::Mdl0& skel);                // DOL 0x800f97a4
	void SetupSpikeMaster(const wii::Mdl0& skel, int nStage);   // DOL 0x800fa578
	void SetupBreakComet(int nStage);                       // DOL 0x800f99fc
	void SetupGate(const wii::Mdl0& skel, int nStage);      // DOL 0x800fa0a4
	void SetupSpike(const wii::Mdl0& skel, int nStage);     // DOL 0x800fa2fc
	int m_nStage{};
	std::vector<kingdom::cKingdomPtkHnd*> m_Effect;   // natively kept (the DOL lets its stack handles go; the emitters live on)
};
}
