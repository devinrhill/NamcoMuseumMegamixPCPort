#include "kingdom/game/g01/cTaskConsoleResultMulti.hpp"

#include <cstdint>
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
// DOL data 0x80372360.. (word per entry): the anim tables below are read straight from it, as the DOL does (rows {appear, wait, out, -1, -1})
const int g_data[] = {
#include "ResultMultiData.inc"
};
constexpr uint32_t DATA_BASE = 0x80372360u;
int D(uint32_t addr) { return g_data[(addr - DATA_BASE) / 4]; }

// rows {appear, wait, out, -1, roulette}: DOL 0x80372468 (digit * 5 + column)
int Digit(int d, int col) { return col == 4 ? 39 : d * 3 + col; }

constexpr int TYPE = 1;   // +0x224 / +0x218 of the cResultAnita: multiplayer

// the console base's "anim over" test: hidden (flag 1 off) or ANIM_END (0x10000)
bool Done(cKingdomAnita* a) { return !a->IsState(1u) || a->IsState(cJetAnita::JET_ANITA_STATE_ANIM_END); }

void Show(cKingdomAnita* a) { a->SetState(1u, true); }

// value digit by digit over `count` anitas from `first` (lowest digit first), column `col`; a leading zero is hidden from digit `keep` on
void Digits(cTaskAnita* t, uint first, uint count, uint value, int col, uint keep)
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
			a->SetAnimNo(Digit((int)d, col), false);
			Show(a);
		}
	}
}

void Roulette(cKingdomAnita* a)
{
	a->SetAnimNo(Digit(0, 4), false);
	a->SetFrame((float)(std::rand() % 10));
}

void Load(cTaskAnita* t, uint n, const char* name)
{
	cKingdomAnita* a = t->pAnita(n);
	a->RegistData(name);
	a->SetState(1u, false);
}

void Place(cTaskAnita* t, uint n, uint layer, int pat)
{
	t->pAnita(n)->GetLayerPos(t->pAnita(n)->vTrans(), layer, pat);   // 0x80014408
}

// DOL 0x800dc9ec / 0x800dcbb0: the player's hit ratio (hits / shots, 0 without shots; stage 10: averaged with the carried rate)
float HitRatio(const cTaskConsolePlayer* p)
{
	float r = 0.0f;
	if (p->nShot() != 0) r = (float)p->nHit() / (float)p->nShot();
	if (p->nRateCarry() != -1) r = 0.5f * (r + (float)p->nRateCarry() / 1000.0f);
	return r;
}
}

// ---- cTaskResultPlayerAnita ----

// DOL 0x800db828 (priority 0x80000005, state 4 = idle, +8 = 1)
cTaskResultPlayerAnita::cTaskResultPlayerAnita(cTaskConsolePlayer* player, int nPlayers)
{
	SetName("g01::cTaskPlayerAnita");
	nDrawPriority(0x80000005u);
	m_b08 = true;
	m_pPlayer = player;
	m_nState = 4;
	m_nStep = 0;
	// the panel's offset: the layer of anita 0xf (pattern 2) from the table 0x80372740 [(players - 2) * 4 + player]
	Load(this, 0xf, "ANT_RESULT");
	Vector4 off{};
	pAnita(0xf)->GetLayerPos(off, (uint)D(0x80372740u + (uint32_t)(((nPlayers - 2) * 4 + player->nPlayer()) * 4)), 2);
	auto place = [&](uint n, uint layer) {
		Load(this, n, "ANT_RESULT");
		Place(this, n, layer, 1);
		Vector4& v = pAnita(n)->vTrans();
		v.x += off.x;
		v.y += off.y;
		v.z += off.z;
	};
	place(0, 7);
	pAnita(0)->SetAnimNo(D(0x80372770u + (uint32_t)(player->nPlayer() * 0x14)), true);   // 0x800dc054: the bar frame of the player (0x64 + 2 * player)
	pAnita(0)->SetState(2u, false);
	place(5, 4);
	place(6, 6);
	place(7, 0);
	for (uint i = 0; i < 4; i++) place(1 + i, (uint)D(0x80372730u + i * 4));
}

