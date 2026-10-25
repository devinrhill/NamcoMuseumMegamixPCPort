#pragma once

#include <vector>

#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include "kingdom/game/g01/cObject.hpp"

namespace wii { class Mdl0; }

// Galaga Remix (g01) start area: kingdom::game::g01::cTaskStart (0x240 bytes, vtable 0x8043bd78; cTaskStage +0x224, made by 0x800faf04 and set
// up by 0x800fb0e4). One model slot per start gimmick m01g_s%x_s%02d (placed at the first skeleton node m01g_s%x_s%02d_), drawn with the a01
// clips; in stage 1 the opening demo plays the a10 clips first (cDemoStartW1S1 0x801c0afc, registered with cDemoMan): the elevator rises with
// PTK_ELV01 and m01se_16elevator / m01se_17elevator2 at the frames of Demo_s1.csv.
namespace kingdom::game::g01
{
class cTaskStart : public cObject, public kingdom::cTaskModel, public cDemoInterface
{
public:
	cTaskStart();                                            // DOL 0x800faf04
	~cTaskStart() override;
	void Setup(int nStage, const wii::Mdl0& skel);           // DOL 0x800fb0e4 (+ 0x800fb188 the placements)
	bool Execute(cJetTask* pParam) override;                 // DOL 0x800fbeec
	void DemoStart() override { m_nState = 0; m_nStep = 0; m_nDemoState = 0; m_bDemoDone = false; }   // DOL 0x800fbf78
	bool IsDemoDone() const override { return m_bDemoDone; }
	void DemoFinish() override;                              // cDemoStartW1S1::Term 0x801c0f20
	// the goal start 0x800fbf90 (state 1): cDemoStartW1S1 has no goal part (the base Start / Execute 0x800c9618 / 0 = done): the default

private:
	struct Place { Vector4 t, r, s; int nIndex; };           // 0x40 bytes each (+0x228 vector)
	void LoadModels();                                       // DOL 0x800fb97c
	void LoadGame();                                         // DOL 0x800fbcb8: the a01 clips
	void LoadDemo();                                         // DOL 0x800fbab0: the a10 clips, the longest one checked (cAnmCheck)
	void DemoInit();                                         // cDemoStartW1S1::Init 0x801c0e3c
	bool DemoCheck();                                        // cDemoStartW1S1::IsWaitEnd 0x801c0fac
	int m_nStage{};                                          // +0x224
	std::vector<Place> m_Place;                              // +0x228
	// the demo object (+0x21c)
	int m_nDemoState{};
	bool m_bDemoDone{};
	int m_nChkModel{-1}, m_nChkKind{};                       // cAnmCheck: model, clip kind (0 cha 1 cla 2 tsa 3 tpa 4 via)
	float m_fChkMax{}, m_fChkLast{};
	Vector4 m_vEffect{};                                     // +0x30: node m01e_22_elv01_001 of m01e_s1_d_s
	float m_fEffectTime{-1.0f}, m_fSeTime{-1.0f}, m_fSeEndTime{-1.0f};   // +0x40 / +0x44 / +0x48 (Demo_s1.csv; -1 = played)
	kingdom::cKingdomPtkHnd m_Effect;                        // +0x50
	int m_nSe{-1}, m_nSeEnd{-1};                             // +0xa0 / +0xac sound handles
};
}
