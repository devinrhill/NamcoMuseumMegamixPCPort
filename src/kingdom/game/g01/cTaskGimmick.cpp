#include "kingdom/game/g01/cTaskGimmick.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"
#include "kingdom/game/g01/cTaskEffect.hpp"
#include "snd/Mixer.hpp"
#include "wii/Archive.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g01
{

// cTaskGimmick's Term 0x800eca00 releases the sound actor (+0x328; natively none: no cue plays on it, the hits use FUN_801c15ac).

namespace
{
constexpr float DEG2RAD = 0.017453292f;   // FLOAT_8047b5fc
constexpr float ANM_SLOW = 0.1f;          // FLOAT_8047b5f4
constexpr float ANM_ONE = 1.0f;           // FLOAT_8047b5f0

// DOL 0x803759ac (node name parts) with the model CRCs 0x80373cf8 and the clip CRCs 0x80373e08 {cha, cla, tpa, tsa} resolved to their names
struct Kind { const char* node; const char* model; const char* clip[4]; };
const Kind KIND[68] = {
	{"m01g_l_co01_", "GIM_L_COMET01_MDL", {"GIM_L_COMET01_ANM_CHA_01", "GIM_L_COMET01_ANM_CLA_01", nullptr, nullptr}},
	{"m01g_l_co02_", "GIM_L_COMET02_MDL", {}},
	{"m01g_l_co03_", "GIM_L_COMET03_MDL", {"GIM_L_COMET03_ANM_CHA_01", "GIM_L_COMET03_ANM_CLA_01", nullptr, nullptr}},
	{"m01g_l_co04_", "GIM_L_COMET04_MDL", {"GIM_L_COMET04_ANM_CHA_01", nullptr, nullptr, "GIM_L_COMET04_ANM_TSA_01"}},
	{"m01g_l_co05_", "GIM_L_COMET05_MDL", {}},
	{"m01g_l_co08_", "GIM_L_COMET08_MDL", {}},
	{"m01g_l_co09_", "GIM_L_COMET09_MDL", {}},
	{"m01g_l_co10_", "GIM_L_COMET10_MDL", {nullptr, nullptr, nullptr, "GIM_L_COMET10_ANM_TSA_01"}},
	{"m01g_l_co11_", "GIM_L_COMET11_MDL", {}},
	{"m01g_l_co12_", "GIM_L_COMET12_MDL", {"GIM_L_COMET12_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co13_", "GIM_L_COMET13_MDL", {}},
	{"m01g_l_co14_", "GIM_L_COMET14_MDL", {"GIM_L_COMET14_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co15_", "GIM_L_COMET15_MDL", {}},
	{"m01g_l_co17_", "GIM_L_COMET17_MDL", {"GIM_L_COMET17_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co18_", "GIM_L_COMET18_MDL", {}},
	{"m01g_l_co19_", "GIM_L_COMET19_MDL", {"GIM_L_COMET19_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co20_", "GIM_L_COMET20_MDL", {"GIM_L_COMET20_ANM_CHA_01", nullptr, nullptr, "GIM_L_COMET20_ANM_TSA_01"}},
	{"m01g_l_co21_", "GIM_L_COMET21_MDL", {"GIM_L_COMET21_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co22_", "GIM_L_COMET22_MDL", {}},
	{"m01g_l_co23_", "GIM_L_COMET23_MDL", {}},
	{"m01g_l_co24_", "GIM_L_COMET24_MDL", {}},
	{"m01g_l_co25_", "GIM_L_COMET25_MDL", {}},
	{"m01g_l_co26_", "GIM_L_COMET26_MDL", {"GIM_L_COMET26_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co27_", "GIM_L_COMET27_MDL", {"GIM_L_COMET27_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_shadow01_", "GIM_SHADOW01_MDL", {}},
	{"m01g_cliff01_", "GIM_CLIFF01_MDL", {}},
	{"m01g_cliff02_", "GIM_CLIFF02_MDL", {}},
	{"m01g_cliff03_", "GIM_CLIFF03_MDL", {}},
	{"m01g_cliff04_", "GIM_CLIFF04_MDL", {}},
	{"m01g_cliff05_", "GIM_CLIFF05_MDL", {}},
	{"m01g_cliff06_", "GIM_CLIFF06_MDL", {}},
	{"m01g_fog_tree01_", "GIM_FOG_TREE01_MDL", {}},
	{"m01g_fog_tree02_", "GIM_FOG_TREE02_MDL", {}},
	{"m01g_fog_tree03_", "GIM_FOG_TREE03_MDL", {}},
	{"m01g_leaf03_", "GIM_LEAF03_MDL", {"GIM_LEAF03_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_leaf04_", "GIM_LEAF04_MDL", {"GIM_LEAF04_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_rock01_", "GIM_ROCK01_MDL", {}},
	{"m01g_rock02_", "GIM_ROCK02_MDL", {}},
	{"m01g_sun01_", "GIM_SUN01_MDL", {"GIM_SUN01_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_web01_", "GIM_WEB01_MDL", {"GIM_WEB01_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_wood04_", "GIM_WOOD04_MDL", {}},
	{"m01g_wood05_", "GIM_WOOD05_MDL", {}},
	{"m01g_wood06_", "GIM_WOOD06_MDL", {}},
	{"m01g_comet03_", "GIM_COMET03_MDL", {}},
	{"m01g_comet05_", "GIM_COMET05_MDL", {}},
	{"m01g_comet08_", "GIM_COMET08_MDL", {}},
	{"m01g_comet09_", "GIM_COMET09_MDL", {}},
	{"m01g_comet10_", "GIM_COMET10_MDL", {}},
	{"m01g_comet15_", "GIM_COMET15_MDL", {}},
	{"m01g_comet19_", "GIM_COMET19_MDL", {}},
	{"m01g_water01_", "GIM_WATER01_MDL", {nullptr, nullptr, nullptr, "GIM_WATER01_ANM_TSA_01"}},
	{"m01g_comb01_", "GIM_COMB01_MDL", {"GIM_COMB01_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_comb02_", "GIM_COMB02_MDL", {"GIM_COMB02_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_hana01_", "GIM_HANA01_MDL", {"GIM_HANA01_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_hana02_", "GIM_HANA02_MDL", {"GIM_HANA02_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_hana03_", "GIM_HANA03_MDL", {"GIM_HANA03_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_ivy01_", "GIM_IVY01_MDL", {"GIM_IVY01_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_ivy02_", "GIM_IVY02_MDL", {"GIM_IVY02_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_ivy03_", "GIM_IVY03_MDL", {"GIM_IVY03_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_ivy04_", "GIM_IVY04_MDL", {"GIM_IVY04_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_ivy05_", "GIM_IVY05_MDL", {"GIM_IVY05_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_ivy06_", "GIM_IVY06_MDL", {"GIM_IVY06_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_leaf01_", "GIM_LEAF01_MDL", {"GIM_LEAF01_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_leaf02_", "GIM_LEAF02_MDL", {"GIM_LEAF02_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_wood01_", "GIM_WOOD01_MDL", {}},
	{"m01g_wood02_", "GIM_WOOD02_MDL", {}},
	{"m01g_wood03_", "GIM_WOOD03_MDL", {}},
	{"m01g_woodleaf01_", "GIM_WOODLEAF01_MDL", {}},
};

// DOL 0x80375548 (node name parts) with the model CRCs 0x80374278 and the clip CRCs 0x803742a8 (bone clips only)
const Kind MCOMET[12] = {
	{"m01g_m_co01_", "GIM_M_COMET01_MDL", {"GIM_M_COMET01_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_m_co02_", "GIM_M_COMET02_MDL", {"GIM_M_COMET02_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_m_co03_", "GIM_M_COMET03_MDL", {"GIM_M_COMET03_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_m_co04_", "GIM_M_COMET04_MDL", {"GIM_M_COMET04_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_m_co05_", "GIM_M_COMET05_MDL", {"GIM_M_COMET05_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_m_co06_", "GIM_M_COMET06_MDL", {"GIM_M_COMET06_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_m_co07_", "GIM_M_COMET07_MDL", {"GIM_M_COMET07_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co06_", "GIM_L_COMET06_MDL", {"GIM_L_COMET06_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co07_", "GIM_L_COMET07_MDL", {"GIM_L_COMET07_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co16_", "GIM_L_COMET16_MDL", {"GIM_L_COMET16_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co28_", "GIM_L_COMET28_MDL", {"GIM_L_COMET28_ANM_CHA_01", nullptr, nullptr, nullptr}},
	{"m01g_l_co29_", "GIM_L_COMET29_MDL", {"GIM_L_COMET29_ANM_CHA_01", nullptr, nullptr, nullptr}},
};

bool Exists(const char* name)
{
	std::size_t size = 0;
	return name != nullptr && wii::ArchiveStore::Find(name, size) != nullptr;
}

// a random start frame: rand % (uint)(1000 * frameMax) / 1000 (FLOAT_8047b5f8)
float RandomFrame(float frameMax)
{
	const unsigned range = (unsigned)(1000.0f * frameMax);
	return range == 0 ? 0.0f : (float)((unsigned)std::rand() % range) / 1000.0f;
}
}

// DOL 0x800ec9a0: kind 9, priority 0x10002, the sound actor (+0x328, 0x801c1680: natively none, no cue plays on it), active
cTaskGimmick::cTaskGimmick()
{
	SetName("g01::cTaskGimmick");
	m_nKind = 9;
	nDrawPriority(0x10002u);
	m_b08 = true;
	m_b09 = true;
}

// DOL 0x800ed088 (every gimmick's draw, e.g. the comets' thunk 0x800ee018; cTaskGimmickMComet's 0x800edcbc is the same code): nothing once
// inactive (a shot comet sets +8 = 0)
void cTaskGimmick::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskModel::Draw(nStatus, pParam);   // FUN_8009aad0
}

int cTaskGimmick::Find(const char* nodeName)
{
	for (int i = 0; i < 68; i++)
		if (std::strstr(nodeName, KIND[i].node) != nullptr) return i;
	return -1;
}

int cTaskGimmickMComet::Find(const char* nodeName)
{
	for (int i = 0; i < 12; i++)
		if (std::strstr(nodeName, MCOMET[i].node) != nullptr) return i;
	return -1;
}

// the end of 0x800eca58 / 0x800ed128: translate +0x38, rotate +0x2c * DEG2RAD, scale +0x20 into the model and cObject +0x20 / +0x30 / +0x40
void cTaskGimmick::Place(const wii::Mdl0Node& node)
{
	cKingdomModel* m = pModel(0);
	m->vTrans() = Vector4(node.translate[0], node.translate[1], node.translate[2], 1.0f);   // w = DAT_80374254 (1.0)
	m->vRotate() = Vector4(node.rotate[0] * DEG2RAD, node.rotate[1] * DEG2RAD, node.rotate[2] * DEG2RAD, 1.0f);
	m->vScale() = Vector4(node.scale[0], node.scale[1], node.scale[2], 1.0f);
	m_v20 = m->vTrans();
	m_v30 = m->vRotate();
	m_v40 = m->vScale();
}

// DOL 0x800eca58: model and clips of the kind (light set 0, fog nFogIdx; 0x8001e16c), every clip at a random start frame
void cTaskGimmick::Setup(const wii::Mdl0Node& node, int nIndex, int nFogIdx)
{
	if (nIndex < 0 || nIndex >= 68) return;
	const Kind& k = KIND[nIndex];
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(k.model, 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx(nFogIdx);
	const float rate = KingdomGlobal::GalagaSys->nWorld() == 3 ? ANM_ONE : ANM_SLOW;
	if (Exists(k.clip[0]))
	{
		m->RegistAnm(0u, k.clip[0]);
		m->SetUpdateRateAnmChr(0u, rate);
		m->SetFrameAnmChr(0u, RandomFrame(m->GetFrameMaxAnmChr(0u)));
	}
	if (Exists(k.clip[1]))
	{
		m->RegistAnm(0u, k.clip[1]);
		m->SetUpdateRateMatClr(0u, rate);
		m->SetFrameMatClr(0u, RandomFrame(m->GetFrameMaxMatClr(0u)));
	}
	if (Exists(k.clip[2]))
	{
		m->RegistAnm(0u, k.clip[2]);
		m->SetUpdateRateAnmTexPat(0u, ANM_ONE);
		// the pattern clip's start frame: rand % (uint)frameMax (whole frames, 0x8001b9e4)
		const unsigned n = (unsigned)m->GetFrameMaxAnmTexPat(0u);
		m->SetFrameAnmTexPat(0u, n == 0 ? 0.0f : (float)((unsigned)std::rand() % n));
	}
	if (Exists(k.clip[3]))
	{
		m->RegistAnm(0u, k.clip[3]);
		m->SetUpdateRateAnmTexSrt(0u, rate);
		m->SetFrameAnmTexSrt(0u, RandomFrame(m->GetFrameMaxAnmTexSrt(0u)));
	}
	Place(node);
	// vtable +0xac (0x800ecff0): the sound actor follows +0x20 (natively none)
}

// DOL 0x800ed000: the clips advance while the object is active and the game is not paused
bool cTaskGimmick::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_b08 && m_b09) UpdateFrame();   // 0x8001ded4
	return cTaskModel::Execute(pParam);
}

// ---- cTaskGimmickMComet -------------------------------------------------------------------------------------------------------------

cTaskGimmickMComet::cTaskGimmickMComet()
{
	SetName("g01::cTaskGimmickMComet");
}

// DOL 0x800ed128: like the generic setup without the random frames (all rates 0.1, pattern 1.0); collidable (+0x14) with an AABB
void cTaskGimmickMComet::Setup(const wii::Mdl0Node& node, int nIndex, int nFogIdx)
{
	if (nIndex < 0 || nIndex >= 12) return;
	m_nIndex = nIndex;
	const Kind& k = MCOMET[nIndex];
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(k.model, 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx(nFogIdx);
	if (Exists(k.clip[0]))
	{
		m->RegistAnm(0u, k.clip[0]);
		m->SetUpdateRateAnmChr(0u, ANM_SLOW);
	}
	Place(node);
	m_b14 = true;
	// CalcAABB (vtable +0x14 = 0x800ed0b8: the g3d bounding volume of the model). Natively the box of the model's vertex positions,
	// moved by T R S each frame (0x801c1950; UNVERIFIED: the bounding volume may already be in world space)
	if (const wii::Mdl0* mdl = m->pMdl0())
	{
		float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
		for (const wii::Mdl0Mesh& mesh : mdl->meshes)
			for (const wii::Mdl0Vertex& v : mesh.verts)
				for (int i = 0; i < 3; i++) { mn[i] = std::min(mn[i], v.pos[i]); mx[i] = std::max(mx[i], v.pos[i]); }
		if (mn[0] <= mx[0]) SetLocalAABB(mn, mx);
	}
	UpdateWorldAABB();
}

// DOL 0x800ed5dc
bool cTaskGimmickMComet::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	UpdateWorldAABB();   // 0x801c1950
	return cTaskGimmick::Execute(pParam);
}

// DOL 0x800ed66c: a shot hit it: PTK_M_COMET01 (slot 0x1c) at the shot's hit point (beam +0x260), m01se_03hit1 (DAT_8047b5e8) on the
// player's sound handle
void cTaskGimmickMComet::HitCallback(cObject* other)
{
	const cTaskConsolePlayer* player = dynamic_cast<const cTaskConsolePlayer*>(other);
	if (player == nullptr) return;
	const float* p = player->vHitPos();
	(new cKingdomPtkHnd())->Generate((uint)cTaskEffect::eM_COMET01, true, false, true, 0.0f, Vector4(p[0], p[1], p[2], 1.0f));
	snd::Play("m01se_03hit1");
}

// ---- cTaskGimmickBreakComet ---------------------------------------------------------------------------------------------------------

namespace
{
// DOL 0x80374398: models of the types 0..2, then per type {cha, cla, tsa, tpa, via} (the colour clips are named CLAR_... on the disc)
const char* const BREAK_MODEL[3] = {"CHAR_ENEMY_COMET01_MDL", "CHAR_ENEMY_COMET02_MDL", "CHAR_ENEMY_COMET03_MDL"};
const char* const BREAK_CLIP[3][2] = {{"CHAR_ENEMY_COMET01_ANM_CHA_01", "CLAR_ENEMY_COMET01_ANM_CLA_01"},
                                      {"CHAR_ENEMY_COMET02_ANM_CHA_01", "CLAR_ENEMY_COMET02_ANM_CLA_01"},
                                      {"CHAR_ENEMY_COMET03_ANM_CHA_01", "CLAR_ENEMY_COMET03_ANM_CLA_01"}};
}

// DOL 0x800ed7a0 (vtable +0xa4 after the gimmick's): kind 10
cTaskGimmickBreakComet::cTaskGimmickBreakComet()
{
	SetName("g01::cTaskGimmickBreakComet");
	m_nKind = 10;
}

// DOL 0x800ed7ac: model of the type (light set 0, fog nFogIdx), bone clip at 0.1 * fRate, colour clip at 0.1; the given SRT; collidable,
// its AABB (vtable +0x14 = 0x801c1780 from the vertex box)
void cTaskGimmickBreakComet::Setup(int nType, float fRate, const Vector4& pos, const Vector4& rot, const Vector4& scale, int nScore, int nFogIdx)
{
	if (nType < 0 || nType >= 3) return;
	m_nType = nType;
	m_nScore = nScore;
	cKingdomModel* m = CreateModel(0);
	m->RegistModel(BREAK_MODEL[nType], 0x36f);
	m->SetLightSetIdx(0);
	m->SetFogIdx(nFogIdx);
	if (Exists(BREAK_CLIP[nType][0]))
	{
		m->RegistAnm(0u, BREAK_CLIP[nType][0]);
		m->SetUpdateRateAnmChr(0u, ANM_SLOW * fRate);
	}
	if (Exists(BREAK_CLIP[nType][1]))
	{
		m->RegistAnm(0u, BREAK_CLIP[nType][1]);
		m->SetUpdateRateMatClr(0u, ANM_SLOW);
	}
	m->vTrans() = pos;
	m->vRotate() = rot;
	m->vScale() = scale;
	m_v20 = pos;
	m_v30 = rot;
	m_v40 = scale;
	m_b14 = true;
	if (const wii::Mdl0* mdl = m->pMdl0())
	{
		float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
		for (const wii::Mdl0Mesh& mesh : mdl->meshes)
			for (const wii::Mdl0Vertex& v : mesh.verts)
				for (int i = 0; i < 3; i++) { mn[i] = std::min(mn[i], v.pos[i]); mx[i] = std::max(mx[i], v.pos[i]); }
		if (mn[0] <= mx[0]) SetLocalAABB(mn, mx);
	}
	UpdateWorldAABB();
}

// DOL 0x800edae8
bool cTaskGimmickBreakComet::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	UpdateWorldAABB();   // 0x801c1950
	return cTaskGimmick::Execute(pParam);
}

// DOL 0x800edb78: the player scores (0x800d73cc), PTK_COMET0x (slot 9 + type) at the comet, m01se_08bak4 (DAT_8047b5ec); the comet is gone
void cTaskGimmickBreakComet::HitCallback(cObject* other)
{
	cTaskConsolePlayer* player = dynamic_cast<cTaskConsolePlayer*>(other);
	if (player == nullptr || !m_b08) return;
	player->AddScore(m_nScore);
	(new cKingdomPtkHnd())->Generate((uint)(9 + m_nType), true, false, true, 0.0f, Vector4(m_v20.x, m_v20.y, m_v20.z, 1.0f));
	snd::Play("m01se_08bak4");
	m_b08 = false;
	m_b14 = false;
	m_b09 = false;
}

}
