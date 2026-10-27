#include "kingdom/game/g04/cTaskEnemyShadow.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"

#include <cmath>

#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g04/cTaskEnemy.hpp"

namespace kingdom::game::g04
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

// MTX44RotRad (0x80298d00): rotation about axis 'X' / 'Y' / 'Z' by rad, column vectors
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
}

// DOL 0x8009a730 (cTaskPBModel ctor) + 0x801b0100 (the model part's Init, vtable +0x50 slot +8)
cTaskEnemyShadow::cTaskEnemyShadow(cTaskEnemy* pOwner, uint nModel) : m_pOwner(pOwner), m_nModel(nModel)
{
	SetName("g04::cTaskEnemyShadow");
	Model->RegistModel("BIN_MODEL_WANI_SH", 0x36f);   // 0x5aabdfba (0x8001f72c)
	Model->RegistAnm(0u, "WANI_SH_CLA");               // 0xe059b96b (0x8001a4d8(m, 0, crc, 0, 1))
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_LIGHT"))                   // 0x3d0ed0bc (cParamLight): row 0 columns 1..3 -> +0x20.., row 1 -> +0x30..
		for (int i = 0; i < 3; i++) m_vLight[i] = csv.F32(0, i + 1, 0.0f);
	m_vLight[3] = 1.0f;                                // +0x2ac = FLOAT_8047ccd8
}

// DOL 0x801b0550, mode 0 (the only one used): d = N.L, f = -1 / d (FLOAT_8047cce8); row r = f * (N_c L_r - d [r == c]), column 3 = -f L_r,
// row 3 = (0, 0, 0, -f d)
void cTaskEnemyShadow::ShadowMatrix(float m[4][4], const float light[3], const float normal[3])
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

// DOL 0x801b0348
void cTaskEnemyShadow::ClipA() { Model->RegistAnm(0u, "WANI_SH_A_CLA"); }   // 0x13924592 (0x8001a4d8(m, 0, crc, 0, 1))

// DOL 0x801b0360
void cTaskEnemyShadow::ClipNormal() { Model->RegistAnm(0u, "WANI_SH_CLA"); }   // 0xe059b96b

// DOL 0x801b0378
void cTaskEnemyShadow::SetClrRate(float rate) { Model->SetUpdateRateMatClr(0u, rate); }   // 0x8001cd4c

// DOL 0x801b0380
bool cTaskEnemyShadow::Execute(cJetTask* pParam)
{
	if (m_pOwner == nullptr || Model == nullptr) return true;
	// the colour clip advances in the app's ScnRoot::UpdateFrame (0x8009312c, the tail of 0x8009219c, after its pause / HOME return); the
	// shadow's own Execute has no frame call and no pause test
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app == nullptr || !(app->IsPause() || app->IsHome())) Model->UpdateFrame();
	kingdom::cKingdomModel* g = m_pOwner->pRedSelModel();   // +0x2b0 -> pModel(+0x240)
	Vector4 p = g->vTrans();
	p.y += -2.5f;                                            // FLOAT_8047cce0
	const float n[3] = {0.0f, 1.0f, 0.0f};                   // the plane (0, 1, 0, 1) on the stack
	float m[4][4];
	ShadowMatrix(m, m_vLight, n);
	m[1][1] = 0.5f;                                          // FLOAT_8047cce4
	m[0][3] = p.x; m[1][3] = p.y; m[2][3] = p.z; m[3][3] = p.w;   // 0x8013ecd4: column 3
	const Vector4 r = g->vRotate();
	float rot[4][4];
	if (r.z != 0.0f) { RotRad(rot, 'Z', r.z); Mul(m, m, rot); }
	if (r.y != 0.0f) { RotRad(rot, 'Y', r.y); Mul(m, m, rot); }
	if (r.x != 0.0f) { RotRad(rot, 'X', r.x); Mul(m, m, rot); }
	// The matrix already holds the gator's world placement: the PushBack onto node "wani" (0x8001f798 -> ScnMdlExpand::PushBack) fails in
	// g3d because a push-back model already has the scene root as parent (ScnGroup::Insert refuses an object with a parent), so the
	// shadow's world matrix is this local one (UNVERIFIED in memory; with the node as parent the shadow lands at twice the gator's position).
	m[3][0] = m[3][1] = m[3][2] = 0.0f; m[3][3] = 1.0f;   // 0x8013e820: 4x4 -> 3x4
	float cm[16];
	for (int c = 0; c < 4; c++)
		for (int rr = 0; rr < 4; rr++) cm[c * 4 + rr] = m[rr][c];
	Model->SetNativeMtxRaw(cm);
	return true;
}

// DOL 0x8009aad0 (kingdom::cTaskPBModel::Draw, the task vtable's thunk 0x80088c10): the model's slot +0x18 (= 0x801b0380, run as Execute
// natively), then when visible (+0x44) the push-back's +0x54 = this frame's submission. The port's engine cTaskPBModel::Draw is empty.
void cTaskEnemyShadow::Draw(int, cJetTask*)
{
	// The ctor pushes this shadow back onto node "wani" of gator model m_nModel (0x8001f798 -> ScnMdlExpand::PushBack): it is drawn
	// with that model only. Without this gate the other model's shadow (constant WANI_SH_CLA, alpha 100 = visible) always showed.
	kingdom::cKingdomModel* g = m_pOwner != nullptr ? m_pOwner->pModel(m_nModel) : nullptr;
	if (Model != nullptr && Model->IsVisible() && g != nullptr && g->IsVisible()) Model->UpdateMatrix();
}

}
