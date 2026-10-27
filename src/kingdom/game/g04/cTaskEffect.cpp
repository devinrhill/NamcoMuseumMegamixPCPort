#include "kingdom/game/g04/cTaskEffect.hpp"

#include <cstdio>
#include <random>

#include "GamePad.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkMan.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"

namespace kingdom::game::g04
{

// DOL 0x801a86d0
cTaskEffect::cTaskEffect()
{
	SetName("g04::cTaskEffect");
	static const char* const SLOT[27] = {
		"PTK_EFFECT01_HIT", "PTK_EFFECT02_HIT", "PTK_EFFECT03_HIT", "PTK_EFFECT04_HIT", "PTK_EFFECT05_HIT", "PTK_EFFECT06_WATER",
		"PTK_EFFECT07_HIT", "PTK_EFFECT08_LINE", "PTK_EFFECT09_WATER01", "PTK_EFFECT10_KAME01", "PTK_EFFECT11_SMOKE", "PTK_EFFECT12_CLEAR01",
		"PTK_EFFECT13_JOY", "PTK_EFFECT14_ASE01", "PTK_EFFECT15_BLACK1", "PTK_EFFECT16_ICE01", "PTK_EFFECT17_LENS01", "PTK_EFFECT18_SMOKE01",
		"PTK_EFFECT19_LIGHT01", "PTK_EFFECT20_LIGHT01", "PTK_EFFECT21_LIGHT01", "PTK_EFFECT22_LIGHT01", "PTK_EFFECT23_TORI01",
		"PTK_EFFECT24_LIGHT01", "PTK_EFFECT25_LIGHT01", "PTK_EFFECT26_LIGHT01", "PTK_EFFECT27_LIGHT01"};   // 0x8037ce30
	get_KingdomPtkMan()->Create(27u, 0x5au, 0xfau, 0x9c4u);
	for (unsigned i = 0; i < 27; i++) get_KingdomPtkMan()->RegistData(i, SLOT[i]);
}

// DOL 0x801a8784
bool cTaskEffect::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app != nullptr && (app->IsPause() || app->IsHome())) return true;
	return cTaskPBPtkMan::Execute(pParam);
}

// DOL 0x801a8b50 + 0x801a8d1c
cTaskEffectBird::cTaskEffectBird(const Vector4& pos) : m_vPos(pos)
{
	SetName("g04::cTaskEffectBird");
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_EFFECT"))   // 0x6c563dae (cParamEffect 0x8019bac8: rows 0..5, column 1)
	{
		m_nMin = (uint)csv.S32(4, 1, 0);
		m_nRange = (uint)csv.S32(5, 1, 1);
	}
}

// DOL 0x801a8be0
bool cTaskEffectBird::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app != nullptr && (app->IsPause() || app->IsHome())) return true;   // KingSys +0xa40 -> +0x1b4; get_pad1(0) & 0x400
	if (m_nWait == 0)
	{
		m_Hnd.Generate(0x16u, true, true, true, 0.0f, m_vPos);   // 0x80021a9c: PTK_EFFECT23_TORI01
		std::mt19937 rnd(0x20061206u);                            // 0x801412b4; genrand 0x8014146c (= std::mt19937: init 0x6c078965, matrix 0x9908b0df, tempering 0x9d2c5680 / 0xefc60000 checked)
		const uint r = (uint)rnd();
		m_nWait = (int)((r % (m_nRange != 0 ? m_nRange : 1u) + m_nMin) * 60u);
	}
	m_nWait--;
	return cTaskKingdom::Execute(pParam);
}

// DOL 0x801a8e04 + 0x801a90cc
cTaskEffectRensFlear::cTaskEffectRensFlear(const Vector4& pos) : m_vPos(pos)
{
	SetName("g04::cTaskEffectRensFlear");
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_EFFECT"))   // 0x6c563dae (cParamEffect 0x8019bac8)
	{
		m_nMin = (uint)csv.S32(0, 1, 0);
		m_nRange = (uint)csv.S32(1, 1, 1);
		m_fScale[0] = csv.F32(2, 1, 1.0f);
		m_fScale[1] = csv.F32(3, 1, 1.0f);
	}
}

// DOL 0x801a9064
void cTaskEffectRensFlear::Start()
{
	const uint r = (uint)m_Rand();
	m_nOn = 1;
	m_nWait = (int)((r % 20u + 10u) * 60u);
}

// DOL 0x801a8ee0
bool cTaskEffectRensFlear::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app != nullptr && (app->IsPause() || app->IsHome())) return true;   // KingSys +0xa40 -> +0x1b4; get_pad1(0) & 0x400
	if (m_nOn == 1)
	{
		if (m_nWait == 0)
		{
			const float t = (float)(1000u - (uint)m_Rand() % 1000u) / 1000.0f;   // FLOAT_8047cc70
			const float sc = (1.0f - t) * m_fScale[0] + t * m_fScale[1];
			m_Hnd.Generate(0x10u, true, true, true, 0.0f, m_vPos, nullptr, Vector4(sc, sc, sc, 1.0f), 0, -1);   // 0x80021a9c
			const uint r = (uint)m_Rand();
			m_nWait = (int)((r % (m_nRange != 0 ? m_nRange : 1u) + m_nMin) * 60u);
		}
		else
			m_nWait--;
	}
	return cTaskKingdom::Execute(pParam);
}

}