void cTaskResultPlayerAnita::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskAnita::Draw(nStatus, pParam);
}

float cTaskResultPlayerAnita::Ratio() const { return HitRatio(m_pPlayer); }

// DOL 0x800dc0a8: the hit rate (anitas 1..4 digits, 6 "%", 5 "."); 0 the first two digits appear, 4 roulette, 1 the rate, 2 out
void cTaskResultPlayerAnita::Rate(int kind)
{
	static const int PERCENT[3] = {D(0x80372688u), D(0x8037268cu), D(0x80372690u)};   // "%" appear / wait / out
	static const int POINT[3] = {D(0x803726b0u), D(0x803726b4u), D(0x803726b8u)};      // "." appear / wait / out
	if (kind == 0)
	{
		for (uint n = 1; n < 3; n++)
		{
			pAnita(n)->SetAnimNo(Digit(0, 0), false);
			Show(pAnita(n));
		}
	}
	else if (kind == 4)
	{
		for (uint n = 1; n < 5; n++)
		{
			Roulette(pAnita(n));
			Show(pAnita(n));
		}
	}
	else if (kind == 1 || kind == 2)
		Digits(this, 1, 4, (uint)(1000.0f * Ratio()), kind, 2);   // "0.0" always shown
	else
		return;
	const int col = kind == 0 ? 0 : (kind == 2 ? 2 : 1);
	pAnita(6)->SetAnimNo(PERCENT[col], false);
	pAnita(5)->SetAnimNo(POINT[col], false);
	if (kind != 2)
	{
		Show(pAnita(6));
		Show(pAnita(5));
	}
}

// DOL 0x800dc8f8
bool cTaskResultPlayerAnita::IsRateEnd()
{
	for (uint n = 1; n < 5; n++)
		if (!Done(pAnita(n))) return false;
	return Done(pAnita(6)) && Done(pAnita(5));
}

// DOL 0x800dc9ec: the bar grows 2 frames per frame up to 100 * the hit ratio; true once it ran its 100 frames
bool cTaskResultPlayerAnita::Gauge()
{
	m_fBar += 2.0f;   // FLOAT_8047b4f0
	const float target = 100.0f * Ratio();   // FLOAT_8047b4f4
	pAnita(0)->SetFrame(m_fBar <= target ? m_fBar : target);
	return 100.0f <= m_fBar;
}

// DOL 0x800dcbb0 (vtable +0x94): the shown state at once
void cTaskResultPlayerAnita::Instant(bool winner)
{
	Show(pAnita(0));
	pAnita(0)->SetFrame(100.0f * Ratio());
	Rate(1);
	m_bWinner = winner;
	if (winner)
	{
		Show(pAnita(7));
		const int a = D(0x80372678u);
		if (a >= 0) pAnita(7)->SetAnimNo(a, true);
	}
	Idle();   // 0x800dcb38
}

