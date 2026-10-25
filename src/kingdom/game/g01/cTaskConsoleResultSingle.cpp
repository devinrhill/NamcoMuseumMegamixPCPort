#include "kingdom/game/g01/cTaskConsoleResultSingle.hpp"

#include <cstdio>
#include <cstdlib>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "snd/Mixer.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "kingdom/game/g01/cTaskConsoleLife.hpp"
#include "kingdom/game/g01/cTaskConsolePlayer.hpp"

namespace kingdom::game::g01
{

namespace
{
// rows {appear, wait, out, -1, roulette}; index by the anita's column (0 appear, 1 wait, 2 out, 4 roulette)
const int DIGIT[10][5] = {{0, 1, 2, -1, 39},    {3, 4, 5, -1, 39},    {6, 7, 8, -1, 39},    {9, 10, 11, -1, 39},  {12, 13, 14, -1, 39},
                          {15, 16, 17, -1, 39}, {18, 19, 20, -1, 39}, {21, 22, 23, -1, 39}, {24, 25, 26, -1, 39}, {27, 28, 29, -1, 39}};   // DOL 0x80372468
const int FRAME_ANM[5] = {111, 112, 113, -1, -1};                         // DOL 0x80372370 anita 0
const int TITLE_ANM[2][5] = {{94, 95, 96, -1, -1}, {97, 98, 99, -1, -1}}; // DOL 0x80372388 anita 1 (by +0x224)
const int KILL_HEAD_ANM[5] = {73, 74, 75, -1, -1};                        // DOL 0x803725f8 anita 2
const int SCORE_HEAD_ANM[5] = {85, 86, 87, -1, -1};                       // DOL 0x8037260c anita 10
const int KILL_LABEL_ANM[5] = {76, 77, 78, -1, -1};                       // DOL 0x80372620 anita 3
const int LIFE_HEAD_ANM[2][5] = {{82, 83, 84, -1, -1}, {91, 92, 93, -1, -1}};   // DOL 0x80372638 anita 0x16
const int RATE_HEAD_ANM[5] = {79, 80, 81, -1, -1};                        // DOL 0x80372660 anita 0x11
const int LIFE_BASE_ANM[2][5] = {{52, 64, 54, 53, -1}, {52, 65, 56, 55, -1}};   // DOL 0x803723b0 anita 0x17 (ANT_LIFE)
const int LIFE_ICON_ANM[7][5] = {{5, 57, 6, -1, -1},   {12, 58, 13, -1, -1}, {19, 59, 20, -1, -1}, {26, 60, 27, -1, -1},
                                 {33, 61, 34, -1, -1}, {40, 62, 41, -1, -1}, {47, 63, 48, -1, -1}};   // DOL 0x803723d8 anita 0x18 by life
const int PERCENT_ANM[3] = {33, 34, 35};   // DOL 0x80372688 anita 0x1b (kinds 0, 1 / 4, 2)
const int SLASH_ANM[3] = {30, 31, 32};     // DOL 0x8037269c anita 0x19
const int POINT_ANM[3] = {36, 37, 38};     // DOL 0x803726b0 anita 0x1a
const uint SCORE_LAYER[6] = {18, 17, 16, 15, 14, 13};   // DOL 0x803726d8 anitas 0xb..0x10
const uint KILL_LAYER[3] = {2, 1, 0};                   // DOL 0x803726f0 anitas 4..6
const uint MAX_LAYER[3] = {6, 5, 4};                    // DOL 0x803726fc anitas 7..9
const uint RATE_LAYER[4] = {11, 9, 8, 7};               // DOL 0x80372708 anitas 0x12..0x15
}

// the console base's "anim over" test used everywhere: hidden (flag 1 off) or ANIM_END (0x10000)
static bool Done(cKingdomAnita* a) { return !a->IsState(1u) || a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END); }

