#include "kingdom/cTaskLensFlare.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "gfx/Gfx.hpp"
#include "gfx/Gfx3D.hpp"
#include "gfx/TextureCache.hpp"
#include "jet/JetUtil.hpp"
#include "kingdom/NativeScene.hpp"
#include "wii/Archive.hpp"

namespace kingdom
{

namespace
{
constexpr float PI = 3.1415927f;   // FLOAT_8047ac00
}

// DOL 0x80096d40 + Init 0x80096e58: draw priority 0x7ffffffd (after the 3D scene, before the HUD), light (0, 0, 0), size 0
cTaskLensFlare::cTaskLensFlare()
{
	SetName("cTaskLensFlare");
	nDrawPriority(0x7ffffffdu);
}

// DOL 0x80097f54: the anita file (natively by name; the DOL finds it by its CRC 0x08a27694 = ANT_LENSFLARE) and, for every texture of it,
// "<name>.brres" (s__s_brres_8036d6b7) from the attached archives (0x80096fb0: a texture object, CI ones with their palette)
void cTaskLensFlare::Load(const std::string& anitaName)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(anitaName, size);
	if (d == nullptr) { std::fprintf(stderr, "cTaskLensFlare: %s not found\n", anitaName.c_str()); return; }
	m_Anp.assign(d, d + size);
	jet::JetUtil::anpToHostEndian(m_Anp);
	const int n = jet::JetUtil::anpTexCount(m_Anp.data());
	m_Tex.assign((std::size_t)(n > 0 ? n : 0), Tex{});
	for (int i = n - 1; i >= 0; i--)
	{
		Texture* t = gfx::FindTexture(jet::JetUtil::anpTexTexName(m_Anp.data(), i));
		if (t != nullptr) m_Tex[(std::size_t)i] = Tex{t->gfxId, t->width, t->height};
	}
}

