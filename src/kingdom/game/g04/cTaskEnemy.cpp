#include "kingdom/game/g04/cTaskEnemy.hpp"
#include "kingdom/game/g04/cTaskEnemyShadow.hpp"

#include <cmath>

#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"
#include "kingdom/game/g04/cTaskEnemyManager.hpp"
#include "kingdom/game/g04/cTaskPacmanBase.hpp"
#include "kingdom/game/g04/g04Util.hpp"

namespace kingdom::game::g04
{

namespace
{
constexpr float HALF_PI = -1.5707964f;   // FLOAT_8047cc94 (-pi/2)

// DOL 0x8037cf28 [audited]: the CHR clips by +0x248
const char* const CLIP_A[5] = {"ANIME_W1_A01", "ANIME_W1_A02", "ANIME_W1_A03", "ANIME_W1_A04", "ANIME_W1_A05"};
// DOL 0x8037cff8 [audited]: the body colour clips [colour * 5 + kind] (colour 0 B, 1 G, 2 O, 3 P; kind W1..W5)
const char* const BODY_CLA[20] = {
	"ANIME_B_W1_CLA", "ANIME_B_W2_CLA", "ANIME_B_W3_CLA", "ANIME_B_W4_CLA", "ANIME_B_W5_CLA",
	"ANIME_G_W1_CLA", "ANIME_G_W2_CLA", "ANIME_G_W3_CLA", "ANIME_G_W4_CLA", "ANIME_G_W5_CLA",
	"ANIME_O_W1_CLA", "ANIME_O_W2_CLA", "ANIME_O_W3_CLA", "ANIME_O_W4_CLA", "ANIME_O_W5_CLA",
	"ANIME_P_W1_CLA", "ANIME_P_W2_CLA", "ANIME_P_W3_CLA", "ANIME_P_W4_CLA", "ANIME_P_W5_CLA"};
// DOL 0x80449fe8 [audited] (multiplayer) / 0x80449ffc (single, green only): the colour clip while out [colour * 5 + kind]
const char* const OUT_A_CLA[20] = {
	"ANIME_OA_B_W1_A_CLA", "ANIME_OA_B_W2_A_CLA", "ANIME_OA_B_W3_A_CLA", "ANIME_OA_B_W4_A_CLA", "ANIME_OA_B_W5_A_CLA",
	"ANIME_OA_G_W1_A_CLA", "ANIME_OA_G_W2_A_CLA", "ANIME_OA_G_W3_A_CLA", "ANIME_OA_G_W4_A_CLA", "ANIME_OA_G_W5_A_CLA",
	"ANIME_OA_O_W1_A_CLA", "ANIME_OA_O_W2_A_CLA", "ANIME_OA_O_W3_A_CLA", "ANIME_OA_O_W4_A_CLA", "ANIME_OA_O_W5_A_CLA",
	"ANIME_OA_P_W1_A_CLA", "ANIME_OA_P_W2_A_CLA", "ANIME_OA_P_W3_A_CLA", "ANIME_OA_P_W4_A_CLA", "ANIME_OA_P_W5_A_CLA"};
// DOL 0x8044a038 [audited]: the colour clip going back into the hole [colour * 5 + kind]
const char* const BACK_A1_CLA[20] = {
	"ANIME_OA_B_W1_A1_CLA", "ANIME_OA_B_W2_A1_CLA", "ANIME_OA_B_W3_A1_CLA", "ANIME_OA_B_W4_A1_CLA", "ANIME_OA_B_W5_A1_CLA",
	"ANIME_OA_G_W1_A1_CLA", "ANIME_OA_G_W2_A1_CLA", "ANIME_OA_G_W3_A1_CLA", "ANIME_OA_G_W4_A1_CLA", "ANIME_OA_G_W5_A1_CLA",
	"ANIME_OA_O_W1_A1_CLA", "ANIME_OA_O_W2_A1_CLA", "ANIME_OA_O_W3_A1_CLA", "ANIME_OA_O_W4_A1_CLA", "ANIME_OA_O_W5_A1_CLA",
	"ANIME_OA_P_W1_A1_CLA", "ANIME_OA_P_W2_A1_CLA", "ANIME_OA_P_W3_A1_CLA", "ANIME_OA_P_W4_A1_CLA", "ANIME_OA_P_W5_A1_CLA"};
// DOL 0x8037cf40 [audited]: the eye clip by kind
const char* const EYE_TSA[5] = {"ANIME_EYE_W1_TSA", "ANIME_EYE_W2_TSA", "ANIME_EYE_W3_TSA", "ANIME_EYE_W4_TSA", "ANIME_EYE_W5_TSA"};
// DOL 0x8037cf58 [audited]: {TPA, CLA} by (kind + colour * 5)
const char* TexPair(int idx, int which)
{
	static const char* const COLOR = "BGOP";
	static char buf[2][40][32];
	char* b = buf[which][idx % 40];
	std::snprintf(b, 32, "ANIME_%c_W%d_%s", COLOR[(idx / 5) & 3], idx % 5 + 1, which == 0 ? "TPA" : "CLA");
	return b;
}

bool IsSingle() { return KingdomGlobal::WaniWaniSys->nPlayers() == 1; }

float Dist3(const Vector4& a, const Vector4& b)
{
	return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}
}

// the manager's 0x801ae84c: new 0x630 (cTaskModel ctor, the vtables, the command vector, 5 + 6 particle handles), +0x1dc / +0x250 = place,
// +0x278 = manager, vtable +0x3c = Init
cTaskEnemy::cTaskEnemy(cTaskEnemyManager* pMan, uint nPlace) : m_pMan(pMan), m_Rand(0x20061206u)
{
	SetName("g04::cTaskEnemy");
	m_nModel = (int)nPlace;
	m_nPlace = nPlace;
	Init();
}

cTaskEnemy::~cTaskEnemy() = default;

// DOL 0x801a91e0 [audited]
void cTaskEnemy::Init()
{
	kingdom::cKingdomModel* m = CreateModel(0u);   // FUN_8009991c
	m->RegistModel(m_nPlace == 0 ? "BIN_MODEL_WANI_2" : "BIN_MODEL_WANI", 0x36f);   // 0x9bdf805e / 0x2c4a9385
	m->Visible();
	m_Anime.Regist(m, "ANIME_G_W1_TPA", 0u);
	m_Anime.Regist(m, "ANIME_G_W1_CLA", 0u);
	m_Anime.Regist(m, "ANIME_EYE_W2_TSA", 0u);
	m = CreateModel(1u);
	m->RegistModel(m_nPlace == 0 ? "BIN_MODEL_WANI_RED_2" : "BIN_MODEL_WANI_RED", 0x36f);   // 0x3d41c554 / 0x3c318f33
	m->Invisible();
	m_Anime.Regist(m, "ANIME_EYE_W6_TSA", 0u);
	pModel(0u)->Visible();
	// cParamEnemyCommon (CSV_ENEMY_COMMON, 0xc5485803)
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_ENEMY_COMMON"))
	{
		m_Param.fMoveDistance = csv.F32(0, 1, 0.0f);
		for (int i = 0; i < 4; i++) m_Param.fAngle[i] = csv.F32(i + 1, 1, 0.0f);
		m_Param.fAdvanceFrame = csv.F32(5, 1, 0.0f);
		m_Param.fReturnFrame = csv.F32(6, 1, 0.0f);
		m_Param.fClrAdvance = csv.F32(7, 1, 0.0f);
		m_Param.fClrReturn = csv.F32(8, 1, 0.0f);
		m_Param.fDamageSpeed = csv.F32(9, 1, 0.0f);
	}
	m_fReturn = -0.05f;   // FLOAT_8047cc90
	m_nAttack = 0;
	m_bSynced = false;
	m_fFrame = 0.0f;
	m_nClip = 0;
	m_fAllFrame = m_fAttackProb = m_fWait = m_fMove = m_fBefore = m_fAfter = 0.0f;
	m_nRed = 0;
	m_nSync = 0;
	m_nEyeTimer = 0;
	m_bEye = false;
	m_b620 = false;
	m_f624 = 0.0f;
	for (uint i = 0; i < 2; i++)
	{
		kingdom::cKingdomModel* mi = pModel(i);
		mi->SetFogIdx((int)m_nPlace + 2);
		mi->SetLightSetIdx((int)m_nPlace + 2);
		if (m_nPlace != 0) mi->IsDrawShadowDepth(true);   // model vtable +0x20 (0x80020298): places 1..4 cast the depth shadow (place 0 has its blob, below)
		mi->vRotate()[1] = m_Param.fAngle[m_nModel];   // the CSV value in degrees; the manager overwrites it with radians right after
		mi->Invisible();
	}
	// place 0: one cTaskEnemyShadow per model (0x801a9594: new 0x2c0, +0x2b0 = this, connected, hung on node "wani" with 0x8001f798)
	if (m_nPlace == 0)
		for (uint i = 0; i < 2; i++)
		{
			m_pShadow[i] = new cTaskEnemyShadow(this, i);   // +0x270 + 4i
			ConnectChild(m_pShadow[i]);
		}
	m_nModel = 0;   // the DOL computes the front from model 0, then clears +0x1dc
	m_vFront = m_vHome;
	FrontPos();
	m_nKind = 0;
	m_nColor = 1;
	SetState(1);
}

// DOL 0x801a9810 [audited] (the rand and the param are members natively)
void cTaskEnemy::Term()
{
	cTaskModel::Term();
}

// DOL 0x801a96fc [audited] (a new game: the look back to green W1, the timing cleared)
void cTaskEnemy::Reset()
{
	kingdom::cKingdomModel* m = pModel(0u);
	m->Visible();
	m_Anime.Regist(m, "ANIME_G_W1_TPA", 0u);
	m_Anime.Regist(m, "ANIME_G_W1_CLA", 0u);
	m_Anime.Regist(m, "ANIME_EYE_W2_TSA", 0u);
	m = pModel(1u);
	m->Invisible();
	m_Anime.Regist(m, "ANIME_EYE_W6_TSA", 0u);
	m_fMove = m_fReturn = m_fAttackProb = m_fWait = 0.0f;
	m_nRed = 0;
	m_nAttack = 0;
	m_nSync = 0;
	m_bSynced = false;
	m_nModel = 0;
	m_nEyeTimer = 0;
	m_bEye = false;
	SetState(1);
}

// DOL 0x801ad238 [audited]
void cTaskEnemy::SetHomePos(const Vector4& v)
{
	m_vHome = v;
}

void cTaskEnemy::FrontPos()
{
	const float ry = pCurModel()->vRotate()[1];
	m_vFront = m_vHome;
	m_vFront[0] += std::cos(HALF_PI + ry) * m_Param.fMoveDistance;
	m_vFront[2] += -std::sin(HALF_PI + ry) * m_Param.fMoveDistance;
}

// the block the DOL inlines everywhere (SetState, SetLook): the colour clip, its rate 0 (0x8001cd4c), gator A's shadow clip rate 0 (0x801b0378)
void cTaskEnemy::ColorClip()
{
	kingdom::cKingdomModel* m = pCurModel();
	if (m_nRed == 1) m_Anime.Regist(m, "ANIME_OA_R_A1_CLA", 0u);   // 0x0629244e
	else if (!IsSingle()) m_Anime.Regist(m, OUT_A_CLA[(m_nColor * 5 + m_nKind) % 20], 0u);
	else m_Anime.Regist(m, OUT_A_CLA[5 + m_nKind % 5], 0u);   // 0x80449ffc: green
	m->SetUpdateRateMatClr(0u, 0.0f);
	if (Shadow()) Shadow()->SetClrRate(0.0f);
}

// the other inlined block (SetState 9 / 0xc / 0x10 / 0x12 / 0x13, Move, AngryOut): the body clip, then gator A's shadow back to WANI_SH_CLA (0x801b0360)
void cTaskEnemy::BodyClr(bool)
{
	kingdom::cKingdomModel* m = pCurModel();
	if (m_nRed == 0) m_Anime.Regist(m, BODY_CLA[(m_nKind + m_nColor * 5) % 20], 0u);
	else
	{
		m_Anime.Regist(m, "ANIME_OA_R_CLA", 0u);   // 0x3e67bcf6
		m->SetUpdateRateMatClr(0u, 1.0f);
		m_Anime.Regist(m, "ANIME_OA_R_CLA", 1u);
		m->SetUpdateRateMatClr(1u, 1.0f);
	}
	if (Shadow()) Shadow()->ClipNormal();
}

// slot 1's clip, rate (0x8001d790) and frame (0x8001c2e0) go to slot 0 (0x8001d124 / 0x8001bc74: all kinds), `next` goes into slot 1
void cTaskEnemy::SwapChr(const char* next)
{
	kingdom::cKingdomModel* m = pCurModel();
	m_Anime.Attach(m, CLIP_A[m_nClip % 5], 0u);
	const float rate = m->GetUpdateRateAnmChr(1u), frame = m->GetFrameAnmChr(1u);
	m->SetUpdateRateAll(0u, rate);
	m->SetFrameAnmAll(0u, frame);
	m_Anime.Attach(m, next, 1u);
	m_Anime.SetWeight(1.0f, m, 0u);
	m_Anime.SetWeight(0.0f, m, 1u);
}

// DOL 0x801aafd4 [audited: case by case vs decomp (earlier pass)] (checked case by case against the decomp)
void cTaskEnemy::SetState(int state)
{
	const int old = m_nState;
	m_nOldState = old;
	m_nState = state;
	m_fTimer = 0.0f;
	m_b254 = false;
	m_b620 = false;
	if (old == 0x13) pModel(0u)->SetFogIdx(4);
	kingdom::cKingdomModel* m = pCurModel();
	switch (state)
	{
	case 0:
		for (uint i = 0; i < 2; i++) pModel(i)->Invisible();
		break;
	case 2:
		m->Invisible();
		m->SetFrameAnmAll(0u, 0.0f);   // 0x8001bc74
		m->SetUpdateRateAll(0u, 1.0f); // 0x8001d124
		m_Anime.Attach(m, "ANIME_W1_A01", 0u);
		m_Anime.Attach(m, "ANIME_W1_A02", 1u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
		ColorClip();
		m_nClip = 0;
		break;
	case 3:
		m->Invisible();
		break;
	case 4:
		if (m_nClip == 4) m_Anime.Regist(m, "ANIME_EYE_W2_TSA", 0u);
		ColorClip();
		m_Anime.Attach(m, "ANIME_W1_A01", 0u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
		m->Invisible();
		m_nClip = 0;
		break;
	case 5:
		m_fFrame = 0.0f;
		m->Invisible();
		m->SetFrameAnmAll(0u, 0.0f);
		m->SetUpdateRateAll(0u, 1.0f);
		ColorClip();
		m_Anime.Attach(m, "ANIME_W1_A01", 0u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
		m_nClip = 0;
		break;
	case 6:
		m->Invisible();
		ColorClip();
		break;
	case 7:
	{
		for (bool& b : m_bFoot) b = false;
		m->Visible();
		if (m_nPlace == 0)
		{
			ColorClip();
		}
		else
			BodyClr(true);
		m_Anime.Attach(m, "ANIME_W1_A01", 0u);
		m_Anime.Attach(m, "ANIME_W1_A02", 1u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
		m->SetUpdateRateAnmChr(0u, m_Param.fAdvanceFrame / m_fMove);   // 0x8001cad4
		// the colour clip runs over the part of the move between the show node and the front
		const float a = Dist3(m_vHome, m_vShow), b = Dist3(m_vHome, m_vFront);
		const float rate = m_Param.fClrAdvance / ((a / b) * m_fMove);
		m->SetUpdateRateMatClr(0u, rate);
		if (Shadow()) Shadow()->SetClrRate(rate);   // 0x801b0378
		m_nClip = 1;
		break;
	}
	case 8:
		for (bool& b : m_bFoot) b = false;
		SwapChr("ANIME_W1_A03");
		m_nClip = 2;
		break;
	case 9:
		BodyClr(true);
		SwapChr("ANIME_W1_A01");
		m_nClip = 0;
		break;
	case 10:
		SwapChr("ANIME_W1_A01");
		m_nClip = 0;
		break;
	case 0xb:
		SwapChr("ANIME_W1_A04");
		if (m_nRed != 0)
		{
			m->SetUpdateRateMatClr(0u, 0.0f);
			m->SetFrameMatClr(0u, m_Param.fClrAdvance);   // 0x8001b89c
		}
		m_nClip = 3;
		break;
	case 0xc:
	case 0x10:
	{
		SwapChr("ANIME_W1_A05");
		m->SetFrameAnmAll(0u, 0.0f);
		m_Anime.Regist(m, "ANIME_EYE_W2_TSA", 0u);
		m->ReleaseAnmTexSrt(0u);   // 0x8001b3f0
		m_nClip = 4;
		for (bool& b : m_bFoot) b = false;
		Vector4 p = m->vTrans();
		m_f624 = Dist3(p, m_vHome) / m_Param.fDamageSpeed;
		BodyClr(true);
		break;
	}
	case 0xe:
		m->SetFrameAnmAll(0u, 0.0f);
		m->SetUpdateRateAll(0u, 1.0f);
		ColorClip();
		m_Anime.Attach(m, "ANIME_W1_A01", 0u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
		m->Invisible();
		break;
	case 0xf:
		m->Invisible();
		ColorClip();
		break;
	case 0x11:
	{
		const float ry = pRedSelModel()->vRotate()[1];
		m_vFront = m_vHome;
		m_vFront[0] += std::cos(HALF_PI + ry) * m_Param.fMoveDistance;
		m_vFront[2] += -std::sin(HALF_PI + ry) * m_Param.fMoveDistance;
		for (bool& b : m_bFoot) b = false;
		if (!IsSingle()) pModel(1u)->Invisible();
		m_Anime.Attach(m, "ANIME_W1_A01", 0u);
		m_Anime.Attach(m, "ANIME_W1_A02", 1u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
		ColorClip();
		SetLook(m_nKind, IsSingle() ? m_nModel : m_nColor);
		m = pCurModel();
		m->SetUpdateRateAnmChr(0u, m_Param.fAdvanceFrame / m_fMove);
		const float a = Dist3(m_vHome, m_vShow), b = Dist3(m_vHome, m_vFront);
		m->SetUpdateRateMatClr(0u, m_Param.fClrAdvance / ((a / b) * m_fMove));
		if (m_nPlace != 0) m->Invisible();
		m_nClip = 1;
		break;
	}
	case 0x12:
		if (m_nPlace == 0)
		{
			BodyClr(true);
			SwapChr("ANIME_W1_A01");
			m_nClip = 0;
		}
		break;
	case 0x13:
		if (m_nPlace == 2)
		{
			m_nKind = 2;
			m_nRed = 0;
			m_nColor = 1;
			m_nModel = 0;
			SetLook(2, 1);
			m = pCurModel();
			BodyClr(true);
			m->Visible();
			m->SetFogIdx(-1);
			m_Anime.Attach(m, "ANIME_W1_A01", 0u);
			m_Anime.SetWeight(1.0f, m, 0u);
			m_Anime.SetWeight(0.0f, m, 1u);
		}
		else
			for (uint i = 0; i < 2; i++) pModel(i)->Invisible();
		break;
	case 0x17:
	{
		const float rate = m->GetUpdateRateAnmChr(1u), frame = m->GetFrameAnmChr(1u);
		m->SetUpdateRateAll(0u, rate);
		m->SetFrameAnmAll(0u, frame);
		if (Shadow()) Shadow()->ClipA();   // 0x801b0348
		if (m_nRed == 1) m_Anime.Regist(m, "ANIME_OA_R_A_CLA", 0u);   // 0xa7179b74
		else m_Anime.Regist(m, BACK_A1_CLA[(m_nKind + m_nColor * 5) % 20], 0u);
		m_Anime.Attach(m, "ANIME_W1_A01", 0u);
		m_Anime.Attach(m, "ANIME_W1_A03", 1u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
		for (bool& b : m_bFoot) b = false;
		break;
	}
	default:
		break;
	}
}

// DOL 0x801ad250 [audited: checked]
void cTaskEnemy::NextCommand()
{
	if (m_nCmd < m_cmd.size())
	{
		const sEnemyCommand* c = m_cmd[m_nCmd++];
		pCurModel()->vTrans() = m_vHome;
		if (c->nType == 1)
		{
			m_bSynced = true;
			SetState(6);
		}
		else if (c->nType == 0)
		{
			SetupCommand(c);
			if (m_nState != 0xf) SetState(2);
		}
		else if (c->nType == 2)
		{
			SetupCommand(c);
			m_pMan->AddRandGroup(c->nMask);   // 0x801af7dc
			if (m_nState != 0xf) SetState(0xe);
		}
	}
	else
	{
		SetState(0xd);
		m_nCmd = 0;
	}
}

// DOL 0x801ad688 [audited: checked]
void cTaskEnemy::SetupCommand(const sEnemyCommand* c)
{
	const sEnemyPattern* p = c->pPattern;
	m_fMove = p->fMove;
	m_fReturn = p->fReturn;
	m_fAllFrame = p->fAllFrame;
	m_fWait = p->fWait;
	m_fBefore = p->fAttackBeforeWait;
	m_fAfter = p->fAttackAfterWait;
	m_nRed = c->nRed;
	m_nRedHP = c->nRedHP;
	if (0.0f < p->fAttack) m_nAttack = (m_Rand() % 100u) < (uint)p->fAttack ? 1 : 0;
	else m_nAttack = 0;
	m_fAttackProb = p->fAttack;
	const float need = 0.0f + m_fMove + m_fReturn + m_fWait + m_fBefore + m_fAfter + 30.0f;   // FLOAT_8047ccc0
	if (m_fAllFrame < need) m_fAllFrame = need;
	if ((float)m_pMan->nFramesLeftInBlock() <= m_fAllFrame) SetState(0xf);
	SetLook(c->nKind, c->nColor);
	m_nModel = c->nRed;
	for (uint i = 0; i < 2; i++) pModel(i)->Invisible();
}

// DOL 0x801ad840 [audited: checked incl tables]
void cTaskEnemy::SetLook(int kind, int color)
{
	kingdom::cKingdomModel* cur = pCurModel();
	const Vector4 t = cur->vTrans(), r = cur->vRotate(), s = cur->vScale();
	m_nColor = IsSingle() ? 1 : color;
	m_nKind = kind;
	if (m_nRed == 1)
	{
		m_nModel = 1;
		m_Anime.Regist(pModel(1u), "ANIME_EYE_W6_TSA", 0u);
	}
	else
	{
		m_nModel = 0;
		const int idx = IsSingle() ? kind + 5 : kind + color * 5;
		kingdom::cKingdomModel* m = pModel(0u);
		m_Anime.Regist(m, TexPair(idx, 0), 0u);
		m_Anime.Regist(m, TexPair(idx, 1), 0u);
		m_Anime.Regist(m, EYE_TSA[kind % 5], 0u);
	}
	pCurModel()->Visible();
	pModel((uint)(1 - m_nModel))->Invisible();
	ColorClip();
	kingdom::cKingdomModel* m = pCurModel();
	m->vTrans() = t;
	m->vRotate() = r;
	m->vScale() = s;
}

// DOL 0x801adf08 [audited: checked] (the result look: single = kind `k`, multiplayer = the winner's colour, 4 = red)
void cTaskEnemy::SetResultLook(int k)
{
	if (IsSingle())
	{
		m_nModel = 0;
		m_nKind = k;
		m_fMove = 30.0f;
		m_fWait = 0.0f;
		m_nColor = 1;
		m_nRed = 0;
	}
	else
	{
		m_nKind = 0;
		if (k == 4)
		{
			m_nRed = 1;
			m_nModel = 1;
		}
		else
		{
			m_nRed = 0;
			m_nColor = k;
			std::mt19937 r(0x20061206u);   // a fresh cKingdomRand: always the same first value
			m_nModel = 0;
			m_nKind = (int)(r() % 5u);
		}
		m_fMove = 30.0f;
	}
	SetLook(m_nKind, m_nColor);
}

// DOL 0x801adcb0 [audited: checked]
void cTaskEnemy::MouthPos(float out[3])
{
	kingdom::cKingdomModel* m = pCurModel();
	const float ry = m->vRotate()[1];
	const float c = std::cos(HALF_PI + ry), s = -std::sin(HALF_PI + ry);
	Vector4 a = ResNodeTrans(m, "chest"), b = ResNodeTrans(m, "neck"), d = ResNodeTrans(m, "mouth");
	out[0] = c * (a[0] + b[0] + d[0]);
	out[1] = a[1] + b[1] + d[1];
	out[2] = s * (a[2] + b[2] + d[2]);
}

// DOL 0x801ae1f0 [audited: checked]: the hit effect at the mouth (+1, +1 in y / z): slot player + 1 (multiplayer), 4 (red), 0
void cTaskEnemy::HitEffect(uint player)
{
	float o[3];
	MouthPos(o);
	Vector4 t = pRedSelModel()->vTrans();
	Vector4 p(t[0] + o[0], t[1] + o[1] + 1.0f, t[2] + o[2] + 1.0f, 1.0f);
	if (!IsSingle()) m_HitFx[player % 5].Generate(player + 1, true, true, true, 0.0f, p);
	else if (m_nRed == 1) m_HitFx[4].Generate(4u, true, true, true, 0.0f, p);
	else m_HitFx[0].Generate(0u, true, true, true, 0.0f, p);
}

// DOL 0x801ae024 [audited: checked]: the hit effect, then the eye blinks shut for `frames` (EYE_W2, the SRT clip released; Execute puts EYE_W6 back on red)
void cTaskEnemy::HitBlink(int frames, uint player)
{
	HitEffect(player);
	m_nEyeTimer = frames;
	m_bEye = true;
	kingdom::cKingdomModel* m = pCurModel();
	m_Anime.Regist(m, "ANIME_EYE_W2_TSA", 0u);
	m->ReleaseAnmTexSrt(0u);
}

// DOL 0x801ade58 [audited: checked]
bool cTaskEnemy::IsHittableOut(float range) const
{
	if (m_nState != 7) return false;
	const float t = (m_fTimer + range) / m_fMove;
	const float lo = m_nPlace == 3 ? 0.9f : 0.8f;   // FLOAT_8047ccc4 / FLOAT_8047ccb8
	return lo <= t && t <= 1.0f;
}

// DOL 0x801aded8 [audited: checked]: still more than (range - 4) frames of "before" left
bool cTaskEnemy::IsBeforeBiteEnd(float range) const
{
	return !(m_fBefore - m_fTimer <= range - 4.0f);
}

// DOL 0x801ae714 [audited: checked]
bool cTaskEnemy::IsAfterBite(float range) const
{
	if (m_nState != 10) return false;
	return !(m_fAfter - m_fTimer <= range);
}

// the foot splashes at eff_foot_l / eff_foot_r (+ pos): place 0 (the far gator) kicks up smoke (slot 0x11, PTK_EFFECT18_SMOKE01), the others
// water (slot 8, PTK_EFFECT09_WATER01). The three DOL copies raise the splashes differently (upL / upR); 0x801ae4a4 puts gator A's right
// splash at pos itself (it passes pos, not the node sum: kept)
void cTaskEnemy::FootFxAt(int k, const float pos[3], float upL, float upR, bool rightAtPos)
{
	kingdom::cKingdomModel* m = pCurModel();
	const uint slot = m_nPlace == 0 ? 0x11u : 8u;
	const Vector4 l = ResNodeTrans(m, "eff_foot_l");
	m_FootFx[k].Generate(slot, true, true, true, 0.0f, Vector4(pos[0] + l[0], pos[1] + l[1] + upL, pos[2] + l[2], 1.0f));
	if (rightAtPos)
	{
		m_FootFx[k].Generate(slot, true, true, true, 0.0f, Vector4(pos[0], pos[1], pos[2], 1.0f));
		return;
	}
	const Vector4 r = ResNodeTrans(m, "eff_foot_r");
	m_FootFx[k].Generate(slot, true, true, true, 0.0f, Vector4(pos[0] + r[0], pos[1] + r[1] + upR, pos[2] + r[2], 1.0f));
}

// DOL 0x801ae4a4 [audited: checked]: the first unspent sixth of the walk that t has passed makes a splash
void cTaskEnemy::FootFx(float t, const float pos[3])
{
	for (int k = 0; k < 6; k++)
		if (0.16666667f * (float)k < t && !m_bFoot[k])   // FLOAT_8047cca0
		{
			if (m_nPlace == 0) FootFxAt(k, pos, 0.0f, 0.0f, true);
			else FootFxAt(k, pos, 0.2f, 0.2f);   // FLOAT_8047cc9c
			m_bFoot[k] = true;
			return;
		}
}

// DOL 0x801a9d60 [audited: foot fx order/offsets, ClrEnd gate, blend 0.1 checked] (state 7)
void cTaskEnemy::Move(cJetTask*)
{
	kingdom::cKingdomModel* m = pCurModel();
	const float t = m_fTimer / m_fMove;
	Vector4 p = m->vTrans();
	p[0] = m_vFront[0] * t + (1.0f - t) * m_vHome[0];
	p[1] = m_vFront[1] * t + (1.0f - t) * m_vHome[1];
	p[2] = m_vFront[2] * t + (1.0f - t) * m_vHome[2];
	m->vTrans() = p;
	m_fTimer += 1.0f;
	const float pos[3] = {p[0], p[1], p[2]};
	for (int k = 0; k < 6; k++)   // the splash inlined: both feet + 0.3 (FLOAT_8047cca4), gator A's right foot not raised
		if (0.16666667f * (float)k < t && !m_bFoot[k])
		{
			FootFxAt(k, pos, 0.3f, m_nPlace == 0 ? 0.0f : 0.3f);
			m_bFoot[k] = true;
			break;
		}
	if (m_nPlace == 0 && !m_b254 && m_Anime.ClrEnd(m, 0u))
	{
		m_b254 = true;
		BodyClr(true);
	}
	m_Anime.Blend(0.1f, m);   // FLOAT_8047cca8
	if (1.0f <= t) SetState(9);
}

// DOL 0x801aa254 [audited: lerp, clip rate, SetFrameMatClr checked] (state 8)
void cTaskEnemy::Back()
{
	kingdom::cKingdomModel* m = pCurModel();
	const float t = m_fTimer / m_fReturn;
	Vector4 p = m->vTrans();
	p[0] = (1.0f - t) * m_vFront[0] + t * m_vHome[0];
	p[1] = (1.0f - t) * m_vFront[1] + t * m_vHome[1];
	p[2] = (1.0f - t) * m_vFront[2] + t * m_vHome[2];
	m->vTrans() = p;
	m_fTimer += 1.0f;
	const float pos[3] = {p[0], p[1], p[2]};
	FootFx(t, pos);
	if (m_nPlace == 0)
	{
		if (p[2] < m_vHide[2] && !m_b620)
		{
			if (Shadow()) Shadow()->ClipA();   // 0x801b0348
			if (m_nRed == 1) m_Anime.Regist(m, "ANIME_OA_R_A_CLA", 0u);
			else m_Anime.Regist(m, BACK_A1_CLA[(m_nKind + m_nColor * 5) % 20], 0u);
			const float a = Dist3(m_vHide, m_vHome), b = Dist3(m_vFront, m_vHome);
			const float rate = m_Param.fClrReturn / ((a / b) * m_fReturn);
			m->SetUpdateRateMatClr(0u, rate);
			if (Shadow()) Shadow()->SetClrRate(rate);   // 0x801b0378
			m_b620 = true;
		}
		if (m_Anime.ClrEnd(m, 0u)) m_b254 = true;
		if (m_b254) m->SetFrameMatClr(0u, m_Param.fClrReturn);
	}
	m_Anime.Blend(0.1f, m);
	if (1.0f <= t) SetState(4);
}

// DOL 0x801aa548 [audited: damage branches, camera shake, clr rate checked] (state 0xb)
void cTaskEnemy::Bite()
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	kingdom::cKingdomModel* m = pCurModel();
	m_fTimer += 1.0f;
	m_Anime.Blend(0.2f, m);   // FLOAT_8047cc9c
	if (m_Anime.ChrEnd(m, 1u))
	{
		SetState(10);
		WiiSnd::Handle h;
		WiiSnd::Play(h, "m04se_09baku");   // DAT_8047cc88 -> 0x8037cf18
		app->TortoiseReact(4);             // 0x801b7c7c(+0x280, 4)
		cTaskPacmanManager* pm = app->pPacmanManager();
		if (pm->nPlayer() == 1) pm->Damage(0u);   // 0x801b6484
		else if (m_nRed == 1)
			for (uint i = 0; i < (uint)KingdomGlobal::WaniWaniSys->nPlayers(); i++) pm->Damage(i);
		else if ((uint)m_nColor < (uint)KingdomGlobal::WaniWaniSys->nPlayers())
			pm->Damage((uint)m_nColor);
		app->CameraShake(1);               // 0x8019d724(+0x2a4, 1)
		m->SetUpdateRateMatClr(0u, 0.0f);
		if (Shadow()) Shadow()->SetClrRate(0.0f);   // 0x801b0378
		WiiSnd::Detach(h);
	}
}

// DOL 0x801aa718 [audited: checked] (state 0xc)
void cTaskEnemy::BackOff()
{
	kingdom::cKingdomModel* m = pCurModel();
	Vector4 p = m->vTrans();
	const float ry = m->vRotate()[1];
	const float c = std::cos(HALF_PI + ry), s = -std::sin(HALF_PI + ry);
	if (!m_Anime.ChrEnd(m, 1u) || m_b254) m_Anime.Blend(0.25f, m);   // FLOAT_8047ccbc
	else m_b254 = true;
	if (m_b254)
	{
		m_fTimer += 1.0f;
		for (int k = 0; k < 6; k++)
			if (0.16666667f * (float)k < m_fTimer / m_f624 && !m_bFoot[k])
			{
				const float pos[3] = {p[0], p[1], p[2]};
				FootFxAt(k, pos, 0.0f, 0.0f);   // inlined here without any raise
				m_bFoot[k] = true;
				break;
			}
	}
	if (m_nPlace == 0 && !m_b620 && m_b254)
	{
		if (Shadow()) Shadow()->ClipA();   // 0x801b0348
		if (m_nRed == 1) m_Anime.Regist(m, "ANIME_OA_R_A_CLA", 0u);
		else m_Anime.Regist(m, BACK_A1_CLA[(m_nKind + m_nColor * 5) % 20], 0u);
		m->SetUpdateRateMatClr(0u, m_Param.fClrReturn / m_f624);
		if (Shadow()) Shadow()->SetClrRate(m_Param.fClrReturn / m_f624);   // 0x801b0378
		m_b620 = true;
	}
	if (m_b254)
	{
		p[0] = c * -m_Param.fDamageSpeed + p[0];
		p[2] = s * -m_Param.fDamageSpeed + p[2];
	}
	m->vTrans() = p;
	if (p[2] < m_vHome[2]) SetState(4);
}

// DOL 0x801aac00 [audited: checked] (state 0x11: only the far gator A comes out for the angry event)
void cTaskEnemy::AngryOut()
{
	if (m_nPlace != 0) return;
	kingdom::cKingdomModel* m = pCurModel();
	const float t = m_fTimer / m_fMove;
	Vector4 p = m->vTrans();
	p[0] = m_vFront[0] * t + (1.0f - t) * m_vHome[0];
	p[1] = m_vFront[1] * t + (1.0f - t) * m_vHome[1];
	p[2] = m_vFront[2] * t + (1.0f - t) * m_vHome[2];
	m->vTrans() = p;
	m_fTimer += 1.0f;
	for (int k = 0; k < 6; k++)
		if (0.16666667f * (float)k < t && !m_bFoot[k])
		{
			const float pos[3] = {p[0], p[1], p[2]};
			m_FootFx[k].Generate(0x11u, true, true, true, 0.0f, Vector4(pos[0], pos[1], pos[2], 1.0f));
			m_bFoot[k] = true;
			break;
		}
	if (!m_b254 && m_Anime.ClrEnd(m, 0u))
	{
		m_b254 = true;
		BodyClr(true);
	}
	m_Anime.Blend(0.1f, m);
	if (1.0f <= t) SetState(0x12);
}

// DOL 0x801ae380 [audited: checked] (state 0x17: place 0 goes back in 32 frames)
void cTaskEnemy::EventBack()
{
	if (m_nPlace != 0) return;
	kingdom::cKingdomModel* m = pCurModel();
	const float t = m_fTimer * 0.03125f;   // FLOAT_8047cccc
	Vector4 p = m->vTrans();
	p[0] = (1.0f - t) * m_vFront[0] + t * m_vHome[0];
	p[1] = (1.0f - t) * m_vFront[1] + t * m_vHome[1];
	p[2] = (1.0f - t) * m_vFront[2] + t * m_vHome[2];
	m->vTrans() = p;
	m_fTimer += 1.0f;
	const float pos[3] = {p[0], p[1], p[2]};
	FootFx(t, pos);
	if (1.0f <= t) SetState(0);
	m_Anime.Blend(0.2f, m);
}

// DOL 0x801a992c [audited: cases/consts checked; DOL ends with cJetTask::Execute (no model UpdateFrame), port calls UpdateFrame UNVERIFIED]. The app's pause flag (+0x1b4) and the HOME bit stop everything; natively the clips then do not advance either (the DOL
// advances them in the app's ScnRoot::UpdateFrame, which the pause skips).
bool cTaskEnemy::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app->IsPause() || app->IsHome()) return true;
	m_fFrame += 1.0f;
	kingdom::cKingdomModel* m = pCurModel();
	switch (m_nState)
	{
	case 2:
		m_fTimer += 1.0f;
		if (m_fWait <= m_fTimer)
		{
			FrontPos();
			SetState(7);
		}
		break;
	case 3:
		if (m_nSync == 1) SetState(2);
		break;
	case 4:
		if (m_fAllFrame <= m_fFrame) SetState(5);
		break;
	case 5:
		if (m_nSync == 1) m_nSync = 0;
		app->pPacmanManager()->ForgetPlace(m_nPlace);   // 0x801b65f8
		NextCommand();
		break;
	case 7: Move(pParam); break;
	case 8: Back(); break;
	case 9:
		m_fTimer += 1.0f;
		m_Anime.Blend(0.2f, m);
		if (m_fBefore <= m_fTimer) SetState(m_nAttack == 1 ? 0xb : 10);
		break;
	case 10:
		m_fTimer += 1.0f;
		m_Anime.Blend(0.2f, m);
		if (m_fAfter <= m_fTimer) SetState(8);
		break;
	case 0xb: Bite(); break;
	case 0xc: BackOff(); break;
	case 0x10: m_fTimer += 1.0f; break;
	case 0x11: AngryOut(); break;
	case 0x12: m_Anime.Blend(0.2f, m); break;
	case 0x13:
		if (m_nPlace == 2)
		{
			const Vector4 e = app->ParamGame().vEventEnemyPos;   // cParamGame +0x60
			Vector4 p = m_vHome;
			p[0] = e.x;
			p[1] = e.y;
			p[2] = e.z;
			m->vTrans() = p;
		}
		break;
	case 0x17: EventBack(); break;
	default: break;
	}
	if (m_bEye && --m_nEyeTimer == 0)
	{
		if (m_nRed != 0) m_Anime.Regist(pCurModel(), "ANIME_EYE_W6_TSA", 0u);
		m_bEye = false;
	}
	UpdateFrame();   // the app's ScnRoot::UpdateFrame (0x8009219c tail) advances every model's clips
	return true;
}

// DOL 0x801a9d5c [audited]: b cTaskModel::Draw (the models are submitted for this frame)
void cTaskEnemy::Draw(int nStatus, cJetTask* pParam)
{
	cTaskModel::Draw(nStatus, pParam);
}

}