// DOL 0x800d7c40 (priority 0x80000004, +0x224 = 0) + Init 0x800d7f78 (console player, life console, the stage's enemy count, the anitas
// hidden at their layers)
cTaskConsoleResultSingle::cTaskConsoleResultSingle(cTaskConsolePlayer* player, cTaskConsoleLife* life)
{
	SetName("g01::cTaskConsoleResultSingle");
	nDrawPriority(0x80000004u);
	m_b08 = true;   // started at once (0x800eb3b0 from 0x800d7c40: state 0, +8 = 1)
	m_pPlayer = player;
	m_pLife = life;
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	m_nMaxKill = (sys->nStage() == 10 ? sys->pScoreNorma(9) : sys->pScoreNorma())->nMaxKill1;
	auto load = [this](uint n, const char* name) {
		cKingdomAnita* a = pAnita(n);
		a->RegistData(name);
		a->SetState(1u, false);
	};
	for (uint n : {0u, 1u, 2u, 3u, 10u, 0x11u, 0x16u}) load(n, "ANT_RESULT");
	// 0x17: the DOL asks anita 0x18 (not loaded yet) for the layer, so 0x17 stays at the origin
	load(0x17, "ANT_LIFE");
	load(0x18, "ANT_LIFE");
	pAnita(0x18)->GetLayerPos(pAnita(0x18)->vTrans(), 0, 1);
	auto place = [this, &load](uint n, uint layer) {
		load(n, "ANT_RESULT");
		pAnita(n)->GetLayerPos(pAnita(n)->vTrans(), layer, 0);   // 0x80014408
	};
	for (uint i = 0; i < 3; i++) place(4 + i, KILL_LAYER[i]);
	for (uint i = 0; i < 3; i++) place(7 + i, MAX_LAYER[i]);
	for (uint i = 0; i < 4; i++) place(0x12 + i, RATE_LAYER[i]);
	for (uint i = 0; i < 6; i++) place(0xb + i, SCORE_LAYER[i]);
	place(0x1a, 10);
	place(0x19, 3);
	place(0x1b, 12);
}

// DOL 0x800dfcb0 -> 0x800eb6c4
void cTaskConsoleResultSingle::Start()
{
	m_nState = 4;
	SetPhase(0);
}

// value digit by digit over `count` anitas from `first` (lowest digit first), column `col`; a leading zero is hidden from digit `keep`
// on (the loops of 0x800d8e74 / 0x800d914c / 0x800d999c)
static void Digits(cTaskAnita* t, uint first, uint count, uint value, int col, uint keep)
{
	uint div = 1;
	for (uint i = 0; i < count; i++, div *= 10)
	{
		const uint d = (value % (div * 10)) / div;
		cKingdomAnita* a = t->pAnita(first + i);
		if (value < div && i >= keep && d == 0)
			a->SetState(1u, false);
		else
		{
			a->SetAnimNo(DIGIT[d][col], false);
			a->SetState(1u, true);
		}
	}
}

// the roulette: the spinning digit anim at a random frame 0..9 (rand % 10 into anita +0xc, then 0x80141c44 refreshes)
static void Roulette(cKingdomAnita* a)
{
	a->SetAnimNo(DIGIT[0][4], false);
	a->SetFrame((float)(std::rand() % 10));
}

// DOL 0x800d8908: the headers (title, KILL, its label, HIT RATE, LIFE, SCORE); kind 0 appear (and show), 2 out
void cTaskConsoleResultSingle::Headers(int kind)
{
	if (kind != 0 && kind != 2) return;
	pAnita(1)->SetAnimNo(TITLE_ANM[m_nType][kind], false);
	pAnita(2)->SetAnimNo(KILL_HEAD_ANM[kind], false);
	pAnita(3)->SetAnimNo(KILL_LABEL_ANM[kind], false);
	pAnita(0x11)->SetAnimNo(RATE_HEAD_ANM[kind], false);
	pAnita(0x16)->SetAnimNo(LIFE_HEAD_ANM[m_nType][kind], false);
	pAnita(10)->SetAnimNo(SCORE_HEAD_ANM[kind], false);
	if (kind == 0)
		for (uint n : {1u, 2u, 3u, 0x11u, 0x16u, 10u}) pAnita(n)->SetState(1u, true);
}

// DOL 0x800d8e74: the shown score (+0x23c) on anitas 0xb..0x10; kind 1 wait, 2 out
void cTaskConsoleResultSingle::Score(int kind)
{
	if (kind == 1 || kind == 2) Digits(this, 0xb, 6, (uint)m_nScoreShown, kind, 1);
}

