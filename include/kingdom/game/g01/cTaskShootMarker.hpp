#pragma once

#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g01/cObject.hpp"

// Galaga Remix (g01) the "SHOOT!" sign over a target: kingdom::game::g01::cTaskShootMarker (0x260 bytes, vtable 0x8043bb18; cObject at +0,
// cTaskAnita at +0x50). One ANT_SCORE anita (anims 0x3d appear, 0x3e loop, 0x3f out), priority 0x80000001. Every frame its owner's
// position (+0x20) is projected to layout coordinates; off screen it is hidden. A child task of its owner: missiles / boss missiles (+0x368),
// gates / spikes (+0x358, offset (0, 10, 0)), the boss's shells (+0x2b0). The owner calls Show() / Hide().
namespace kingdom::game::g01
{
class cTaskShootMarker : public cObject, public kingdom::cTaskAnita
{
public:
	// DOL 0x800f84d4(this, owner, offset, rot, scale): null = (0, 0, 0, 1) / 0 / ONE
	cTaskShootMarker(const cObject* pOwner, const Vector4* pOffset = nullptr, const Vector4* pRot = nullptr, const Vector4* pScale = nullptr);
	bool Execute(cJetTask* pParam) override;     // DOL 0x800f8c08
	void Draw(int nStatus, cJetTask* pParam) override;   // DOL 0x800f8e6c
	void Show() { m_b08 = true; m_nState = 0; m_nStep = 0; }   // the owners' switch, e.g. 0x800f0358 / 0x800ca664
	void Hide() { m_nState = 2; m_nStep = 0; }                 // e.g. 0x800f039c / 0x800ca690

private:
	bool Project();                              // DOL 0x800f88a4
	const cObject* m_pOwner{};                   // +0x250
	Vector4 m_vOffset{0.0f, 0.0f, 0.0f, 1.0f};   // +0x220
	Vector4 m_vRot{0.0f, 0.0f, 0.0f, 0.0f};      // +0x230
	Vector4 m_vScale{1.0f, 1.0f, 1.0f, 1.0f};    // +0x240
};
}
