#pragma once

#include <string>
#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"

// kingdom::cTaskLensFlare (0x3a0 bytes, vtable 0x80394608, a cTaskPushBack; ctor 0x80096d40, Init 0x80096e58: draw priority 0x7ffffffd, a
// 4 KB depth read buffer). The flare of a light (Galaga's stages: cTaskStage +0x230, the light at node light_001 of m01e_s%x_lens_s.brres).
//   Load 0x80097f54: an anita file (ANT_LENSFLARE) whose textures are loaded as "<name>.brres" (0x80096fb0) and whose patterns 0 and 1 hold
//     every sprite twice: as it looks with the light at the screen's edge and at its centre.
//   SetLight 0x80098038 (+0x2f0), SetSize 0x80098030 (+0x378: the square that is tested for visibility).
//   Draw 0x8009805c (vtable +0x38): the light on the screen (+0x300, 0x80140538), t = how close it is to the centre (+0x310: 1 at the centre,
//     0 at half the screen diagonal and beyond), its direction from the centre (+0x374); the visibility (0x80097234: a camera-facing square of
//     the size at the light, the depth buffer read back around it (0x800979d4 / 0x80097d60), the share of its pixels not hidden, averaged over
//     the last 5 frames of the camera, +0x314). Then each layer of pattern 0 from the last: size / colour between pattern 0 and 1 by t, alpha *
//     visibility; at s = -px / half width along the line from the centre through the light; turned by -orot and the light's direction; additive.
// Natively the visibility comes from an occlusion query (gfx::VisibleShare) and the sprites from gfx::DrawQuad.
namespace kingdom
{
class cTaskLensFlare : public cTaskKingdom
{
public:
	cTaskLensFlare();                                          // DOL 0x80096d40 + 0x80096e58
	void Load(const std::string& anitaName);                   // DOL 0x80097f54
	void SetLight(const float pos[3]) { for (int i = 0; i < 3; i++) m_vLight[i] = pos[i]; }   // DOL 0x80098038
	void SetSize(float f) { m_fSize = f; }                     // DOL 0x80098030
	void Draw(int nStatus, cJetTask* pParam) override;         // DOL 0x8009805c

private:
	std::vector<byte> m_Anp;                                   // +0x384 the anita data (host order)
	struct Tex { int gfxId{}, width{}, height{}; };
	std::vector<Tex> m_Tex;                                    // +0x1c0 (0x3c each): the textures by anita texture index
	float m_vLight[3]{};                                       // +0x2f0
	float m_fSize{};                                           // +0x378
	float m_fHistory[5]{};                                     // +0x318.. the last visibilities (one camera natively)
};
}