// DOL 0x800d914c: the hit rate (anitas 0x12..0x15, "." 0x1a, "%" 0x1b); 0 zeros appear, 4 roulette, 1 the rate, 2 out
void cTaskConsoleResultSingle::Rate(int kind)
{
	if (kind == 0 || kind == 4)
	{
		for (uint n = 0x12; n < 0x16; n++)
		{
			if (kind == 0) pAnita(n)->SetAnimNo(DIGIT[0][0], false);
			else Roulette(pAnita(n));
			pAnita(n)->SetState(1u, true);
		}
		pAnita(0x1b)->SetAnimNo(PERCENT_ANM[kind == 0 ? 0 : 1], false);
		pAnita(0x1b)->SetState(1u, true);
		pAnita(0x1a)->SetAnimNo(POINT_ANM[kind == 0 ? 0 : 1], false);
		pAnita(0x1a)->SetState(1u, true);
		return;
	}
	if (kind != 1 && kind != 2) return;
	// player hits +0x238 / shots +0x23c (0 without shots); the carried rate +0x240 (stage 10: stage 9's, -1 = none): 0.5 * (rate + it / 1000)
	// (FLOAT_8047b4d0 / FLOAT_8047b4cc)
	float rate = 0.0f;   // FLOAT_8047b4c4
	if (m_pPlayer != nullptr && m_pPlayer->nShot() != 0) rate = (float)m_pPlayer->nHit() / (float)m_pPlayer->nShot();
	if (m_pPlayer != nullptr && m_pPlayer->nRateCarry() != -1) rate = 0.5f * (rate + (float)m_pPlayer->nRateCarry() / 1000.0f);
	Digits(this, 0x12, 4, (uint)(1000.0f * rate), kind, 2);   // FLOAT_8047b4cc: per mille, "0.0" always shown
	pAnita(0x1b)->SetAnimNo(PERCENT_ANM[kind], false);
	pAnita(0x1a)->SetAnimNo(POINT_ANM[kind], false);
	if (kind == 1)
	{
		pAnita(0x1b)->SetState(1u, true);
		pAnita(0x1a)->SetState(1u, true);
	}
}

// DOL 0x800d999c: kills (anitas 4..6) "/" (0x19) the stage's enemies (7..9, +0x244); 0 appear (kills "000"), 4 roulette, 1 the kills, 2 out
void cTaskConsoleResultSingle::Kill(int kind)
{
	switch (kind)
	{
	case 0:
		Digits(this, 7, 3, (uint)m_nMaxKill, 0, 1);
		for (uint n = 4; n < 7; n++)
		{
			pAnita(n)->SetAnimNo(DIGIT[0][0], false);
			pAnita(n)->SetState(1u, true);
		}
		pAnita(0x19)->SetAnimNo(SLASH_ANM[0], false);
		pAnita(0x19)->SetState(1u, true);
		break;
	case 4:
		for (uint n = 4; n < 7; n++) Roulette(pAnita(n));
		pAnita(0x19)->SetAnimNo(SLASH_ANM[1], false);
		break;
	case 1:
	case 2:
		Digits(this, 7, 3, (uint)m_nMaxKill, kind, 1);
		Digits(this, 4, 3, m_pPlayer != nullptr ? (uint)m_pPlayer->nKill() : 0u, kind, 1);   // player +0x234
		pAnita(0x19)->SetAnimNo(SLASH_ANM[kind], false);
		if (kind == 1) pAnita(0x19)->SetState(1u, true);
		break;
	}
}

// DOL 0x800da394
bool cTaskConsoleResultSingle::IsKillEnd()
{
	for (uint n = 7; n < 10; n++) if (!Done(pAnita(n))) return false;
	for (uint n = 4; n < 7; n++) if (!Done(pAnita(n))) return false;
	return Done(pAnita(0x19));
}

