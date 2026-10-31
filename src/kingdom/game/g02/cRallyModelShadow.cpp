#include "kingdom/game/g02/cRallyModelShadow.hpp"

#include <cmath>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"

namespace kingdom::game::g02
{

namespace
{
void Mul(float out[4][4], const float a[4][4], const float b[4][4])
{
	float t[4][4];
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++) t[r][c] = a[r][0] * b[0][c] + a[r][1] * b[1][c] + a[r][2] * b[2][c] + a[r][3] * b[3][c];
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++) out[r][c] = t[r][c];
}

// MTX44RotRad (FUN_80298d00): rotation about 'X' / 'Y' / 'Z', column vectors
void RotRad(float m[4][4], char axis, float rad)
{
	const float s = std::sin(rad), c = std::cos(rad);
	for (int r = 0; r < 4; r++)
		for (int k = 0; k < 4; k++) m[r][k] = r == k ? 1.0f : 0.0f;
	if (axis == 'X') { m[1][1] = c; m[1][2] = -s; m[2][1] = s; m[2][2] = c; }
	else if (axis == 'Y') { m[0][0] = c; m[0][2] = s; m[2][0] = -s; m[2][2] = c; }
	else { m[0][0] = c; m[0][1] = -s; m[1][0] = s; m[1][1] = c; }
}

void Normalize(float out[3], const float v[3])
{
	const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	for (int i = 0; i < 3; i++) out[i] = l > 0.0f ? v[i] / l : 0.0f;
}

// FUN_800c5ce0: the shadow colour clip by world (table 0x80371188: WORLD1, WORLD1, WORLD2, WORLD3, WORLD4); multiplayer by stage in world
const char* ShadowClip()
{
	static const char* const CLIP[5] = {"SHADOW_COLOR_WORLD1", "SHADOW_COLOR_WORLD1", "SHADOW_COLOR_WORLD2", "SHADOW_COLOR_WORLD3",
	                                    "SHADOW_COLOR_WORLD4"};
	const cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	int i = 0;
	if (sys != nullptr) i = sys->IsMulti() ? (sys->nStageIdx() == 0 ? 0 : (sys->nStageIdx() - 1) % 6 + 1) : sys->nWorld();
	return i >= 0 && i < 5 ? CLIP[i] : CLIP[0];   // (the DOL reads past the table for 5 / 6)
}
}

// DOL 0x8008120c [audited] (the shadow part): new 0x2b0 (cTaskPBModel ctor 0x8009a730), CAR_PAC_SH for colours 1..6 (0x8001f72c), the world's colour
// clip in slot 0 (0x8001a4d8(m, 0, crc, 0, 1)), cParamShadow, light = param +0x20..+0x2c; the car connects it as its child
cRallyModelShadow::cRallyModelShadow(cTaskUnit* pOwner) : m_pOwner(pOwner)
{
	SetName("g02::cRallyModelShadow");
	const int colour = pOwner != nullptr ? pOwner->nColour() : 0;
	if (colour >= 1 && colour <= 6) Model->RegistModel("CAR_PAC_SH", 0x36f);   // 0xf3bd8e11 (both branches of the DOL)
	Model->RegistAnm(0u, ShadowClip());
	m_Param.Load();
	for (int i = 0; i < 4; i++) m_vLight[i] = m_Param.vLight[i];   // memcpy(+0x2a0, param +0x20, 0x10)
}

// DOL 0x80088718 [audited], mode 0 (the only one used): d = N.L, f = -1 / d (FLOAT_8047aa8c); row r = f * (N_c L_r - d [r == c]), column 3 = -f L_r,
// row 3 = (0, 0, 0, -f d)
void cRallyModelShadow::ShadowMatrix(float m[4][4], const float light[3], const float normal[3])
{
	float l[3], n[3];
	Normalize(l, light);
	Normalize(n, normal);
	const float d = n[0] * l[0] + n[1] * l[1] + n[2] * l[2];
	const float f = d != 0.0f ? -1.0f / d : 0.0f;
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 3; c++) m[r][c] = f * (n[c] * l[r] - (r == c ? d : 0.0f));
		m[r][3] = f * -l[r];
	}
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[3][3] = f * -d;
}

// DOL 0x8007c838 [audited]
bool cRallyModelShadow::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (m_pOwner == nullptr || Model == nullptr) return true;
	kingdom::cKingdomModel* car = m_pOwner->pModel(0);
	if (car == nullptr) return true;
	Model->UpdateFrame();   // UNVERIFIED: the colour clip advances (the DOL's function has no frame call; g3d advances it in the scene)
	Vector4 p = car->vTrans();
	p.y += m_Param.fPosY;                                    // +0x298 -> +0x30
	const float n[3] = {0.0f, 1.0f, 0.0f};                   // the plane (0, 1, 0, 1) on the stack (FLOAT_8047aa28 / 8047aa2c)
	float m[4][4];
	ShadowMatrix(m, m_vLight, n);
	m[1][1] = m_Param.fScaleY;                               // +0x34
	m[0][3] = p.x; m[1][3] = p.y; m[2][3] = p.z; m[3][3] = p.w;   // TBox2::set mislabel: column 3 = the position
	const Vector4 r = car->vRotate();
	float rot[4][4];
	if (r.z != 0.0f) { RotRad(rot, 'Z', r.z); Mul(m, m, rot); }
	if (r.y != 0.0f) { RotRad(rot, 'Y', r.y); Mul(m, m, rot); }
	if (r.x != 0.0f) { RotRad(rot, 'X', r.x); Mul(m, m, rot); }
	m[3][0] = m[3][1] = m[3][2] = 0.0f; m[3][3] = 1.0f;   // 0x8013e820: 4x4 -> 3x4 (ScnObj::SetMtx of every LOD)
	float cm[16];
	for (int c = 0; c < 4; c++)
		for (int rr = 0; rr < 4; rr++) cm[c * 4 + rr] = m[rr][c];
	Model->SetNativeMtxRaw(cm);
	// +0x20..+0x28 copied into every LOD's ScnMdl +0xdc..+0xe4 (the model scale): natively the raw matrix already carries it (UNVERIFIED)
	return true;
}

}