// DOL 0x8009805c
void cTaskLensFlare::Draw(int nStatus, cJetTask* pParam)
{
	(void)nStatus;
	(void)pParam;
	if (m_Anp.empty()) return;
	float view[16], proj[16];
	if (!NativeScene::HasCamera() || !NativeScene::GetMatrices(view, proj)) return;
	// the light on the screen (0x80140538 with the camera's viewport: pixels, origin top-left); natively the game's 640 x 480
	const float W = (float)gfx::SCREEN_W, H = (float)gfx::SCREEN_H;
	float clip[4];
	for (int r = 0; r < 4; r++)
		clip[r] = proj[r] * (view[0] * m_vLight[0] + view[4] * m_vLight[1] + view[8] * m_vLight[2] + view[12])
		        + proj[4 + r] * (view[1] * m_vLight[0] + view[5] * m_vLight[1] + view[9] * m_vLight[2] + view[13])
		        + proj[8 + r] * (view[2] * m_vLight[0] + view[6] * m_vLight[1] + view[10] * m_vLight[2] + view[14])
		        + proj[12 + r] * (view[3] * m_vLight[0] + view[7] * m_vLight[1] + view[11] * m_vLight[2] + view[15]);
	if (clip[3] == 0.0f) return;
	const float lx = (clip[0] / clip[3] * 0.5f + 0.5f) * W, ly = (0.5f - clip[1] / clip[3] * 0.5f) * H;
	const float hw = W * 0.5f, hh = H * 0.5f;   // +0x390 / +0x394 (FLOAT_8047abd8)
	const float dx = lx - hw, dy = ly - hh;
	const float dist = std::sqrt(dx * dx + dy * dy), diag = std::sqrt(hw * hw + hh * hh);
	const float t = dist < diag ? (diag - dist) / diag : 0.0f;   // +0x310
	const float dir = std::atan2(-dx, -dy);                       // +0x374 (Atan2FIdx * FLOAT_8047abdc = radians)
	// 0x80097234: the visibility, averaged over the last 5 frames (FLOAT_8047abec); behind the camera nothing is visible
	float vis = clip[3] > 0.0f ? gfx::VisibleShare(view, proj, m_vLight, 0.5f * m_fSize) : 0.0f;
	if (vis < 0.0f) vis = 1.0f;   // native: no occlusion queries -> always visible
	if (vis > 1.0f) vis = 1.0f;
	for (int i = 4; i > 0; i--) m_fHistory[i] = m_fHistory[i - 1];
	m_fHistory[0] = vis;
	float avg = 0.0f;
	for (float h : m_fHistory) avg += h;
	avg /= 5.0f;   // +0x314
	if (std::getenv("MEGAMIX_LENSDBG")) std::fprintf(stderr, "lens flare: light (%.0f %.0f) t %.2f vis %.2f avg %.2f\n", lx, ly, t, vis, avg);
	void* anp = m_Anp.data();
	if (jet::JetUtil::anpPatCount(anp) < 2) return;
	const int n = jet::JetUtil::anpPatLayCount(anp, 0);   // 0x80142d34(anp, 0)
	for (int i = n - 1; i >= 0; i--)
	{
		const jet::sDatLayP& a = jet::JetUtil::anpPatLayPtr(anp, 0, i)->lay;   // 0x80142d54(anp, 0, i): at the edge
		const jet::sDatLayP& b = jet::JetUtil::anpPatLayPtr(anp, 1, i)->lay;   // (anp, 1, i): at the centre
		auto mix = [t](float x, float y) { return x != y ? t * (y - x) + x : x; };
		const float isx = mix(a.isx, b.isx), isy = mix(a.isy, b.isy);
		const float cr = mix((float)a.r, (float)b.r), cg = mix((float)a.g, (float)b.g), cb = mix((float)a.b, (float)b.b), ca = mix((float)a.a, (float)b.a);
		const int alpha = (int)((float)(int)ca * avg);
		if (std::getenv("MEGAMIX_LENSDBG")) std::fprintf(stderr, "  layer %d tex %d (gfx %d) px %.1f half (%.1f %.1f) is (%.2f %.2f) rgba %d %d %d %d -> a %d\n", i, a.tex, a.tex >= 0 && a.tex < (int)m_Tex.size() ? m_Tex[(std::size_t)a.tex].gfxId : -1, a.px, a.suh, a.svh, isx, isy, (int)cr, (int)cg, (int)cb, (int)ca, alpha);
		if ((alpha & 0xff) == 0) continue;
		// 0x80097710: shown layers with a texture index (-1 = none), additive (GXSetBlendMode 1, SRCALPHA, ONE)
		if (a.tex < 0 || a.tex >= (int)m_Tex.size()) continue;
		const Tex& tx = m_Tex[(std::size_t)a.tex];
		float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
		if (tx.gfxId != 0 && tx.width > 0 && tx.height > 0)
		{
			u0 = a.u0 / (float)tx.width; v0 = a.v0 / (float)tx.height; u1 = a.u1 / (float)tx.width; v1 = a.v1 / (float)tx.height;
		}
		// along the line from the centre through the light: s = -px / half width (screen space, origin in the middle, y up)
		const float s = -a.px / hw;
		const float x = s * dx, y = -s * dy;
		const float rot = PI * -(a.orot / 180.0f) - dir;   // FLOAT_8047ac00 * -(orot / FLOAT_8047ac04) - +0x374
		const float c = std::cos(rot), sn = std::sin(rot);
		const float qx[4] = {-a.suh, -a.suh, a.suh, a.suh}, qy[4] = {a.svh, -a.svh, a.svh, -a.svh};   // TL, BL, TR, BR
		float xy[8];
		for (int k = 0; k < 4; k++)
		{
			const float px = qx[k] * isx, py = qy[k] * isy;
			xy[k * 2] = x + px * c - py * sn;
			xy[k * 2 + 1] = y + px * sn + py * c;
		}
		const float uv[8] = {u0, v0, u0, v1, u1, v0, u1, v1};
		gfx::DrawQuad(tx.gfxId, xy, uv, (std::uint8_t)cr, (std::uint8_t)cg, (std::uint8_t)cb, (std::uint8_t)(alpha & 0xff), gfx::BLEND_ADD, true);
	}
}

}