// DOL 0x800da4a0. Phases: 0 frame + jingle, 1 title / KILL header, 2 kills (roulette 75 frames), 3 hit rate (the same), 4 life, 5 score count
// up, 6 skip (A / B / Z in 2..5: everything final at once), 7 wait (> 2 s or A / B / Z), 8 out; 1 = done
bool cTaskConsoleResultSingle::PhaseExec()
{
	if ((GamePad::Get(0).trig & (GamePad::PAD_A | GamePad::PAD_B | GamePad::PAD_Z)) != 0)   // get_pad1(0)->trig & 0x860
	{
		if ((unsigned)(m_nPhase - 2) < 4) SetPhase(6);
		else if (m_nPhase == 7) SetPhase(8);
	}
	const int life = m_pLife != nullptr ? m_pLife->nLife() : 0;   // life console +0x224
	switch (m_nPhase)
	{
	case 0:
		if (m_nStep == 0)
		{
			snd::Play("m01se_15clear");   // DAT_8047b4c0 on a cGalagaSndHnd, detached
			pAnita(0)->SetAnimNo(FRAME_ANM[0], false);
			pAnita(0)->SetState(1u, true);
			m_nStep++;
		}
		else if (m_nStep == 1 && Done(pAnita(0)))
		{
			pAnita(0)->SetAnimNo(FRAME_ANM[1], false);
			SetPhase(1);
		}
		break;
	case 1:
		if (m_nStep == 0)
		{
			pAnita(1)->SetAnimNo(TITLE_ANM[m_nType][0], false);
			pAnita(1)->SetState(1u, true);
			pAnita(2)->SetAnimNo(KILL_HEAD_ANM[0], false);
			pAnita(2)->SetState(1u, true);
			SetPhase(2);
		}
		break;
	case 2:
		if (m_nStep == 0)
		{
			if (Done(pAnita(1)) && Done(pAnita(2)))
			{
				pAnita(3)->SetAnimNo(KILL_LABEL_ANM[0], false);
				pAnita(3)->SetState(1u, true);
				Kill(0);
				m_nStep++;
			}
		}
		else if (m_nStep == 1)
		{
			if (Done(pAnita(3)) && IsKillEnd())
			{
				Kill(4);
				m_fTimer = 0.0f;
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			if (m_fTimer <= 75.0f) m_fTimer += 1.0f;   // FLOAT_8047b4e8 / 4ec
			else
			{
				Kill(1);
				SetPhase(3);
			}
		}
		break;
	case 3:
		if (m_nStep == 0)
		{
			pAnita(0x11)->SetAnimNo(RATE_HEAD_ANM[0], false);
			pAnita(0x11)->SetState(1u, true);
			Rate(0);
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			Rate(4);
			m_fTimer = 0.0f;
			m_nStep++;
		}
		else if (m_nStep == 2)
		{
			if (m_fTimer <= 75.0f) m_fTimer += 1.0f;
			else
			{
				Rate(1);
				SetPhase(4);
			}
		}
		break;
	case 4:
		if (m_nStep == 0)
		{
			pAnita(0x16)->SetAnimNo(LIFE_HEAD_ANM[m_nType][0], false);
			pAnita(0x16)->SetState(1u, true);
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			if (Done(pAnita(0x16)))
			{
				pAnita(0x16)->SetAnimNo(LIFE_HEAD_ANM[m_nType][1], false);
				pAnita(0x16)->SetState(1u, true);
				pAnita(0x17)->SetAnimNo(LIFE_BASE_ANM[m_nType][3], false);
				pAnita(0x17)->SetState(1u, true);
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			if (Done(pAnita(0x17)))
			{
				pAnita(0x18)->SetAnimNo(LIFE_ICON_ANM[life][0], false);
				pAnita(0x18)->SetState(1u, true);
				m_nStep++;
			}
		}
		else if (m_nStep == 3 && Done(pAnita(0x18)))
			SetPhase(5);
		break;
	case 5:
		if (m_nStep == 0)
		{
			// the player's shown score (+0x244) + the life console's bonus +0x228
			m_nScoreTotal = (m_pPlayer != nullptr ? m_pPlayer->nScore() : 0) + (m_pLife != nullptr ? m_pLife->nBonus() : 0);
			pAnita(10)->SetAnimNo(SCORE_HEAD_ANM[0], false);
			pAnita(10)->SetState(1u, true);
			m_nStep++;
		}
		else if (m_nStep == 1 && Done(pAnita(10)))
		{
			const int diff = m_nScoreTotal - m_nScoreShown;
			if (diff > 10000) m_nScoreShown += 0x563;
			if (diff > 1000) m_nScoreShown += 0x89;
			else if (diff > 100) m_nScoreShown += 0xd;
			else if (diff > 20) m_nScoreShown += 3;
			else m_nScoreShown += 1;
			Score(1);
			if (m_nScoreShown == m_nScoreTotal) SetPhase(7);
		}
		break;
	case 6:
		pAnita(3)->SetState(1u, true);
		pAnita(3)->SetAnimNo(KILL_LABEL_ANM[1], false);
		Kill(1);
		pAnita(0x11)->SetState(1u, true);
		pAnita(0x11)->SetAnimNo(RATE_HEAD_ANM[1], false);
		Rate(1);
		pAnita(0x16)->SetState(1u, true);
		pAnita(0x16)->SetAnimNo(LIFE_HEAD_ANM[m_nType][1], false);
		pAnita(0x17)->SetState(1u, true);
		pAnita(0x17)->SetAnimNo(LIFE_BASE_ANM[m_nType][1], false);
		pAnita(0x18)->SetState(1u, true);
		pAnita(0x18)->SetAnimNo(LIFE_ICON_ANM[life][1], false);
		pAnita(10)->SetState(1u, true);
		pAnita(10)->SetAnimNo(SCORE_HEAD_ANM[1], false);
		m_nScoreTotal = (m_pPlayer != nullptr ? m_pPlayer->nScore() : 0) + (m_pLife != nullptr ? m_pLife->nBonus() : 0);
		m_nScoreShown = m_nScoreTotal;
		Score(1);
		SetPhase(7);
		break;
	case 7:
		// OSGetTick: (now - stamp) / (bus clock / 4) > 2, i.e. 3 whole seconds = 180 frames
		if (m_nStep == 0)
		{
			m_nWaitFrame = 0;
			m_nStep++;
		}
		else if (m_nStep == 1 && ++m_nWaitFrame >= 180)
			SetPhase(8);
		break;
	case 8:
		if (m_nStep == 0)
		{
			Headers(2);
			Kill(2);
			Score(2);
			Rate(2);
			pAnita(0)->SetAnimNo(FRAME_ANM[2], false);
			pAnita(0x17)->SetAnimNo(LIFE_BASE_ANM[m_nType][2], false);
			pAnita(0x18)->SetAnimNo(LIFE_ICON_ANM[life][2], false);
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			for (uint n : {1u, 2u, 3u, 0x11u, 0x16u, 10u}) if (!Done(pAnita(n))) return false;
			if (!IsKillEnd()) return false;
			for (uint n = 0xb; n < 0x11; n++) if (!Done(pAnita(n))) return false;
			for (uint n = 0x12; n < 0x16; n++) if (!Done(pAnita(n))) return false;
			for (uint n : {0x1bu, 0x1au, 0u, 0x17u, 0x18u}) if (!Done(pAnita(n))) return false;
			return true;
		}
		break;
	}
	return false;
}

// DOL 0x800eb400: in state 4 (step 0) the phase machine; done -> state 5
bool cTaskConsoleResultSingle::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	m_nDbgFrame++;
	if (std::getenv("MEGAMIX_G01DBG") && m_nDbgFrame % 600 == 1) std::fprintf(stderr, "g01 result exec state %d phase %d step %d\n", m_nState, m_nPhase, m_nStep);
	if (m_nState == 4 && PhaseExec()) m_nState = 5;
	if (std::getenv("MEGAMIX_G01DBG") && m_nPhase * 16 + m_nStep != m_nDbg) { m_nDbg = m_nPhase * 16 + m_nStep; std::fprintf(stderr, "g01 result phase %d step %d state %d (task frame %d)\n", m_nPhase, m_nStep, m_nState, m_nDbgFrame); }
	return cTaskAnita::Execute(pParam);
}


// DOL 0x800eb548 (cTaskGameConsoleBase): only while active (+8) and shown (+9), the anitas over a 640 x 480 ortho
void cTaskConsoleResultSingle::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskAnita::Draw(nStatus, pParam);   // FUN_80014df8
}

}
