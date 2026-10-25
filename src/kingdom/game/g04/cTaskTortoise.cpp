#include "kingdom/game/g04/cTaskTortoise.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"

namespace kingdom::game::g04
{

// the builder 0x80093260: new 0x2e0 (cTaskModel, three particle handles), Init (vtable +0x3c), priority 0x10004, then the model at kame_set
// rotated (0, pi, 0)
cTaskTortoise::cTaskTortoise(const Vector4& pos)
{
	SetName("g04::cTaskTortoise");
	Init();
	nDrawPriority(0x10004u);
	kingdom::cKingdomModel* m = pModel(0u);
	m->vTrans() = pos;
	m->vRotate() = Vector4(0.0f, 3.1415927f, 0.0f, 1.0f);   // -0x7094(r2) / -0x707c / -0x7080 (UNVERIFIED: the same values as 0x801b7810's)
}

// DOL 0x801b7768
void cTaskTortoise::Init()
{
	kingdom::cKingdomModel* m = CreateModel(0u);
	m->RegistModel("BIN_MODEL_KAME", 0x36f);   // 0x141e644a
	m_nOldState = 0;
	SetState(0);
	m->SetLightSetIdx(1);
	m->SetFogIdx(1);
	m_b2d0 = m_bTurned = m_bResultFx = m_b2d3 = false;
}

// DOL 0x801b7810
void cTaskTortoise::Reset()
{
	SetState(0);
	m_b2d0 = false;
	m_bTurned = false;
	pModel(0u)->vRotate() = Vector4(0.0f, 3.1415927f, 0.0f, 1.0f);   // FLOAT_8047cd88 / cd8c / cd90
	// the result effect (+0x280) detached when it was on (0x8008a508-like): natively the handle just stays
	m_bResultFx = false;
	m_b2d3 = false;
}

// DOL 0x801b7c7c
void cTaskTortoise::SetState(int state)
{
	m_nOldState = m_nState;
	m_nState = state;
	m_nTimer = 0;
	m_b2d0 = false;
	kingdom::cKingdomModel* m = pModel(0u);
	auto clip = [&](const char* name) {
		m_Anime.Attach(m, name, 0u);
		m_Anime.SetWeight(1.0f, m, 0u);
		m_Anime.SetWeight(0.0f, m, 1u);
	};
	Vector4 p = m->vTrans();
	switch (state)
	{
	case 0:
	case 3:
	case 10: clip("ANIME_KAME_A03"); break;
	case 1:
	case 2:
		clip("ANIME_KAME_A02");
		p[1] += 0.5f;   // FLOAT_8047cdac
		m_TalkFx.Generate(9u, false, true, true, state == 1 ? 100.0f : 0.0f, p);   // PTK_EFFECT10_KAME01 (state 1: delay 100)
		break;
	case 4:
		m->SetFrameAnmAll(0u, 0.0f);
		clip("ANIME_KAME_A04");
		m_Anime.Regist(m, "ANIME_KAME_A04_CLA", 0u);
		m_HitFx.Generate(6u, true, true, true, 0.0f, m->vTrans());   // PTK_EFFECT07_HIT
		break;
	case 5:
		clip("ANIME_KAME_A05");
		m_Anime.Attach(m, "TEX_ANIME_KAME_05", 0u);
		m_bResultFx = true;
		m_ResultFx.Generate(0xbu, false, false, true, 0.0f, m->vTrans());   // PTK_EFFECT12_CLEAR01
		break;
	case 6:
		clip("ANIME_KAME_A06");
		m_Anime.Attach(m, "TEX_ANIME_KAME_06", 0u);   // 0x0aee700b
		m_bResultFx = true;
		m_ResultFx.Generate(0xcu, false, false, true, 0.0f, m->vTrans());   // PTK_EFFECT13_JOY
		break;
	case 7:
		clip("ANIME_KAME_A07");
		m_Anime.Attach(m, "TEX_ANIME_KAME_07", 0u);
		m_bResultFx = true;
		m_ResultFx.Generate(0xdu, false, false, true, 0.0f, m->vTrans());   // PTK_EFFECT14_ASE01
		break;
	case 8:
		clip("ANIME_KAME_A08");
		m_Anime.Attach(m, "TEX_ANIME_KAME_08", 0u);
		m_bResultFx = true;
		m_ResultFx.Generate(0xeu, false, true, true, 0.0f, m->vTrans());   // PTK_EFFECT15_BLACK1
		break;
	case 9:
		clip("ANIME_KAME_A10");
		m_Anime.Attach(m, "TEX_ANIME_KAME_10", 0u);
		m_bResultFx = false;
		m_ResultFx.Generate(0xfu, true, true, true, 0.0f, m->vTrans());   // PTK_EFFECT16_ICE01
		break;
	default: break;
	}
}

// DOL 0x801b86b8: alone the rank 0..4 picks 5..9; in a match the winner gets 5, a tie 9
void cTaskTortoise::Result(int rank)
{
	m_bTurned = false;
	if (KingdomGlobal::WaniWaniSys->nPlayers() == 1) SetState(rank >= 0 && rank <= 4 ? 5 + rank : 9);
	else SetState(rank == 4 ? 9 : 5);
}

// DOL 0x801b7898
bool cTaskTortoise::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app->IsPause() || app->IsHome()) return true;
	kingdom::cKingdomModel* m = pModel(0u);
	switch (m_nState)
	{
	case 0:
		if (m_Anime.ChrEnd(m, 0u)) m->SetFrameAnmChr(0u, 0.0f);   // 0x8001b624: loop
		break;
	case 1:
		// 20 frames before the clip's end the talk effect is detached (+0x230, 0x8008a508-like): natively it runs out
		m_nTimer++;
		break;
	case 2:
		if (m_Anime.ChrEnd(m, 0u))
		{
			Vector4 p = m->vTrans();
			p[1] += 2.0f;   // FLOAT_8047cd98
			m_TalkFx.Generate(9u, true, true, true, 0.0f, p);
			m->SetFrameAnmChr(0u, 0.0f);
		}
		m_nTimer++;
		break;
	case 4:
		if (m_Anime.ChrEnd(m, 0u)) SetState(3);
		m_nTimer++;
		break;
	case 3:
	case 5:
	case 6: m_nTimer++; break;
	case 8:
		if (58.0f <= m->GetFrameAnmChr(0u) && !m_b2d0)   // FLOAT_8047cd9c
		{
			m_Anime.Attach(m, "ANIME_KAME_A09", 0u);
			m_Anime.Attach(m, "TEX_ANIME_KAME_10", 0u);
			m_b2d0 = true;
		}
		break;
	case 10:
	{
		Vector4 r = m->vRotate();
		if (r[1] < 0.0f) m_bTurned = true;
		r[1] -= 0.17453292f;   // FLOAT_8047cda0
		m->vRotate() = r;
		break;
	}
	case 0xb:
	{
		Vector4 p = m->vTrans();
		p[0] += 0.1f;   // FLOAT_8047cda4
		m->vTrans() = p;
		if (10.0f <= p[0]) m_b2d3 = true;   // FLOAT_8047cda8
		break;
	}
	default: break;
	}
	UpdateFrame();
	return true;
}

}