// DOL 0x800dcd38. States 0 appear (bar, roulette 75 frames, the rate, the crown), 1 shown, 2 out, 3 out done, 4 idle
bool cTaskResultPlayerAnita::Execute(cJetTask* pParam)
{
	if (m_nState == 0)
	{
		if (m_nStep == 0)
		{
			Rate(0);
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			Show(pAnita(0));
			if (Gauge())
			{
				Rate(4);
				m_fTimer = 0.0f;
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			if (m_fTimer <= 75.0f) m_fTimer += 1.0f;   // FLOAT_8047b4e8 / 4ec
			else
			{
				Rate(1);
				if (!m_bWinner)
				{
					m_nState = 1;
					m_nStep = 0;
				}
				else
				{
					Show(pAnita(7));
					const int a = D(0x80372674u);
					if (a >= 0) pAnita(7)->SetAnimNo(a, true);
					m_nStep++;
				}
			}
		}
		else if (m_nStep == 3 && Done(pAnita(7)))
		{
			const int a = D(0x80372678u);
			if (a >= 0) pAnita(7)->SetAnimNo(a, true);
			m_nState = 1;
			m_nStep = 0;
		}
	}
	else if (m_nState == 2)
	{
		if (m_nStep == 0)
		{
			pAnita(0)->SetState(1u, false);
			const int a = D(0x8037267cu);
			if (a >= 0) pAnita(7)->SetAnimNo(a, true);
			Rate(2);
			m_nStep++;
		}
		else if (m_nStep == 1 && IsRateEnd())
		{
			pAnita(7)->SetState(1u, false);
			m_nState = 3;
			m_nStep = 0;
		}
	}
	return cTaskAnita::Execute(pParam);
}

// ---- cTaskConsoleResultMulti ----

// DOL 0x800dd06c (priority 0x80000004, +0x224 = 1) + Init 0x800dd444
cTaskConsoleResultMulti::cTaskConsoleResultMulti(cTaskConsolePlayer** players, int nPlayers, cTaskConsoleLife* life)
{
	SetName("g01::cTaskConsoleResultMulti");
	nDrawPriority(0x80000004u);
	m_b08 = true;   // started at once (0x800eb3b0: state 0, +8 = 1)
	m_pLife = life;
	m_nPlayers = nPlayers;
	cG01GalagaSys* sys = KingdomGlobal::GalagaSys;
	const sScoreNorma* norma = sys->nStage() == 10 ? sys->pScoreNorma(9) : sys->pScoreNorma();
	m_nNorma = (&norma->nMaxKill1)[nPlayers - 1];   // word [players - 1] of the ScoreNorma row (MaxKill1..4)
	for (int i = 0; i < nPlayers && i < 4; i++)
	{
		m_pPanel[i] = new cTaskResultPlayerAnita(players[i], nPlayers);
		ConnectChild(m_pPanel[i]);
	}
	SumKills();
	for (uint n : {0u, 1u, 2u, 3u, 10u}) Load(this, n, "ANT_RESULT");
	Load(this, 0xb, "ANT_LIFE");
	Load(this, 0xc, "ANT_LIFE");
	Place(this, 0xc, 1, 1);
	for (uint i = 0; i < 3; i++)
	{
		Load(this, 4 + i, "ANT_RESULT");
		Place(this, 4 + i, (uint)D(0x80372718u + i * 4), 0);   // team kills
	}
	for (uint i = 0; i < 3; i++)
	{
		Load(this, 7 + i, "ANT_RESULT");
		Place(this, 7 + i, (uint)D(0x80372724u + i * 4), 0);   // norma
	}
	Load(this, 0xd, "ANT_RESULT");
	Place(this, 0xd, 3, 0);
	Load(this, 0xe, "ANT_RESULT");
}

// DOL 0x800dd3b4 (vtable +0x94)
void cTaskConsoleResultMulti::SumKills()
{
	m_nKills = 0;
	for (int i = 0; i < m_nPlayers && i < 4; i++) m_nKills += m_pPanel[i]->pPlayer()->nKill();   // player +0x234
}

// DOL 0x800dfc48: console state 4 (0x800eb6c4), phase 0, SumKills
void cTaskConsoleResultMulti::Start()
{
	m_nState = 4;
	SetPhase(0);
	SumKills();
	m_b08 = true;
}

// DOL 0x800ddb3c (vtable +0x98): the kill counters; 0 appear, 4 roulette, 1 the kills, 2 out
void cTaskConsoleResultMulti::Kill(int kind)
{
	const int slash[3] = {D(0x8037269cu), D(0x803726a0u), D(0x803726a4u)};
	switch (kind)
	{
	case 0:
		Digits(this, 7, 3, (uint)m_nNorma, 0, 1);
		for (uint n = 4; n < 7; n++)
		{
			pAnita(n)->SetAnimNo(Digit(0, 0), false);
			Show(pAnita(n));
		}
		pAnita(0xd)->SetAnimNo(slash[0], false);
		Show(pAnita(0xd));
		break;
	case 4:
		for (uint n = 4; n < 7; n++) Roulette(pAnita(n));
		pAnita(0xd)->SetAnimNo(slash[1], false);
		break;
	case 1:
	case 2:
		Digits(this, 7, 3, (uint)m_nNorma, kind, 1);
		Digits(this, 4, 3, (uint)m_nKills, kind, 1);
		pAnita(0xd)->SetAnimNo(slash[kind], false);
		if (kind == 1) Show(pAnita(0xd));
		break;
	}
}

// DOL 0x800de52c (vtable +0x9c)
bool cTaskConsoleResultMulti::IsKillEnd()
{
	for (uint n = 7; n < 10; n++) if (!Done(pAnita(n))) return false;
	for (uint n = 4; n < 7; n++) if (!Done(pAnita(n))) return false;
	return Done(pAnita(0xd));
}

// DOL 0x800de638
bool cTaskConsoleResultMulti::PhaseExec()
{
	if ((GamePad::Get(0).trig & (GamePad::PAD_A | GamePad::PAD_B | GamePad::PAD_Z)) != 0)   // get_pad1(0)->trig & 0x860
	{
		if ((unsigned)(m_nPhase - 1) < 4) SetPhase(5);
		else if (m_nPhase == 6) SetPhase(7);
	}
	const int life = m_pLife != nullptr ? m_pLife->nLife() : 0;   // life console +0x224
	const int FRAME[3] = {D(0x80372370u), D(0x80372374u), D(0x80372378u)};
	const int TITLE[3] = {D(0x80372388u + TYPE * 20), D(0x8037238cu + TYPE * 20), D(0x80372390u + TYPE * 20)};
	const int KHEAD[3] = {D(0x803725f8u), D(0x803725fcu), D(0x80372600u)};
	const int KLABEL[3] = {D(0x80372620u), D(0x80372624u), D(0x80372628u)};
	const int LHEAD[3] = {D(0x80372638u + TYPE * 20), D(0x8037263cu + TYPE * 20), D(0x80372640u + TYPE * 20)};
	const int LBASE[4] = {D(0x803723b0u + TYPE * 20), D(0x803723b4u + TYPE * 20), D(0x803723b8u + TYPE * 20), D(0x803723bcu + TYPE * 20)};
	const int LICON[3] = {D(0x803723d8u + (uint32_t)life * 20), D(0x803723dcu + (uint32_t)life * 20), D(0x803723e0u + (uint32_t)life * 20)};
	const int BANNER[3] = {D(0x803726c4u), D(0x803726c8u), D(0x803726ccu)};
	auto set = [this](uint n, int anim) { if (anim >= 0) pAnita(n)->SetAnimNo(anim, false); };
	switch (m_nPhase)
	{
	case 0:
		if (m_nStep == 0)
		{
			m_nWinMask = 0;
			int best = 0;
			for (int i = 0; i < m_nPlayers && i < 4; i++)
			{
				const int kills = m_pPanel[i]->pPlayer()->nKill();
				if (kills == best) m_nWinMask |= 1u << i;
				else if (best < kills)
				{
					m_nWinMask = 1u << i;
					best = kills;
				}
			}
			snd::Play("m01se_15clear");   // DAT_8047b4c0
			set(0, FRAME[0]);
			Show(pAnita(0));
			m_nStep++;
		}
		else if (m_nStep == 1 && Done(pAnita(0)))
		{
			set(0, FRAME[1]);
			SetPhase(1);
		}
		break;
	case 1:
		if (m_nStep == 0)
		{
			set(1, TITLE[0]);
			Show(pAnita(1));
			set(2, KHEAD[0]);
			Show(pAnita(2));
			m_nStep++;
		}
		else if (m_nStep == 1 && Done(pAnita(1)) && Done(pAnita(2)))
		{
			set(1, TITLE[1]);
			set(2, KHEAD[1]);
			SetPhase(2);
		}
		break;
	case 2:
		if (m_nStep == 0)
		{
			set(3, KLABEL[0]);
			Show(pAnita(3));
			Kill(0);
			m_nStep++;
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
			set(10, LHEAD[0]);
			Show(pAnita(10));
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			if (Done(pAnita(10)))
			{
				set(10, LHEAD[1]);
				set(0xb, LBASE[3]);
				Show(pAnita(0xb));
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			if (Done(pAnita(0xb)))
			{
				set(0xc, LICON[0]);
				Show(pAnita(0xc));
				m_nStep++;
			}
		}
		else if (m_nStep == 3 && Done(pAnita(0xc)))
			SetPhase(4);
		break;
	case 4:
		if (m_nStep == 0)
		{
			pAnita(0xe)->SetAnimNo(BANNER[0], true);
			Show(pAnita(0xe));
			m_nStep++;
		}
		if (m_nStep == 1)
		{
			if (Done(pAnita(0xe)))
			{
				for (int i = 0; i < m_nPlayers && i < 4; i++)
					m_pPanel[i]->Appear((m_nWinMask & (1u << m_pPanel[i]->pPlayer()->nPlayer())) != 0);
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			bool all = true;
			for (int i = 0; i < m_nPlayers && i < 4 && all; i++) all = m_pPanel[i]->IsShown();
			if (all) SetPhase(6);
		}
		break;
	case 5:   // skipped: everything in its shown state
		Show(pAnita(1));
		set(1, TITLE[1]);
		Show(pAnita(2));
		set(2, KHEAD[1]);
		Show(pAnita(3));
		set(3, KLABEL[1]);
		Kill(1);
		Show(pAnita(10));
		set(10, LHEAD[1]);
		Show(pAnita(0xb));
		set(0xb, LBASE[1]);
		Show(pAnita(0xc));
		set(0xc, LICON[1]);
		Show(pAnita(0xe));
		set(0xe, BANNER[1]);
		for (int i = 0; i < m_nPlayers && i < 4; i++)
			m_pPanel[i]->Instant((m_nWinMask & (1u << m_pPanel[i]->pPlayer()->nPlayer())) != 0);
		SetPhase(6);
		break;
	case 6:
		// OSGetTick: (now - stamp) / (bus clock / 4) > 2, i.e. 3 whole seconds = 180 frames
		if (m_nStep == 0)
		{
			m_nWaitFrame = 0;
			m_nStep++;
		}
		else if (m_nStep == 1 && ++m_nWaitFrame >= 180)
			SetPhase(7);
		break;
	case 7:
		if (m_nStep == 0)
		{
			set(0, FRAME[2]);
			set(1, TITLE[2]);
			set(2, KHEAD[2]);
			set(3, KLABEL[2]);
			Kill(2);
			set(10, LHEAD[2]);
			set(0xb, LBASE[2]);
			set(0xc, LICON[2]);
			set(0xe, BANNER[2]);
			for (int i = 0; i < m_nPlayers && i < 4; i++) m_pPanel[i]->GoOut();
			m_bOut = true;
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			for (uint n : {0u, 1u, 2u, 3u, 10u, 0xbu, 0xcu, 0xeu}) if (!Done(pAnita(n))) return false;
			if (!IsKillEnd()) return false;
			for (int i = 0; i < m_nPlayers && i < 4; i++) if (!m_pPanel[i]->IsOut()) return false;
			return true;
		}
		break;
	}
	return false;
}

// DOL 0x800eb400: in state 4 the phase machine; done -> state 5
bool cTaskConsoleResultMulti::Execute(cJetTask* pParam)
{
	if (cG01GalagaSys::IsPause()) return true;   // DAT_80479118: nothing while paused
	if (m_nState == 4 && PhaseExec()) m_nState = 5;
	return cTaskAnita::Execute(pParam);
}

// DOL 0x800eb548 (cTaskGameConsoleBase): only while active (+8) and shown (+9)
void cTaskConsoleResultMulti::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_b08 || !m_b09) return;
	cTaskAnita::Draw(nStatus, pParam);
}

}
