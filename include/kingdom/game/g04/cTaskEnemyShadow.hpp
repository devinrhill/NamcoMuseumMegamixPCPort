#pragma once

#include "kingdom/cTaskPBModel.hpp"

// Gator Panic Remix (g04): kingdom::game::g04::cTaskEnemyShadow (0x2c0 bytes, vtable 0x8044a300, a cTaskPBModel). Gator A (place 0) builds
// one per model (normal / red) in cTaskEnemy::Init 0x801a9594: ctor 0x8009a730, +0x2b0 = the gator, connected to the gator, hung on node "wani"
// of that model (0x8001f798 = ScnMdlExpand::PushBack(shadow, "wani"), which g3d refuses: see Execute). Every frame it is squashed onto the
// ground plane along the CSV_LIGHT direction at the gator's position: a planar projected shadow.
namespace kingdom::game::g04
{
class cTaskEnemy;

class cTaskEnemyShadow : public kingdom::cTaskPBModel
{
public:
	cTaskEnemyShadow(cTaskEnemy* pOwner, uint nModel);   // DOL 0x8009a730 + 0x801b0100 (BIN_MODEL_WANI_SH, WANI_SH_CLA, CSV_LIGHT)
	// ~cTaskEnemyShadow DOL 0x801b076c: calls vtable +0xc (an empty blr) and then ~cTaskPBModel; the implicit destructor is the same
	bool Execute(cJetTask* pParam) override;             // DOL 0x801b0380
	void ClipA();                                        // DOL 0x801b0348: WANI_SH_A_CLA in CLR slot 0 (the gator sinks back)
	void ClipNormal();                                   // DOL 0x801b0360: WANI_SH_CLA
	void SetClrRate(float rate);                         // DOL 0x801b0378: SetUpdateRateMatClr(0, rate)

	// DOL 0x801b0550 (mode 0): the matrix projecting along the light L onto the plane through the origin with normal N (4x4, row major)
	static void ShadowMatrix(float m[4][4], const float light[3], const float normal[3]);

private:
	cTaskEnemy* m_pOwner{};    // +0x2b0
	uint m_nModel{};           // the owner's model the shadow hangs on (node "wani")
	float m_vLight[4]{};       // +0x2a0 (CSV_LIGHT row 0, columns 1..3; w +0x2ac = 1)
};
}
