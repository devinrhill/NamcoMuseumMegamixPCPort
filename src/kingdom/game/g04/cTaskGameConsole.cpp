#include "kingdom/game/g04/cTaskGameConsole.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "WiiSnd.hpp"
#include "gfx/FontWriter.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/cTaskConsRetry.hpp"
#include "kingdom/game/g04/cAppG04WaniWaniGame.hpp"
#include "kingdom/game/g04/cG04WaniWaniSys.hpp"
#include "kingdom/game/g04/cTaskPacmanBase.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

namespace kingdom::game::g04
{

namespace
{
constexpr uint ANIM_END = 0x10000u;   // cJetAnita::JET_ANITA_STATE_ANIM_END

bool Paused()
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	return app != nullptr && app->IsPause();
}

void Se(const char* cue)
{
	WiiSnd::Handle h;
	WiiSnd::Play(h, cue);
	WiiSnd::Detach(h);
}

// kingdom FUN_80025b6c with pad 0: the cue on the TV and a copy on Wiimote 1's speaker
void SeRemote(const char* cue)
{
	WiiSnd::Handle h;
	WiiSnd::PlayRemote(h, 0, cue);
	WiiSnd::Detach(h);
}

uint Digit(uint n, int k)
{
	for (int i = 0; i < k; i++) n /= 10u;
	return n % 10u;
}
}

// ---- cTaskScoreNum ----

// DOL 0x801b6788 [audited: checked]
cTaskScoreNum::cTaskScoreNum()
{
	SetName("g04::cTaskScoreNum");
	for (uint i = 0; i < 3; i++)
	{
		cKingdomAnita* a = pAnita(i);
		a->RegistData("GAME_ANITA");   // 0xb16955c5
		a->SetAnimNo(1, true);
		a->SetState(3u, false);
	}
	m_nNum = 0;
	m_bBlink = false;
	for (int& d : m_nDigit) d = 1;
	nDrawPriority(0x80000004u);   // &DAT_80000004
	SetMode(0);
}

// DOL 0x801b688c [audited]
void cTaskScoreNum::Reset()
{
	m_nNum = 0;
	m_bBlink = false;
	for (int& d : m_nDigit) d = 1;
	for (uint i = 0; i < 3; i++)
	{
		pAnita(i)->SetAnimNo(1, true);
		pAnita(i)->SetState(3u, false);
	}
}

// DOL 0x801b691c [audited: table 0x8037d660 = identity so d*3+k]
void cTaskScoreNum::SetMode(int mode)
{
	m_nMode = mode;
	if (mode == 0)
		for (uint i = 0; i < 3; i++) pAnita(i)->SetState(3u, false);
	else if (mode == 2)
	{
		for (uint i = 0; i < 3; i++) pAnita(i)->SetAnimNo((int)(Digit(m_nNum, (int)i) * 3u + 1u), true);   // 0x8037d664
		for (uint i = 0; i < 3; i++) pAnita(i)->SetState(1u, true);
	}
	else if (mode == 1)
		for (uint i = 0; i < 3; i++)
		{
			pAnita(i)->SetAnimNo((int)(Digit(m_nNum, (int)i) * 3u), true);   // 0x8037d660
			pAnita(i)->SetState(3u, true);
		}
	else if (mode == 3)
		for (uint i = 0; i < 3; i++)
		{
			pAnita(i)->SetAnimNo((int)(Digit(m_nNum, (int)i) * 3u + 2u), true);   // 0x8037d668
			pAnita(i)->SetState(2u, true);
		}
}

// DOL 0x801b6c9c [audited]: a changed digit plays "change" (+8, state 2), then "appear" (+4 -> state 1 / 0 ...), then shows (+0)
void cTaskScoreNum::Step()
{
	for (uint i = 0; i < 3; i++)
	{
		cKingdomAnita* a = pAnita(i);
		const uint d = Digit(m_nNum, (int)i);
		if (m_nDigit[i] == 0)
		{
			if (a->IsState(ANIM_END))
			{
				a->SetAnimNo((int)(d * 3u + 1u), true);
				m_nDigit[i] = 1;
			}
		}
		else if (m_nDigit[i] == 1)
			a->SetState(2u, m_bBlink);
		else if (m_nDigit[i] == 2)
		{
			a->SetState(2u, true);
			if (a->IsState(ANIM_END))
			{
				a->SetAnimNo((int)(d * 3u), true);
				a->SetState(3u, true);
				m_nDigit[i] = 0;
			}
		}
	}
}

// DOL 0x801b6ebc [audited: frame reset + unknown helper call after write checked] (mode 4: the new record blinks)
void cTaskScoreNum::StepBlink()
{
	if (!m_bBlink)
	{
		Step();
		return;
	}
	for (uint i = 0; i < 3; i++)
	{
		cKingdomAnita* a = pAnita(i);
		if (m_nDigit[i] == 0)
		{
			if (a->IsState(ANIM_END))
			{
				for (uint k = 0; k < 3; k++)
				{
					m_nDigit[k] = 1;
					pAnita(k)->SetAnimNo((int)(Digit(m_nNum, (int)k) * 3u + 1u), true);
					pAnita(k)->SetFrame(0.0f);
					pAnita(k)->SetState(2u, true);
				}
				break;
			}
		}
		else if (m_nDigit[i] == 2)
		{
			a->SetState(2u, true);
			if (a->IsState(ANIM_END))
			{
				a->SetAnimNo((int)(Digit(m_nNum, (int)i) * 3u), true);
				a->SetState(3u, true);
				m_nDigit[i] = 0;
				a->SetFrame(0.0f);
			}
		}
	}
}

// DOL 0x801b6c04 [audited]
bool cTaskScoreNum::Execute(cJetTask* pParam)
{
	if (Paused() || (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0) return true;
	if (m_nMode == 2) Step();
	else if (m_nMode == 4) StepBlink();
	return cTaskAnita::Execute(pParam);
}

// DOL 0x801b713c [audited] (at most 999)
void cTaskScoreNum::SetNumber(uint n)
{
	if (0x3e6u < n) n = 999u;
	const uint old = m_nNum;
	if (old == n) return;
	m_nNum = n;
	for (uint i = 0; i < 3; i++)
	{
		cKingdomAnita* a = pAnita(i);
		if (Digit(old, (int)i) == Digit(n, (int)i))
			a->SetState(2u, true);
		else
		{
			m_nDigit[i] = 2;
			a->SetState(1u, true);
			a->SetState(2u, true);
			a->SetAnimNo((int)(Digit(old, (int)i) * 3u + 2u), true);   // 0x8037d668: the old digit leaves
		}
	}
}

// DOL 0x801b72e4 [audited]
void cTaskScoreNum::ShowNumber(uint n)
{
	if (0x3e6u < n) n = 999u;
	for (uint i = 0; i < 3; i++)
	{
		cKingdomAnita* a = pAnita(i);
		a->SetState(1u, true);
		a->SetState(2u, false);
		a->SetAnimNo((int)(Digit(n, (int)i) * 3u + 1u), true);
		m_nDigit[i] = 1;
	}
}

// DOL 0x801b7458 [audited]
void cTaskScoreNum::SetPos(uint digit, const Vector4& v)
{
	pAnita(digit)->vTrans() = v;
}

// DOL 0x801b74a0 [audited]
void cTaskScoreNum::SetBlink(bool b, int which)
{
	m_bBlink = b;
	for (uint i = 0; i < 3; i++) pAnita(i)->SetFrame(0.0f);
	for (uint i = 0; i < 3; i++)
	{
		pAnita(i)->SetAnimNo((int)(Digit(m_nNum, (int)i) * 3u + (uint)which), true);
		pAnita(i)->SetState(2u, true);
	}
}

// DOL 0x801b75e0 [audited: written from the decomp]: the mask's state bits cleared / set on the three digits, their frame back to 0 (no clip change)
void cTaskScoreNum::SetStateBits(uint mask, bool set)
{
	for (uint i = 0; i < 3; i++)
	{
		pAnita(i)->SetState(mask, set);
		pAnita(i)->SetFrame(0.0f);
	}
}

// ---- cTaskConsComboScore ----

// DOL 0x8019dc8c [audited]
cTaskConsComboScore::cTaskConsComboScore()
{
	SetName("g04::cTaskConsComboScore");
	for (uint i = 0; i < 3; i++) pAnita(i)->RegistData("GAME_ANITA");
	for (uint i = 0; i < 2; i++)
	{
		cKingdomAnita* a = pAnita(i + 1u);
		a->SetAnimNo(0x24, true);
		a->SetState(3u, false);
		Vector4 v;
		if (a->GetLayerPos(v, i, 4)) a->vTrans() = v;   // 0x80014408(a, a + 0x400, i, 4)
	}
	m_nNum = 0;
	SetMode(0);
}

// DOL 0x8019e060 [audited]
void cTaskConsComboScore::SetMode(int mode)
{
	m_nMode = mode;
	m_nTimer = 0;
	static const int BIG[4] = {0, 0x84, 0x83, 0x85};
	static const int OFS[4] = {0, 1, 0, 2};   // 0x8037c4c4 / 0x8037c4c0 / 0x8037c4c8
	if (mode == 0)
	{
		for (uint i = 0; i < 3; i++) pAnita(i)->SetState(3u, false);
		return;
	}
	if (mode == 1 || mode == 2)
		for (uint i = 0; i < 3; i++) pAnita(i)->SetState(3u, true);
	pAnita(0u)->SetAnimNo(BIG[mode & 3], true);
	pAnita(0u)->SetState(3u, true);
	for (uint i = 0; i < 2; i++) pAnita(i + 1u)->SetAnimNo(0x24 + (int)Digit(m_nNum, (int)i) * 3 + OFS[mode & 3], true);
}

// DOL 0x8019df84 [audited]: shown for 60 frames, then leaves (3)
bool cTaskConsComboScore::Execute(cJetTask* pParam)
{
	if (!Paused())
	{
		if (m_nMode == 1)
		{
			if (0x3bu < ++m_nTimer) SetMode(3);
		}
		else if (m_nMode == 3)
		{
			if (pAnita(0u)->IsState(ANIM_END)) SetMode(0);
		}
		else if (m_nMode == 2 && pAnita(0u)->IsState(ANIM_END))
			SetMode(1);
		return cTaskAnita::Execute(pParam);
	}
	return true;
}

// DOL 0x8019e378 [audited] (2 COMBO appears)
void cTaskConsComboScore::First(uint n)
{
	if (n == 2) SetMode(2);
	if (1u < n)
	{
		m_nNum = n;
		for (uint i = 0; i < 2; i++) pAnita(i + 1u)->SetAnimNo(0x24 + (int)Digit(n, (int)i) * 3, true);
	}
}

// cTaskConsScore::SetMode(7) (0x801a671c) loops cTaskScoreNum::SetMode(3) (0x801b691c) over FIVE pointers from +0x1e8: the fifth is the combo
// (+0x1f8). On this layout that stores 3 at +0x1cc (the combo's timer) and gives anitas 0..2 the "leave" digit clip of +0x1d0 (the combo
// count) through table 0x8037d668 (= d * 3 + 2), state bit 2 set. Kept 1:1; what those clip numbers show in the combo's layout is UNVERIFIED.
void cTaskConsComboScore::ScoreNumMode3()
{
	m_nTimer = 3u;
	for (uint i = 0; i < 3; i++)
	{
		pAnita(i)->SetAnimNo((int)(Digit(m_nNum, (int)i) * 3u + 2u), true);
		pAnita(i)->SetState(2u, true);
	}
}

// DOL 0x8019e444 [audited]
void cTaskConsComboScore::Set(uint n)
{
	if (m_nNum == n)
	{
		SetMode(1);
		return;
	}
	m_nNum = n;
	if (m_nMode == 1)
	{
		for (uint i = 0; i < 3; i++) pAnita(i)->SetState(3u, true);
		m_nTimer = 0;
		for (uint i = 0; i < 2; i++) pAnita(i + 1u)->SetAnimNo(0x24 + (int)Digit(n, (int)i) * 3, true);
	}
	else if (m_nMode == 0 || (uint)(m_nMode - 2) < 2u)
		SetMode(2);
}

// ---- cTaskConsScore ----

// DOL 0x801a6160 [audited: checked (priority set by the builder in the DOL, in the ctor natively)]
cTaskConsScore::cTaskConsScore()
{
	SetName("g04::cTaskConsScore");
	static const int ANIM[4] = {0x6c, 0x72, 0x6f, 0x69};
	for (uint i = 0; i < 4; i++)
	{
		pAnita(i)->RegistData("GAME_ANITA");
		pAnita(i)->SetAnimNo(ANIM[i], true);
	}
	static const int PAT[4] = {1, 2, 3, 0};   // DAT_8037cc38: the digit layers of the board (pattern per number)
	for (uint n = 0; n < 4; n++)
	{
		m_pNum[n] = new cTaskScoreNum();
		for (uint d = 0; d < 3; d++)
		{
			Vector4 v;
			if (pAnita(0u)->GetLayerPos(v, d, PAT[n])) m_pNum[n]->SetPos(d, v);
		}
		ConnectChild(m_pNum[n]);
	}
	m_pCombo = new cTaskConsComboScore();   // +0x1f8
	ConnectChild(m_pCombo);
	Reset();
	for (cTaskScoreNum* p : m_pNum) p->Reset();
	m_pNum[3]->SetNumber((uint)KingdomGlobal::WaniWaniSys->nHighScore());
	nDrawPriority(0x80000003u);
}

// DOL 0x801a6548 [audited]
void cTaskConsScore::Reset()
{
	static const int ANIM[4] = {0x6c, 0x72, 0x6f, 0x69};
	for (uint i = 0; i < 4; i++) pAnita(i)->SetAnimNo(ANIM[i], true);
	for (cTaskScoreNum* p : m_pNum) p->Reset();
	m_pNum[3]->SetMode(2);
	m_pNum[3]->SetBlink(false, 1);
	m_pNum[0]->SetBlink(false, 1);
	m_pNum[3]->SetNumber((uint)KingdomGlobal::WaniWaniSys->nHighScore());
	m_nBonus = m_nDamage = m_nShown = m_nHits = m_nCombo = 0;
	m_nHigh = (uint)KingdomGlobal::WaniWaniSys->nHighScore();
	m_pNum[3]->SetNumber(m_nHigh);
	SetMode(0);
}

uint cTaskConsScore::Score() const
{
	const int s = m_nCombo + m_nBonus + m_nHits - m_nDamage;
	return s < 0 ? 0u : (uint)s;
}

// DOL 0x801a669c [audited: checked]: the boards' entry animation (mode 2) ends -> their game animations (mode 3)
bool cTaskConsScore::Execute(cJetTask* pParam)
{
	if (Paused()) return true;   // KingSys +0xa40 -> +0x1b4
	if (m_nMode == 2 && pAnita(0u)->IsState(ANIM_END)) SetMode(3);
	return cTaskAnita::Execute(pParam);
}

// DOL 0x801a671c [audited: modes 0-3,6,7 vs decomp; mode 2 now uses 0x801b75e0 (SetStateBits) instead of SetBlink]
void cTaskConsScore::SetMode(int mode)
{
	m_nMode = mode;
	if (mode == 0)
	{
		for (uint i = 0; i < 5; i++) pAnita(i)->SetState(3u, false);
		m_pNum[3]->SetMode(0);
	}
	else if (mode == 1)
	{
		pAnita(3u)->SetAnimNo(0x69, true);
		pAnita(3u)->SetState(3u, true);
		m_pNum[3]->SetMode(2);
		m_pNum[3]->ShowNumber((uint)KingdomGlobal::WaniWaniSys->nHighScore());
		m_pNum[3]->SetMode(1);
	}
	else if (mode == 2)
	{
		for (uint i = 0; i < 5; i++) pAnita(i)->SetState(3u, false);
		m_pNum[3]->SetMode(0);
		static const int ANIM[3] = {0x6c, 0x72, 0x6f};
		for (uint i = 0; i < 3; i++)
		{
			pAnita(i)->SetAnimNo(ANIM[i], true);
			pAnita(i)->SetState(3u, true);
		}
		pAnita(3u)->SetAnimNo(0x6a, true);
		pAnita(3u)->SetState(1u, true);
		m_pNum[0]->SetMode(1);
		m_pNum[2]->SetMode(1);
		m_pNum[1]->SetMode(1);
		m_pNum[3]->SetMode(2);
		m_pNum[3]->SetStateBits(2u, false);   // 0x801b75e0(+0x1f4, 2, 0): bit 2 cleared, frame 0
		m_pNum[3]->ShowNumber((uint)KingdomGlobal::WaniWaniSys->nHighScore());
	}
	else if (mode == 3)
	{
		static const int ANIM[4] = {0x6d, 0x73, 0x70, 0x6a};
		for (uint i = 0; i < 4; i++)
		{
			pAnita(i)->SetAnimNo(ANIM[i], true);
			pAnita(i)->SetState(1u, true);
		}
		m_pNum[0]->SetMode(2);
		m_pNum[1]->SetMode(2);
		m_pNum[2]->SetMode(2);
		m_pNum[3]->SetMode(2);
		m_pNum[3]->ShowNumber((uint)KingdomGlobal::WaniWaniSys->nHighScore());
	}
	else if (mode == 6)
	{
		const uint s = Score();
		if ((uint)KingdomGlobal::WaniWaniSys->nHighScore() <= s)
		{
			m_pNum[3]->SetBlink(true, 1);
			m_pNum[0]->SetBlink(true, 1);
			if (m_nHigh <= s)
			{
				m_nHigh = s;
				m_pNum[3]->SetNumber(s);
			}
			m_pNum[0]->SetMode(4);
			m_pNum[3]->SetMode(4);
		}
	}
	else if (mode == 7)
	{
		for (cTaskScoreNum* p : m_pNum) p->SetMode(3);
		m_pCombo->ScoreNumMode3();   // the loop's fifth pointer (+0x1f8)
		pAnita(0u)->SetAnimNo(0x6e, true);
		pAnita(1u)->SetAnimNo(0x77, true);
		pAnita(2u)->SetAnimNo(0x7a, true);
		pAnita(3u)->SetAnimNo(0x7a, true);
	}
}

// DOL 0x801a6bc4 [audited]
void cTaskConsScore::SetHits(cTaskPacmanBase*, int n)
{
	m_nHits = n;
	m_pNum[1]->SetNumber((uint)n);
	int s = m_nBonus + m_nCombo + m_nHits - m_nDamage;
	if (s < 1) s = 0;
	if (m_nShown != s)
	{
		m_nShown = s;
		m_pNum[0]->SetNumber((uint)s);
	}
}

// DOL 0x801a6c38 [audited]
void cTaskConsScore::SetDamage(cTaskPacmanBase*, int n)
{
	if (m_nDamage == n) return;
	m_nDamage = n;
	m_pNum[2]->SetNumber((uint)n);
	int s = m_nBonus + m_nCombo + m_nHits - m_nDamage;
	if (s < 1) s = 0;
	if (m_nShown != s)
	{
		m_nShown = s;
		m_pNum[0]->SetNumber((uint)s);
	}
}

// DOL 0x801a6cb8 [audited]
bool cTaskConsScore::CanPay(cTaskPacmanBase*, int n)
{
	return 0 <= m_nCombo + m_nBonus + m_nHits - n;
}

// DOL 0x801a6cdc [audited]
void cTaskConsScore::Combo(cTaskPacmanBase*, int combo, uint chain)
{
	if (chain < 2u) return;
	m_nCombo = combo;
	if (chain == 2u) m_pCombo->First(2u);
	else m_pCombo->Set(0x62u < chain ? 99u : chain);
}

// ---- cTaskConsReadly ----

// DOL 0x801a42b8 [audited]
cTaskConsReadly::cTaskConsReadly()
{
	SetName("g04::cTaskConsReadly");
	pAnita(0u)->RegistData("GAME_ANITA");
	pAnita(0u)->SetAnimNo(99, true);
	pAnita(1u)->RegistData("GAME_ANITA");
	pAnita(1u)->SetAnimNo(0x65, true);
	m_nMode = 0;
	for (uint i = 0; i < 2; i++) pAnita(i)->SetState(3u, false);
	nDrawPriority(0x80000008u);
}

// DOL 0x801a4440 [audited]
void cTaskConsReadly::Reset()
{
	pAnita(0u)->SetAnimNo(99, true);
	pAnita(1u)->SetAnimNo(0x65, true);
	m_nMode = 0;
	for (uint i = 0; i < 2; i++) pAnita(i)->SetState(3u, false);
}

// DOL 0x801a4544 [audited]
void cTaskConsReadly::SetMode(int mode)
{
	m_nMode = mode;
	if (mode == 0)
		for (uint i = 0; i < 2; i++) pAnita(i)->SetState(3u, false);
	else if (mode == 1)
		pAnita(0u)->SetState(3u, true);
	else if (mode == 2)
	{
		for (uint i = 0; i < 2; i++) pAnita(i)->SetState(3u, false);
		pAnita(1u)->SetState(3u, true);
	}
}

// DOL 0x801a461c [audited]
void cTaskConsReadly::Ready()
{
	m_nMode = 1;
	pAnita(0u)->SetState(3u, true);
}

// DOL 0x801a46e4 [audited] (GO's end calls the tortoise's 0x801b87a4, which does nothing)
bool cTaskConsReadly::Execute(cJetTask* pParam)
{
	if (Paused()) return true;
	if (m_nMode == 1)
	{
		if (pAnita(0u)->IsState(ANIM_END)) pAnita(0u)->SetAnimNo(100, true);
	}
	return cTaskAnita::Execute(pParam);
}

// ---- cTaskConsTimer ----

int cTaskConsTimer::Anim(int digit, int which) const
{
	// 0x8037cc98 + colour * 0x84 + digit * 0xc + which * 4 (colour 0: 0x86.., colour 1: 0x42..); the colon at +0x78
	return (m_nColor == 0 ? 0x86 : 0x42) + digit * 3 + which;
}

// DOL 0x801a6e90 [audited: colon anim fixed to 0x1e (was 0x1f)]
cTaskConsTimer::cTaskConsTimer()
{
	SetName("g04::cTaskConsTimer");
	m_nColor = 0;
	for (uint i = 0; i < 3; i++)
	{
		cKingdomAnita* a = pAnita(i);
		a->RegistData("GAME_ANITA");
		a->SetState(3u, false);
		a->SetAnimNo(Anim(0, 1), true);
		Vector4 v;
		if (a->GetLayerPos(v, i == 2 ? 3u : i, 0x13)) a->vTrans() = v;
		m_nDigit[i] = 0;
		m_nChange[i] = 1;
	}
	cKingdomAnita* c = pAnita(3u);
	c->RegistData("GAME_ANITA");
	c->SetAnimNo(m_nColor == 0 ? 0x1e : 0x60, true);   // DAT_8037cd10 (+0x78) = 0x1e (word 0 of the colon set; SetMode 2 does not change it)
	Vector4 v;
	if (c->GetLayerPos(v, 2u, 0x13)) c->vTrans() = v;
	m_nStep = 0;
	m_bRun = true;
	m_nFrames = 0xb4;
	nDrawPriority(0x80000005u);
	SetMode(0);
}

// DOL 0x801a70ac [audited: colon anim fixed to 0x1e]
void cTaskConsTimer::Reset()
{
	m_nColor = 0;
	for (uint i = 0; i < 3; i++)
	{
		cKingdomAnita* a = pAnita(i);
		a->SetState(3u, false);
		a->SetAnimNo(Anim(0, 1), true);
		Vector4 v;
		if (a->GetLayerPos(v, i == 2 ? 3u : i, 0x13)) a->vTrans() = v;
		m_nDigit[i] = 0;
		m_nChange[i] = 1;
	}
	pAnita(3u)->SetAnimNo(0x1e, true);   // DAT_8037cd10
	SetMode(0);
	m_nStep = 0;
	m_bRun = true;
	m_nFrames = 0xb4;
}

// DOL 0x801a7918 [audited]
void cTaskConsTimer::SetMode(int mode)
{
	m_nMode = mode;
	if (mode == 0)
		for (uint i = 0; i < 4; i++) pAnita(i)->SetState(3u, false);
	else if (mode == 1)
		for (uint i = 0; i < 4; i++) pAnita(i)->SetState(3u, true);
	else if (mode == 2)
	{
		const uint s = (uint)m_nFrames / 60u;
		const uint sec = s % 60u;
		pAnita(0u)->SetAnimNo(Anim((int)(sec % 10u), 1), true);
		pAnita(1u)->SetAnimNo(Anim((int)(sec / 10u), 1), true);
		pAnita(2u)->SetAnimNo(Anim((int)(s / 60u), 1), true);
		for (uint i = 0; i < 4; i++)
		{
			pAnita(i)->SetState(1u, true);
			pAnita(i)->SetState(2u, false);
		}
	}
	else if (mode == 3)
	{
		for (uint i = 0; i < 3; i++) pAnita(i)->SetAnimNo(Anim(0, 2), true);
		pAnita(3u)->SetAnimNo(m_nColor == 0 ? 0x20 : 0x62, true);   // DAT_8037cd18
		for (uint i = 0; i < 4; i++) pAnita(i)->SetState(2u, true);
	}
}

// DOL 0x801a7bec [audited]
void cTaskConsTimer::SetTime(uint frames)
{
	m_nFrames = (int)frames;
	const uint s = frames / 60u, sec = s % 60u;
	pAnita(0u)->SetAnimNo(Anim((int)(sec % 10u), 0), true);
	pAnita(1u)->SetAnimNo(Anim((int)(sec / 10u), 0), true);
	pAnita(2u)->SetAnimNo(Anim((int)(s / 60u), 0), true);
	m_nStep = 1;
}

// DOL 0x801a7234 [audited]
bool cTaskConsTimer::Execute(cJetTask* pParam)
{
	if (Paused() || (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0) return true;
	if (m_nMode == 2)
	{
		if (!m_bRun) return cTaskAnita::Execute(pParam);
		const uint f = (uint)m_nFrames;
		if (0 < m_nFrames)
		{
			if (m_nStep == 0)
			{
				if (pAnita(0u)->IsState(ANIM_END))
				{
					const uint s = f / 60u, sec = s % 60u;
					pAnita(0u)->SetState(2u, true);
					pAnita(0u)->SetAnimNo(Anim((int)(sec % 10u), 0), true);
					if (m_nChange[1] != 0)
					{
						pAnita(1u)->SetState(2u, true);
						pAnita(1u)->SetAnimNo(Anim((int)(sec / 10u), 0), true);
					}
					if (m_nChange[2] != 0)
					{
						pAnita(2u)->SetState(2u, true);
						pAnita(2u)->SetAnimNo(Anim((int)(s / 60u), 0), true);
					}
					m_nStep = 2;
				}
			}
			else if (m_nStep == 1)
			{
				if (f % 60u == 0)
				{
					m_nChange[0] = 1;
					const uint s = f / 60u, min = s / 60u, sec = s % 60u, ones = sec % 10u, tens = sec / 10u;
					m_nChange[1] = ones == 0 ? 1 : 0;
					m_nChange[2] = (tens == 0 && ones == 0) ? 1 : 0;
					pAnita(0u)->SetState(2u, true);
					pAnita(0u)->SetAnimNo(Anim((int)ones, 2), true);
					if (m_nChange[1] != 0)
					{
						pAnita(1u)->SetState(2u, true);
						pAnita(1u)->SetAnimNo(Anim((int)tens, 2), true);
					}
					if (m_nChange[2] != 0)
					{
						pAnita(2u)->SetState(2u, true);
						pAnita(2u)->SetAnimNo(Anim((int)min, 2), true);
					}
					m_nDigit[0] = ones;
					m_nDigit[1] = tens;
					m_nDigit[2] = min;
					m_nStep = 0;
				}
			}
			else if (m_nStep == 2)
			{
				const uint sec = (f / 60u) % 60u;
				if (pAnita(0u)->IsState(ANIM_END))
				{
					pAnita(0u)->SetState(2u, false);
					pAnita(0u)->SetAnimNo(Anim((int)(sec % 10u), 1), true);
					if (m_nChange[1] != 0)
					{
						pAnita(1u)->SetState(2u, false);
						pAnita(1u)->SetAnimNo(Anim((int)(sec / 10u), 1), true);
					}
					if (m_nChange[2] != 0)
					{
						pAnita(2u)->SetState(2u, false);
						pAnita(2u)->SetAnimNo(Anim((int)((f / 60u) / 60u), 1), true);
					}
					m_nStep = 1;
				}
			}
		}
		m_nFrames--;
		if (m_nFrames == 600)
		{
			m_nColor = 1;
			pAnita(0u)->SetAnimNo(Anim(0, 1), true);
			pAnita(1u)->SetAnimNo(Anim(1, 1), true);            // DOL -0x7fc83358 = 0x8037cca8 + colour * 0x84: 0:10 shows a 1
			pAnita(2u)->SetAnimNo(Anim(0, 1), true);
			pAnita(3u)->SetAnimNo(0x61, true);                  // -0x7fc832ec = 0x8037cd14 + 0x84
			Se("m04se_13speed_up");                              // DAT_8047cc28
		}
		if (m_nFrames == 0) Se("m04se_14time_up");               // DAT_8047cc2c
	}
	return cTaskAnita::Execute(pParam);
}

// ---- cTaskConsMenu ----

namespace
{
// DOL 0x8037c590 [audited]: {appear, wait, point, decide} per item; 0x8037c5d0: the layout pattern the pointer tests
const int MENU_ANIM[4][4] = {{0x2b, 0x2c, 0x2d, 0x2e}, {0x24, 0x25, 0x26, 0x27}, {0x2f, 0x30, 0x31, 0x32}, {0x34, 0x35, 0x36, 0x37}};
const int MENU_PAT[4] = {0xc, 0xb, 0xd, 0xe};
}

// DOL 0x8019e5a0 [audited: checked (+0x1e4 float 1.0 not kept: unread)]
cTaskConsMenu::cTaskConsMenu()
{
	SetName("g04::cTaskConsMenu");
	for (uint i = 0; i < 4; i++)
	{
		cKingdomAnita* a = pAnita(i);
		a->RegistData("SELECT_ANITA");   // 0xb4e3e93e
		a->SetAnimNo(MENU_ANIM[i][0], true);
		a->SetState(1u, false);
	}
	SetMode(0);   // vtable +0x88 (0x8019ea3c) through +0x70 = Init
	nDrawPriority(0x80000003u);
}

// DOL 0x8019e880 [audited]
void cTaskConsMenu::Reset()
{
	for (uint i = 0; i < 4; i++)
	{
		pAnita(i)->SetAnimNo(MENU_ANIM[i][0], true);
		pAnita(i)->SetState(1u, false);
	}
	m_nSel = 2;
	m_nOldSel = 2;
	m_bDecided = false;
}

// DOL 0x8019ea3c [audited]
void cTaskConsMenu::SetMode(int mode)
{
	m_nMode = mode;
	if (mode == 0)
	{
		m_bDecided = false;
		for (uint i = 0; i < 4; i++) pAnita(i)->SetState(3u, false);
	}
	else if (mode == 1)
		for (uint i = 0; i < 4; i++)
		{
			pAnita(i)->SetState(3u, true);
			pAnita(i)->SetAnimNo(MENU_ANIM[i][0], true);
		}
	else if (mode == 2)
	{
		for (uint i = 0; i < 4; i++)
		{
			pAnita(i)->SetState(3u, true);
			pAnita(i)->SetAnimNo(MENU_ANIM[i][1], true);
		}
		m_nSel = 2;
		pAnita(2u)->SetAnimNo(0x31, true);
	}
	else if (mode == 3)
	{
		cKingdomAnita* a = pAnita((uint)m_nSel & 3u);
		a->SetAnimNo(MENU_ANIM[m_nSel & 3][3], true);
		a->SetFrame(0.0f);
	}
	else if (mode == 4)
		for (uint i = 0; i < 4; i++) pAnita(i)->SetState(3u, true);
}

// DOL 0x8019f41c [audited]: every item plays its animation backwards from the middle
void cTaskConsMenu::Close()
{
	SetMode(5);
	for (uint i = 0; i < 4; i++)
	{
		cKingdomAnita* a = pAnita(i);
		a->SetState(0x20000u, true);   // JET_ANITA_STATE_ANIM_REVERSE
		a->SetFrameRate(-1.0f);        // FLOAT_8047cb60
		a->SetFrame(0.5f * a->GetFrameNum(-1));
		a->SetState(2u, true);
	}
}

// DOL 0x8019f50c [audited]
bool cTaskConsMenu::IsClosed()
{
	for (uint i = 0; i < 4; i++)
		if (pAnita(i)->IsState(ANIM_END)) return true;
	return false;
}

// DOL 0x8019f134 [audited]: up / down move (wrapping), A / B / Z decide, TWO goes to "back"
bool cTaskConsMenu::Keys(int pad)
{
	const uint trig = GamePad::Get(pad).trig;
	int dir = 0;
	bool used = true;
	if (trig & GamePad::PAD_DOWN) dir = 1;
	else if (trig & GamePad::PAD_UP) dir = -1;
	else if ((trig & (GamePad::PAD_A | GamePad::PAD_B | GamePad::PAD_Z)) != 0)
	{
		if (m_nSel != 4)
		{
			{
				WiiSnd::Handle h;   // 0x8019f1dc: FUN_80025b6c(handles, 0, DAT_8047cb3c, 0)
				WiiSnd::PlayRemote(h, 0, "m00se_02Kettei");
				WiiSnd::Detach(h);
			}
			SetMode(3);
			m_bDecided = true;
		}
	}
	else
		used = false;
	if (dir != 0)
	{
		Se("m00se_01idou");   // DAT_8047cb38
		if (m_nSel != 4) pAnita((uint)m_nSel)->SetAnimNo(MENU_ANIM[m_nSel][1], true);
		if (m_nSel == 4) m_nSel = dir < 1 ? 3 : 0;
		else
		{
			const int n = m_nSel + dir;
			m_nSel = 4 <= n ? 0 : (n < 0 ? 3 : n);
		}
		pAnita((uint)m_nSel)->SetAnimNo(MENU_ANIM[m_nSel][2], true);
		pAnita((uint)m_nSel)->SetFrame(0.0f);
	}
	return used;
}

// DOL 0x8019ee04 [audited]: the pointer over an item's pattern (x = px * 320, y = -py * 240) selects it; A / B / Z decide
bool cTaskConsMenu::Pointer(int pad)
{
	const GamePad::State& s = GamePad::Get(pad);
	// the DOL converts both to int before the test (fctiwz): render mode fbWidth / efbHeight * 0.5 (FLOAT_8047cb4c)
	const Vector4 p((float)(int)(s.pointerX * 640.0f * 0.5f), (float)(int)(-s.pointerY * 480.0f * 0.5f), 0.0f, 1.0f);
	uint i = 0;
	for (; i < 4; i++)
		if (pAnita(i)->IsHit(p, MENU_PAT[i])) break;
	if (i == 4) return Keys(pad);
	if ((uint)m_nSel != i)
	{
		if (m_nSel >= 0 && m_nSel < 4) pAnita((uint)m_nSel)->SetAnimNo(MENU_ANIM[m_nSel][1], true);
		pAnita(i)->SetAnimNo(MENU_ANIM[i][2], true);
		m_nSel = (int)i;
		Se("m00se_01idou");
	}
	bool used = false;
	if ((s.trig & (GamePad::PAD_A | GamePad::PAD_B | GamePad::PAD_Z)) != 0)
	{
		SeRemote("m00se_02Kettei");   // 0x8019f06c: FUN_80025b6c(handles, 0, cue, 0)
		SetMode(3);
		m_bDecided = true;
		used = true;
	}
	m_nOldSel = m_nSel;
	return used;
}

// DOL 0x8019ec48 [audited: checked (stick direction store +0x1e0 unread)] (TWO without a pointer action: "back" is decided when it is selected, else the cursor jumps to it)
// (FUN_80199c30, the Nunchuk stick as a direction 1 / 2 / 4 / 8, is stored at +0x1e0, which nothing reads: not ported)
void cTaskConsMenu::Input()
{
	const int pad = 0;
	const bool used = GamePad::Get(pad).pointerCount < 1 ? Keys(pad) : Pointer(pad);
	if ((GamePad::Get(pad).trig & GamePad::PAD_2) != 0 && !used)
	{
		if (m_nSel == 0)
		{
			SeRemote("m00se_03Cancel");   // DAT_8047cb40 (0x8019ecf4: FUN_80025b6c(handles, 0, cue, 0))
			SetMode(3);
			m_bDecided = true;
		}
		else
		{
			Se("m00se_01idou");
			if (m_nSel != 0 && m_nSel < 4)
			{
				pAnita((uint)m_nSel)->SetAnimNo(MENU_ANIM[m_nSel][1], true);
				pAnita(0u)->SetAnimNo(0x2d, true);
				m_nSel = 0;
			}
		}
	}
}

// DOL 0x8019e924 [audited]
bool cTaskConsMenu::Execute(cJetTask* pParam)
{
	if (Paused()) return true;
	if (m_nMode == 1)
	{
		if (pAnita(0u)->IsState(ANIM_END)) SetMode(2);
	}
	else if (m_nMode == 2)
		Input();
	else if (m_nMode == 3)
	{
		if (pAnita((uint)m_nSel & 3u)->IsState(ANIM_END)) SetMode(4);
	}
	else if (m_nMode == 4 && onDecide)
		onDecide();
	return cTaskAnita::Execute(pParam);
}

// ---- cTaskConsMultiMenuTime ----

namespace
{
const int MULTI_TIMES[3] = {0x3c, 0xb4, 0x12c};   // DOL 0x8037c8b0 [audited] (seconds)
}

// DOL 0x801a219c [audited]
cTaskConsMultiMenuTime::cTaskConsMultiMenuTime()
{
	SetName("g04::cTaskConsMultiMenuTime");
	for (uint i = 0; i < 4; i++) pAnita(i)->RegistData("SELECT02_ANITA");   // 0x1bc00af6
	m_nIndex = 1;
	for (uint i = 0; i < 3; i++) pAnita(i)->SetAnimNo(0, true);
	pAnita(3u)->SetAnimNo(0x1f, true);
	pAnita(3u)->SetState(1u, true);
	pAnita(3u)->SetState(2u, false);
	const uint who[4] = {0, 1, 3, 2};   // anita 0 / 1 / 3 / 2 -> layer 0 / 1 / 2 / 3 (0x80014408)
	for (uint k = 0; k < 4; k++)
	{
		Vector4 v;
		if (pAnita(who[k])->GetLayerPos(v, k, 8)) pAnita(who[k])->vTrans() = v;
	}
	nDrawPriority(0x80000004u);
	SetMode(0);
}

// DOL 0x801a24a4 [audited]
void cTaskConsMultiMenuTime::Reset()
{
	m_nIndex = 1;
	for (uint i = 0; i < 3; i++)
	{
		pAnita(i)->SetAnimNo(0, true);
		pAnita(i)->vTrans() = Vector4(0.0f, 0.0f, 0.0f, 0.0f);   // +0x400..+0x40c = 0 (FLOAT_8047cbb4)
	}
	pAnita(3u)->SetAnimNo(0x1f, true);
	pAnita(3u)->SetState(1u, true);
	pAnita(3u)->SetState(2u, false);
	const uint who[4] = {0, 1, 3, 2};
	for (uint k = 0; k < 4; k++)
	{
		Vector4 v;
		if (pAnita(who[k])->GetLayerPos(v, k, 8)) pAnita(who[k])->vTrans() = v;
	}
	m_nIndex = KingdomGlobal::WaniWaniSys->nTimeIndex();
	SetMode(0);
}

// DOL 0x801a2e5c [audited]
int cTaskConsMultiMenuTime::Time() const
{
	return MULTI_TIMES[m_nIndex];
}

// DOL 0x801a27c0 [audited: checked earlier] (checked: anim numbers go through the table 0x8037c8c0, which is the identity 0..29, so digit * 3 / i * 3 + 2
// are used directly; case 5 sets the colon twice like the DOL)
void cTaskConsMultiMenuTime::SetMode(int mode)
{
	m_nMode = mode;
	auto showDigits = [this](uint n) { for (uint i = 0; i < n; i++) pAnita(i)->SetState(3u, true); };
	auto colon = [this] { pAnita(3u)->SetState(1u, true); pAnita(3u)->SetState(2u, false); };
	const uint t = (uint)MULTI_TIMES[m_nIndex];
	switch (mode)
	{
	case 0:
		for (uint i = 0; i < 4; i++) pAnita(i)->SetState(3u, false);
		break;
	case 1:
		showDigits(3);
		colon();
		pAnita(2u)->SetAnimNo((int)((t / 0x3c) % 0x3c) * 3, true);
		for (uint i = 0; i < 2; i++) pAnita(i)->SetAnimNo(0, true);
		break;
	case 2:
	case 3:
		showDigits(3);
		colon();
		break;
	case 5:
	{
		showDigits(3);
		colon();
		const uint d[3] = {t % 10, (t % 0x3c) / 10, (t / 0x3c) % 0x3c};
		for (uint i = 0; i < 3; i++) pAnita(i)->SetAnimNo((int)d[i] * 3, true);
		colon();
		break;
	}
	case 4:
		showDigits(3);
		pAnita(3u)->SetState(1u, true);
		pAnita(3u)->SetState(2u, false);
		for (uint i = 0; i < 3; i++) pAnita(i)->SetAnimNo((int)i * 3 + 2, true);
		break;
	}
}

// DOL 0x801a2bd0 [audited] (left arrow)
void cTaskConsMultiMenuTime::Left()
{
	const uint old = (uint)MULTI_TIMES[m_nIndex];
	m_nIndex = m_nIndex - 1 < 0 ? 2 : m_nIndex - 1;
	SeRemote("m00se_02Kettei");   // DAT_8047cbb0 (0x801a2c5c: FUN_80025b6c(handles, 0, cue, 0))
	pAnita(2u)->SetAnimNo((int)((old / 0x3c) % 0x3c) * 3 + 2, true);
	pAnita(0u)->SetAnimNo(2, true);
	pAnita(1u)->SetAnimNo(2, true);
	SetMode(3);
	KingdomGlobal::WaniWaniSys->nTimeIndex() = m_nIndex;
}

// DOL 0x801a2d14 [audited] (right arrow)
void cTaskConsMultiMenuTime::Right()
{
	const uint old = (uint)MULTI_TIMES[m_nIndex];
	m_nIndex = m_nIndex + 1 < 3 ? m_nIndex + 1 : 0;
	SeRemote("m00se_02Kettei");   // 0x801a2da4: FUN_80025b6c(handles, 0, cue, 0)
	pAnita(2u)->SetAnimNo((int)((old / 0x3c) % 0x3c) * 3 + 2, true);
	pAnita(1u)->SetAnimNo(2, true);
	pAnita(0u)->SetAnimNo(2, true);
	SetMode(3);
	KingdomGlobal::WaniWaniSys->nTimeIndex() = m_nIndex;
}

// DOL 0x801a271c [audited]
bool cTaskConsMultiMenuTime::Execute(cJetTask* pParam)
{
	if (m_nMode == 1)
	{
		if (pAnita(2u)->IsState(ANIM_END)) SetMode(2);
	}
	else if (m_nMode == 3 && pAnita(2u)->IsState(ANIM_END))
		SetMode(1);
	return cTaskAnita::Execute(pParam);
}

// ---- cTaskConsMultiMenu ----

namespace
{
// DOL 0x8037c6b8 [audited]: player panel {appear, wait}; 0x8037c6d8: item {appear, wait, point, decide}
const int MM_PLAYER[4][2] = {{0x27, 0x28}, {0x29, 0x2a}, {0x2b, 0x2c}, {0x2d, 0x2e}};
const int MM_ITEM[5][4] = {{0x3e, 0x3f, 0x40, 0x41}, {0x2f, 0x30, 0x31, 0x32}, {0x3b, 0x3c, 0x3c, 0x3d}, {0x42, 0x43, 0x44, 0x45}, {0x47, 0x48, 0x49, 0x4a}};
const int MM_MARK_PAT[4] = {0, 1, 2, 3};   // DOL 0x8037c738 [audited]
const int MM_HIT_PAT[6] = {0xc, 0xb, 0xd, 0xe, 0xf, 0x10};
}

// DOL 0x801a00d0 [audited: names drawn natively (gfx::DrawText); CSV_GAME text params read by cParamGame]
cTaskConsMultiMenu::cTaskConsMultiMenu()
{
	SetName("g04::cTaskConsMultiMenu");
	for (uint i = 0; i < 15; i++) pAnita(i)->RegistData("SELECT02_ANITA");   // 0x1bc00af6
	m_pTime = new cTaskConsMultiMenuTime();
	ConnectChild(m_pTime);
	Reset();   // vtable +0x74
	m_bDecided = false;
	nDrawPriority(0x80000003u);   // the console's builder sets 0x80000003 (checked); kept here for a menu built alone
}

// DOL 0x801a0480 [audited]
void cTaskConsMultiMenu::Reset()
{
	for (uint i = 0; i < 4; i++) pAnita(i)->SetAnimNo(MM_PLAYER[i][0], true);
	for (uint i = 0; i < 5; i++) pAnita(4u + i)->SetAnimNo(MM_ITEM[i][0], true);
	pAnita(9u)->SetAnimNo(0x33, true);
	pAnita(10u)->SetAnimNo(0x37, true);
	cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	uint mask = (uint)sys->nWinMask();
	for (uint p = 0; p < (uint)sys->nPlayers(); p++)
	{
		Vector4 v;
		if (pAnita(p + 11u)->GetLayerPos(v, 0, MM_MARK_PAT[p])) pAnita(p + 11u)->vTrans() = v;
		pAnita(p + 11u)->SetAnimNo(0x24, true);
		if ((mask & 1u) == 0) pAnita(p + 11u)->SetState(3u, false);
		else pAnita(p + 11u)->SetAnimNo(3, true);
		mask >>= 1;
	}
	m_nFocus = 3;
	SetMode(0);
	m_pTime->Reset();
}

// DOL 0x801a1d3c [audited: IsSel/interface slots 0x801a1d3c..1db4 checked in the earlier pass] / 0x801a1d64 / 0x801a1d8c / 0x801a1db4 (interface +0x10..+0x1c: focus 0 / 4 / 1 / 3 and decided, the single menu's IsSel(0 / 3 / 1 / 2))
bool cTaskConsMultiMenu::IsSel(int i) const
{
	static const int FOCUS[4] = {0, 1, 3, 4};   // IsSel(0 back / 1 how to play / 2 start / 3 players)
	return m_bDecided && m_nFocus == FOCUS[i & 3];
}

// DOL 0x801a0810 [audited: checked earlier] (checked against the decomp)
void cTaskConsMultiMenu::SetMode(int mode)
{
	m_nMode = mode;
	cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	const uint n = (uint)sys->nPlayers();
	auto marks = [&] {
		uint mask = (uint)sys->nWinMask();
		for (uint p = 0; p < n; p++)
		{
			pAnita(p)->SetState(3u, true);
			pAnita(p + 11u)->SetState(3u, (mask & 1u) != 0);
			mask >>= 1;
		}
	};
	if (mode == 0 || mode == 1)
	{
		for (uint i = 0; i < 15; i++) pAnita(i)->SetState(3u, false);
		m_pTime->SetMode(0);
		if (mode == 1)
		{
			for (uint i = 4; i < 9; i++) pAnita(i)->SetState(3u, true);
			pAnita(9u)->SetState(3u, true);
			pAnita(10u)->SetState(3u, true);
			marks();
			for (uint p = 0; p < 4; p++) pAnita(p)->SetAnimNo(MM_PLAYER[p][0], true);
			for (uint i = 0; i < 5; i++) pAnita(4u + i)->SetAnimNo(MM_ITEM[i][0], true);
			pAnita(9u)->SetAnimNo(0x33, true);
			pAnita(10u)->SetAnimNo(0x37, true);
			pAnita(9u)->SetState(3u, true);
			pAnita(10u)->SetState(3u, true);
		}
	}
	else if (mode == 2)
	{
		for (uint i = 4; i < 9; i++) pAnita(i)->SetState(3u, true);
		pAnita(9u)->SetState(3u, true);
		pAnita(10u)->SetState(3u, true);
		marks();
		m_pTime->SetMode(5);   // 0x801a27b4 = li r4, 5; b SetMode
		pAnita(5u)->SetAnimNo(0x30, true);
		pAnita(7u)->SetAnimNo(0x44, true);
		m_nFocus = 3;
		uint mask = (uint)sys->nWinMask();
		for (uint p = 0; p < n; p++)
		{
			Vector4 v;
			if (pAnita(p + 11u)->GetLayerPos(v, 0, MM_MARK_PAT[p])) pAnita(p + 11u)->vTrans() = v;
			if ((mask & 1u) == 0) pAnita(p + 11u)->SetState(3u, false);
			else
			{
				pAnita(p + 11u)->SetAnimNo(0x25, true);
				pAnita(p + 11u)->SetState(3u, true);
			}
			mask >>= 1;
		}
	}
	else if (mode == 3)
		SeRemote("m00se_02Kettei");   // DAT_8047cb7c (0x801a0c70: FUN_80025b6c(handles, 0, cue, 0))
	else if (mode == 4)   // first calls FUN_801a27bc on the time console: an empty blr
		pAnita((uint)m_nFocus + 4u)->SetAnimNo(MM_ITEM[m_nFocus][3], true);
}

// the shared "move the focus" of the pointer branch (0x801a0fe4): wait anims for the items, arrows as given
void cTaskConsMultiMenu::Focus(int focus, int arrowLeft, int arrowRight)
{
	Se("m00se_01idou");   // DAT_8047cb78
	pAnita(4u)->SetAnimNo(MM_ITEM[0][1], true);
	pAnita(5u)->SetAnimNo(MM_ITEM[1][1], true);
	pAnita(7u)->SetAnimNo(MM_ITEM[3][1], true);
	pAnita(8u)->SetAnimNo(MM_ITEM[4][1], true);
	pAnita(9u)->SetAnimNo(arrowLeft, true);
	pAnita(10u)->SetAnimNo(arrowRight, true);
	if (focus != 2) pAnita((uint)focus + 4u)->SetAnimNo(MM_ITEM[focus][2], true);
	m_nFocus = focus;
}

void cTaskConsMultiMenu::Arrow(bool right)
{
	pAnita(9u)->SetState(3u, true);
	pAnita(10u)->SetState(3u, true);
	if (right)
	{
		pAnita(10u)->SetAnimNo(0x3a, true);
		pAnita(9u)->SetAnimNo(0x35, true);
		m_pTime->Right();
	}
	else
	{
		pAnita(9u)->SetAnimNo(0x36, true);
		pAnita(10u)->SetAnimNo(0x39, true);
		m_pTime->Left();
	}
}

// DOL 0x801a0fe4 [audited]: the pointer over the six hit patterns (back, how to play, time left / right, start, players)
bool cTaskConsMultiMenu::Pointer(int pad)
{
	const GamePad::State& s = GamePad::Get(pad);
	const Vector4 p((float)(int)(s.pointerX * 640.0f * 0.5f), (float)(int)(-s.pointerY * 480.0f * 0.5f), 0.0f, 1.0f);   // int like 0x8019ee04 (fctiwz)
	int focus = -1;
	bool right = false;
	if (pAnita(4u)->IsHit(p, 0xc)) focus = 0;
	else if (pAnita(4u)->IsHit(p, 0xb)) focus = 1;
	else if (pAnita(4u)->IsHit(p, 0xd)) focus = 3;
	else if (pAnita(4u)->IsHit(p, 0xe)) focus = 4;
	else if (pAnita(4u)->IsHit(p, 0xf)) focus = 2;
	else if (pAnita(4u)->IsHit(p, 0x10)) { focus = 2; right = true; }
	if (m_nFocus != focus && focus != -1)
	{
		if (focus == 2) Focus(2, right ? 0x34 : 0x35, right ? 0x39 : 0x38);
		else Focus(focus, 0x34, 0x38);
		m_nFocus = focus;
	}
	else if (focus == -1)
		Keys(pad);   // its result is dropped
	if ((s.trig & (GamePad::PAD_A | GamePad::PAD_B | GamePad::PAD_Z)) != 0)
	{
		if (focus == 2) Arrow(right);
		else if (m_nFocus != 2)
		{
			pAnita((uint)m_nFocus + 4u)->SetAnimNo(MM_ITEM[m_nFocus][3], true);
			pAnita((uint)m_nFocus + 4u)->SetState(3u, true);
			SetMode(3);
			m_bDecided = true;
		}
	}
	return true;   // 0x801a1944 li r3, 1 on every path: TWO (back) is never taken while the pointer is on screen (unlike the single menu)
}

// DOL 0x801a197c [audited]
// (FUN_80199c30, the Nunchuk stick as a direction 1 / 2 / 4 / 8, is stored at +0x1e0, which nothing reads: not ported)
bool cTaskConsMultiMenu::Keys(int pad)
{
	const uint trig = GamePad::Get(pad).trig;
	int dir = 0;
	bool used = true;
	if (trig & GamePad::PAD_DOWN) dir = 1;
	else if (trig & GamePad::PAD_UP) dir = -1;
	else if ((trig & (GamePad::PAD_A | GamePad::PAD_B | GamePad::PAD_Z)) != 0)
	{
		if (m_nFocus != 2)
		{
			pAnita((uint)m_nFocus + 4u)->SetAnimNo(MM_ITEM[m_nFocus][3], true);
			pAnita((uint)m_nFocus + 4u)->SetState(2u, true);
			SetMode(3);
			m_bDecided = true;
		}
	}
	else if (trig & GamePad::PAD_LEFT)
	{
		if (m_nFocus == 2) Arrow(false);
	}
	else if (trig & GamePad::PAD_RIGHT)
	{
		if (m_nFocus == 2) Arrow(true);
	}
	else
		used = false;
	if (dir != 0)
	{
		Se("m00se_01idou");
		pAnita((uint)m_nFocus + 4u)->SetAnimNo(MM_ITEM[m_nFocus][1], true);
		const int n = m_nFocus + dir;
		m_nFocus = n < 5 ? (n < 0 ? 4 : n) : 0;
		pAnita((uint)m_nFocus + 4u)->SetAnimNo(MM_ITEM[m_nFocus][2], true);
		pAnita((uint)m_nFocus + 4u)->SetFrame(0.0f);
		pAnita(9u)->SetAnimNo(m_nFocus == 2 ? 0x35 : 0x34, true);
		pAnita(10u)->SetAnimNo(m_nFocus == 2 ? 0x39 : 0x38, true);
	}
	return used;
}

// DOL 0x801a0ce4 [audited]
void cTaskConsMultiMenu::Input()
{
	const int pad = 0;
	const bool used = GamePad::Get(pad).pointerCount < 1 ? Keys(pad) : Pointer(pad);
	if ((GamePad::Get(pad).trig & GamePad::PAD_2) != 0 && !used)
	{
		if (m_nFocus == 0)
		{
			SeRemote("m00se_03Cancel");   // DAT_8047cb80 (0x801a0e9c: FUN_80025b6c(handles, 0, cue, 0))
			pAnita(4u)->SetAnimNo(0x41, true);
			m_bDecided = true;
			if (onDecide) onDecide();
		}
		else
		{
			Se("m00se_01idou");
			pAnita(4u)->SetAnimNo(0x3f, true);
			pAnita(5u)->SetAnimNo(0x30, true);
			pAnita(7u)->SetAnimNo(0x43, true);
			pAnita(8u)->SetAnimNo(0x48, true);
			pAnita(9u)->SetAnimNo(0x34, true);
			pAnita(10u)->SetAnimNo(0x38, true);
			pAnita(4u)->SetAnimNo(0x40, true);
			m_nFocus = 0;
		}
	}
	if (m_nFocus == 2)
	{
		// the arrows' press animations settle back into their idle frames (DOL: copy the frame, switch the anim)
		if ((pAnita(9u)->GetState() & ANIM_END) != 0)
		{
			const float f = pAnita(10u)->GetFrame();
			pAnita(9u)->SetAnimNo(0x35, true);
			pAnita(9u)->SetFrame(f);
		}
		else if ((pAnita(10u)->GetState() & ANIM_END) != 0)
		{
			const float f = pAnita(9u)->GetFrame();
			pAnita(10u)->SetAnimNo(0x39, true);
			pAnita(10u)->SetFrame(f);
		}
	}
}

// DOL 0x801a0658 [audited]
bool cTaskConsMultiMenu::Execute(cJetTask* pParam)
{
	if (Paused()) return true;
	if (m_nMode == 2) Input();
	else if (m_nMode == 3 && pAnita((uint)m_nFocus + 4u)->IsState(ANIM_END) && onDecide)
		onDecide();
	return cTaskAnita::Execute(pParam);
}


namespace
{
// FUN_80016dd8 / 80016dd0 / 80016e88 + width test 0x80018afc: the name at (x, y) screen pixels, size - 2 when it is `wide` px or more, left aligned on the baseline
void DrawName(float x, float y, const char* name, uint color, int size, float wide)
{
	const int h = gfx::FontHeight("RESIDENT_FONT_MII");
	if (h <= 0 || name == nullptr) return;
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_MII";   // KingSys +0xa68 = RESIDENT_FONT_MII (0x5fd151b6, stored by 0x8000aa90)
	st.color = color;
	st.vorigin = gfx::TextStyle::VOrigin::Baseline;
	st.scaleX = st.scaleY = (float)size / (float)h;
	if (gfx::MeasureTextWidth(name, st) >= wide) st.scaleX = st.scaleY = (float)(size - 2) / (float)h;
	gfx::DrawText((float)(int)x, (float)(int)y, name, st);
}
}

// DOL 0x801a3cd8 [audited: checked, threshold 160] (called by Draw 0x801a36a0)
void cTaskConsMultiScore::DrawNames()
{
	static const int PAT[4] = {0xf, 0x10, 0x11, 0x12};   // DOL 0x8037c9b8 [audited], layer 0 of anita 0
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	if (app == nullptr || sys == nullptr) return;
	for (uint i = 0; i < (uint)sys->nPlayers() && i < 4u; i++)
	{
		Vector4 v;
		if (!pAnita(0u)->GetLayerPos(v, 0, PAT[i])) continue;
		DrawName(320.0f + v.x, 240.0f - v.y, sys->pMatchingData()->player[i].szName, app->ParamGame().nNameColor[0][i], app->ParamGame().nNameSize[0], 160.0f);
	}
}

// DOL 0x801a36a0 [audited: checked]: the anita, then (modes 1..4) the names
void cTaskConsMultiScore::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
	if (m_nMode >= 1 && m_nMode <= 4) DrawNames();
}

// DOL 0x801a5e4c [audited: checked (colour/size sets from the earlier pass)]: one winner name at layer 0 of anita 0xb's pattern `pat` (set 1 / size row 17 for the tie boards, set 2 / row 18 for the single winner)
void cTaskConsResult::DrawName(int pat, int player, bool tie)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	if (app == nullptr || sys == nullptr || player < 0 || player > 3) return;
	Vector4 v;
	if (!pAnita(0xbu)->GetLayerPos(v, 0, pat)) return;
	const cParamGame& pg = app->ParamGame();
	::kingdom::game::g04::DrawName(320.0f + v.x, 240.0f - v.y, sys->pMatchingData()->player[player].szName, pg.nNameColor[tie ? 1 : 2][player], pg.nNameSize[tie ? 1 : 2], 170.0f);
}

// DOL 0x801a5c68 [audited: checked] (mode 5): count 1 = the winner at pattern 0; 4 / 3 / 2 = the leaders on patterns 5.. (count 3: 0xc.., count 4: 5..8, count 2: 5..)
void cTaskConsResult::DrawNames()
{
	static const int PAT[10] = {5, 6, 7, 8, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11};   // DOL 0x8037cbb4 [audited]
	if (m_nMode != 5) return;
	if (m_nWinCount == 1)
	{
		int w = 4;
		for (int i = 3; i >= 0; i--)
			if ((m_nWinMask >> i) & 1u) w = i;
		DrawName(0, w, false);
	}
	else if (m_nWinCount == 4)
		for (int i = 0; i < 4; i++) DrawName(PAT[i], i, true);
	else if (m_nWinCount == 3 || m_nWinCount == 2)
	{
		int k = 0;
		for (int i = 0; i < 4; i++)
			if ((m_nWinMask >> i) & 1u) DrawName(PAT[(m_nWinCount == 3 ? 4 : 0) + k++], i, true);
	}
}

// DOL 0x801a5674 [audited: modes 4/5 names]: the anita, then (modes 4 / 5) the names
void cTaskConsResult::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
	if (m_nMode == 4 || m_nMode == 5) DrawNames();
}

// DOL 0x801a070c [audited]: the anita, then (modes 2 / 3) the names
void cTaskConsMultiMenu::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
	if (m_nMode == 2 || m_nMode == 3) DrawNames();
}

// DOL 0x801a1ddc [audited]: each player's name at layer 0 of SELECT02 anita 4's pattern 4 + i (table 0x8037c748), screen (320 + x, 240 - y), left aligned
// on the baseline (writer mode 0), size = CSV_GAME row 16 (2 less when the name is 160 px or wider), colour = CSV_GAME rows 4 + i.
// The font: KingSys +0xa68 = RESIDENT_FONT_MII (0x5fd151b6, stored by 0x8000aa90).
void cTaskConsMultiMenu::DrawNames()
{
	cG04WaniWaniSys* sys = KingdomGlobal::WaniWaniSys;
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (sys == nullptr || app == nullptr) return;
	const int h = gfx::FontHeight("RESIDENT_FONT_MII");
	if (h <= 0) return;
	for (uint i = 0; i < (uint)sys->nPlayers() && i < 4u; i++)
	{
		Vector4 v;
		if (!pAnita(4u)->GetLayerPos(v, 0, (int)(4 + i))) continue;
		const char* name = sys->pMatchingData()->player[i].szName;
		gfx::TextStyle st;
		st.font = "RESIDENT_FONT_MII";
		st.color = app->ParamGame().nNameColor[0][i];
		st.vorigin = gfx::TextStyle::VOrigin::Baseline;
		float size = (float)app->ParamGame().nNameSize[0];
		st.scaleX = st.scaleY = size / (float)h;
		if (gfx::MeasureTextWidth(name, st) >= 160.0f) st.scaleX = st.scaleY = (size - 2.0f) / (float)h;
		gfx::DrawText((float)(int)(320.0f + v.x), (float)(int)(240.0f - v.y), name, st);
	}
}

// ---- cTaskConsMenuTutorial ----

namespace
{
using kingdom::game::menu::sTutorialData;
using kingdom::game::menu::sTutorialPage;
sTutorialPage s_Page[2] = {{0, 1, "TUTORIAL01_ANITA"}, {1, 2, "TUTORIAL01_ANITA"}};   // DOL 0x80394340 [audited]
sTutorialData* TutorialData()   // DOL 0x8036d548 [audited] (2 pages, MENU_TEXT_TUTORIAL)
{
	static sTutorialData s{2u, "MENU_TEXT_TUTORIAL", {&s_Page[0], &s_Page[1]}};
	return &s;
}
}

// DOL 0x80094870 [audited] (base ctor 0x80145a9c with COMMON_ANITA_TUTORIAL)
cTaskConsMenuTutorial::cTaskConsMenuTutorial(kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	RegistData("COMMON_ANITA_TUTORIAL", TutorialData());
}

bool cTaskConsMenuTutorial::IsDeleteEnd()   // DOL 0x800948cc [audited]
{
	return cTaskConsMenuTutorialBase::IsDeleteEnd();
}

namespace
{
sTutorialPage s_MultiPage[3] = {{0, 4, "TUTORIAL01_ANITA"}, {2, 5, "TUTORIAL01_ANITA"}, {1, 6, "TUTORIAL01_ANITA"}};   // DOL 0x80394270 [audited]
sTutorialData* MultiTutorialData()   // DOL 0x8036d508 [audited] (3 pages, MENU_TEXT_TUTORIAL)
{
	static sTutorialData s{3u, "MENU_TEXT_TUTORIAL", {&s_MultiPage[0], &s_MultiPage[1], &s_MultiPage[2]}};
	return &s;
}
}

// DOL 0x80094688 [audited] (base ctor 0x80145a9c with COMMON_ANITA_TUTORIAL)
cTaskConsMenuMultiTutorial::cTaskConsMenuMultiTutorial(kingdom::cTaskCursor* pCursor)
{
	SetCursor(pCursor);
	RegistData("COMMON_ANITA_TUTORIAL", MultiTutorialData());
}

bool cTaskConsMenuMultiTutorial::IsDeleteEnd()   // DOL 0x800946e4 [audited]
{
	return cTaskConsMenuTutorialBase::IsDeleteEnd();
}


// ---- cTaskConsMultiScore ----

namespace
{
const int MULTI_ANIM[4][3] = {{0x75, 0x76, 0x77}, {0x78, 0x79, 0x7a}, {0x7b, 0x7c, 0x7d}, {0x7e, 0x7f, 0x80}};   // DAT_8037c968: board p: shown, appear, out
const int MULTI_NUM_PAT[4] = {6, 7, 8, 9};        // DAT_8037c9a8: the digit patterns of player p's number
const int MULTI_CROWN_PAT[4] = {0xb, 0xc, 0xd, 0xe};   // DAT_8037c9c8: where the crown of player p sits
}

// DOL 0x801a2e7c [audited: ctor/tables 0x8037c968/c9a8/c9c8 verified; text params natively] (the console's 0x801b0808 builds the object: priority 0x80000003, child of the console)
cTaskConsMultiScore::cTaskConsMultiScore()
{
	SetName("g04::cTaskConsMultiScore");
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	for (int i = 0; i < n && i < 4; i++)
	{
		pAnita((uint)i)->RegistData("GAME_ANITA");
		pAnita((uint)i)->SetAnimNo(MULTI_ANIM[i][0], true);
		cKingdomAnita* crown = pAnita((uint)i + 4u);
		crown->RegistData("GAME_ANITA");
		crown->SetAnimNo(0x66, true);
		Vector4 v;
		if (crown->GetLayerPos(v, 0, MULTI_CROWN_PAT[i])) crown->vTrans() = v;   // FUN_80014408(anita, anita + 0x400, 0, pattern)
	}
	for (int i = 0; i < n && i < 4; i++)
	{
		m_pNum[i] = new cTaskScoreNum();
		for (uint d = 0; d < 3; d++)
		{
			Vector4 v;
			if (m_pNum[i]->pAnita(d)->GetLayerPos(v, d, MULTI_NUM_PAT[i])) m_pNum[i]->SetPos(d, v);   // = 0x801b73fc, inlined
		}
		ConnectChild(m_pNum[i]);
	}
	// the cParamGame values read for the name text (+0x2d4 char space): the names are drawn by DrawNames (0x801a3cd8) through gfx::DrawText
	for (uint i = 0; i < 8; i++) pAnita(i)->SetState(3u, false);
	nDrawPriority(0x80000003u);
}

// DOL 0x801a34d0 [audited]
void cTaskConsMultiScore::Reset()
{
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	for (int i = 0; i < n && i < 4; i++)
	{
		for (uint d = 0; d < 3; d++)
		{
			Vector4 v;
			if (m_pNum[i]->pAnita(d)->GetLayerPos(v, d, MULTI_NUM_PAT[i])) m_pNum[i]->SetPos(d, v);
		}
		m_pNum[i]->Reset();
	}
	SetMode(0);
	for (int i = 0; i < 4; i++) m_nDamage[i] = m_nHits[i] = m_nCombo[i] = m_nState[i] = 0;
	for (uint i = 0; i < 8; i++) pAnita(i)->SetState(3u, false);
}

// DOL 0x801a393c [audited]: 0 hidden, 1 appear (the boards slide in, the crowns wait), 2 the boards' idle animation, 3 numbers only, 5 the numbers only / crowns off (result)
void cTaskConsMultiScore::SetMode(int mode)
{
	m_nMode = mode;
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	if (mode == 0)
	{
		for (uint i = 0; i < 8; i++) pAnita(i)->SetState(3u, false);
		for (int i = 0; i < n && i < 4; i++) m_pNum[i]->SetMode(0);
	}
	else if (mode == 1 || mode == 2)
	{
		for (int i = 0; i < n && i < 4; i++)
		{
			pAnita((uint)i)->SetState(3u, true);
			pAnita((uint)i + 4u)->SetState(3u, true);
		}
		for (int i = 0; i < n && i < 4; i++) pAnita((uint)i)->SetAnimNo(MULTI_ANIM[i][mode - 1], true);   // DAT_8037c968[3 i] (mode 1) / [3 i + 1] (mode 2)
		if (mode == 1)
			for (int i = 0; i < n && i < 4; i++)
			{
				pAnita((uint)i + 4u)->SetState(2u, true);
				m_nState[i] = 0;
				pAnita((uint)i + 4u)->SetAnimNo(0x66, true);
			}
		for (int i = 0; i < n && i < 4; i++) m_pNum[i]->SetMode(mode);
	}
	else if (mode == 3)
	{
		for (int i = 0; i < n && i < 4; i++) m_pNum[i]->SetMode(3);
	}
	else if (mode == 5)
	{
		for (int i = 0; i < n && i < 4; i++) m_pNum[i]->SetMode(3);
		for (int i = 0; i < n && i < 4; i++) pAnita((uint)i)->SetAnimNo(MULTI_ANIM[i][2], true);
		for (uint i = 0; i < 4; i++) pAnita(i + 4u)->SetState(1u, false);
	}
}

// DOL 0x801a35cc [audited]: a crown whose appear animation ended switches to its loop (0x67)
bool cTaskConsMultiScore::Execute(cJetTask* pParam)
{
	if (Paused()) return true;
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	for (int i = 0; i < n && i < 4; i++)
		if (m_nState[i] == 0 && pAnita((uint)i + 4u)->IsState(ANIM_END))
		{
			pAnita((uint)i + 4u)->SetAnimNo(0x67, true);
			m_nState[i] = 1;
		}
	return cTaskAnita::Execute(pParam);
}

// DOL 0x801a3ef4 [audited]
void cTaskConsMultiScore::Leaders(uint& mask, int& count) const
{
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	int best = -1;
	count = 1;
	for (int i = 0; i < n && i < 4; i++)
	{
		const int s = Score((uint)i);
		if (best < s)
		{
			count = 1;
			best = s;
			mask = 1u << i;
		}
		else if (best == s)
		{
			mask |= 1u << i;
			count++;
		}
	}
}

// DOL 0x801a3fc0 [audited]
uint cTaskConsMultiScore::Hits() const
{
	uint sum = 0;
	for (int i = 0; i < KingdomGlobal::WaniWaniSys->nPlayers() && i < 4; i++) sum += (uint)m_nHits[i];
	return sum;
}

// DOL 0x801a37a4 [audited]: the player's number, then every crown follows the leaders (a new leader's crown appears, one that fell behind leaves)
void cTaskConsMultiScore::Refresh(uint p)
{
	m_pNum[p]->SetNumber((uint)Score(p));
	uint mask = 0;
	int count = 0;
	Leaders(mask, count);
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	for (int i = 0; i < n && i < 4; i++, mask >>= 1)
	{
		cKingdomAnita* crown = pAnita((uint)i + 4u);
		if ((mask & 1u) == 0)
		{
			if (m_nState[i] == 1)
			{
				crown->SetAnimNo(0x68, true);
				crown->SetState(3u, true);
				m_nState[i] = 2;
			}
		}
		else if (m_nState[i] != 1)
		{
			crown->SetState(3u, true);
			m_nState[i] = 0;
			crown->SetAnimNo(0x66, true);
		}
	}
}

void cTaskConsMultiScore::SetBonus(cTaskPacmanBase*, int) {}   // listener +0x0c = 0x801a42a8 -> 0x801a4274 (blr): the red gator bonus is not counted in a match
void cTaskConsMultiScore::SetHits(cTaskPacmanBase* p, int n)
{
	m_nHits[p->nPlayer()] = n;
	Refresh(p->nPlayer());
}
void cTaskConsMultiScore::SetDamage(cTaskPacmanBase* p, int n)
{
	m_nDamage[p->nPlayer()] = n;
	Refresh(p->nPlayer());
}
void cTaskConsMultiScore::Combo(cTaskPacmanBase* p, int combo, uint)
{
	if ((uint)combo < (uint)m_nCombo[p->nPlayer()]) return;
	m_nCombo[p->nPlayer()] = combo;
}
bool cTaskConsMultiScore::CanPay(cTaskPacmanBase* p, int n)
{
	return m_nHits[p->nPlayer()] + m_nCombo[p->nPlayer()] - n >= 0;
}

// DOL 0x801a4078 [audited]: after the round the single leader's number blinks
void cTaskConsMultiScore::Finish()
{
	uint mask = 0;
	int count = 0;
	Leaders(mask, count);
	if (count == 1)
		for (uint i = 0; i < 4; i++, mask >>= 1)
			if ((mask & 1u) != 0) m_pNum[i]->SetBlink(true, 1);
	SetMode(4);
}

// DOL 0x801a6000 [audited]
void cTaskConsResult::SetWinners(uint mask, int count)
{
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	m_nWinMask = mask;
	m_nWinCount = count;
	KingdomGlobal::WaniWaniSys->nWinMask() = (int)mask;
	if (count == 1)
	{
		for (int i = 0; i < n; i++, mask >>= 1)
			if ((mask & 1u) != 0) m_nRank = i;
	}
	else
		m_nRank = 4;
}

// ---- cTaskConsResult ----

// DOL 0x801a488c [audited]
cTaskConsResult::cTaskConsResult()
{
	SetName("g04::cTaskConsResult");
	for (uint i = 0; i < 0x10; i++) pAnita(i)->RegistData("RESULT_ANITA");   // 0x8c032fe9
	Reset();
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_RESULT"))   // 0xc95c6d4f (cParamResult)
		for (int i = 0; i < 5; i++) m_nBorder[i] = csv.S32(i, 1, 0);
	// the font writer (+0x1e0) for the multiplayer names: natively DrawNames (0x801a5e4c) through gfx::DrawText
	nDrawPriority(0x80000002u);
}

// DOL 0x801a4d7c [audited]
void cTaskConsResult::Reset()
{
	static const int ANIM[16] = {0, -1, 1, 2, 3, 4, 5, 6, 8, 10, 0xc, 0xe, 0xf, 0x11, 0x13, 0x15};
	for (uint i = 0; i < 16; i++)
		if (ANIM[i] >= 0) pAnita(i)->SetAnimNo(ANIM[i], true);
	m_nRank = 0;
	m_fTimer = 0.0f;
	SetMode(0);
}

// DOL 0x801a609c [audited]
void cTaskConsResult::SetScore(uint score)
{
	int r = 0;
	while (r < 5 && score < (uint)m_nBorder[r]) r++;
	m_nRank = r;
}

// DOL 0x801a60fc [audited]
int cTaskConsResult::Rank(uint score) const
{
	for (int r = 0; r < 5; r++)
		if ((uint)m_nBorder[r] <= score) return r;
	return 5;
}

// DOL 0x801a5778 [audited: checked earlier] (modes 0..3 single player, 4 the multiplayer board; checked against the decomp, tables 0x8037cb98 / 0x8037cba8 / 0x8037cbd0)
void cTaskConsResult::SetMode(int mode)
{
	m_nMode = mode;
	m_fTimer = 0.0f;
	if (mode == 0)
		for (uint i = 0; i < 0x10; i++) pAnita(i)->SetState(3u, false);
	else if (mode == 1)
		pAnita(0u)->SetState(3u, true);
	else if (mode == 2)
		pAnita((uint)m_nRank + 2u)->SetState(3u, true);
	else if (mode == 3)
	{
		cKingdomAnita* a = pAnita((uint)m_nRank + 2u);
		a->SetState(3u, true);
		a->SetFrame(0.0f);
	}
	else if (mode == 4)   // the multiplayer board: one winner (anitas 7..10 = the winner's gator) or the draw board (anita 0xb with the tied players' faces, 0xc..0xf)
	{
		for (uint i = 0; i < 0x10; i++) pAnita(i)->SetState(3u, false);
		if (m_nWinCount == 1)
		{
			for (uint i = 7; i < 0xb; i++)
			{
				pAnita(i)->SetState(3u, m_nRank == (int)i - 7);
				pAnita(i)->SetFrame(0.0f);
			}
			Se("m04se_03maitta");
		}
		else
		{
			pAnita(0xbu)->SetState(3u, true);
			pAnita(0xbu)->SetFrame(0.0f);
			Se("m04se_05konnnanonn");
			static const int POS4[4] = {1, 2, 3, 4};      // DAT_8037cb98
			static const int POS3[4] = {9, 0xa, 0xb, 5};  // DAT_8037cba8
			static const int ANIM[4] = {0xf, 0x11, 0x13, 0x15};   // DAT_8037cbd0 (every other word)
			const int* pos = m_nWinCount == 3 ? POS3 : POS4;
			int k = 0;
			if (m_nWinCount < 2 || 4 < m_nWinCount) return;   // only 2 / 3 / 4 tied players place faces and pick the board's animation
			for (uint i = 0; i < 4; i++)
			{
				if (m_nWinCount != 4 && ((m_nWinMask >> i) & 1u) == 0) continue;
				cKingdomAnita* a = pAnita(i + 0xcu);
				Vector4 v;
				if (a->GetLayerPos(v, 0, pos[m_nWinCount == 4 ? (int)i : k])) a->vTrans() = v;
				a->SetAnimNo(ANIM[i], true);
				a->SetState(3u, true);
				k++;
			}
			pAnita(0xbu)->SetAnimNo(m_nWinCount == 4 ? 0x19 : m_nWinCount == 3 ? 0x18 : 0x17, true);
		}
	}
}

// DOL 0x801a5194 [audited] (checked against the decomp, modes 1 / 3 / 4 / 5 / 6): the board, the rank message, its voice, then (180 frames or a button) the console is told
bool cTaskConsResult::Execute(cJetTask* pParam)
{
	if (Paused()) return true;
	const uint trig = GamePad::Get(0).trig;
	const bool any = (trig & (GamePad::PAD_A | GamePad::PAD_B | GamePad::PAD_MINUS | GamePad::PAD_PLUS | GamePad::PAD_UP | GamePad::PAD_LEFT |
	                          GamePad::PAD_RIGHT | GamePad::PAD_DOWN)) != 0;
	if (m_nMode == 1)
	{
		if (pAnita(0u)->IsState(ANIM_END)) SetMode(KingdomGlobal::WaniWaniSys->nPlayers() == 1 ? 3 : 4);
	}
	else if (m_nMode == 3)
	{
		if (pAnita((uint)m_nRank + 2u)->IsState(ANIM_END))
		{
			static const char* const VOICE[5] = {"m04se_03maitta", "m04se_04abunai", "m04se_05konnnanonn", "m04se_06amai", "m04se_07zamaamiro"};
			WiiSnd::Play(m_Voice, VOICE[m_nRank < 5 ? m_nRank : 4]);
			m_fTimer += 1.0f;
			SetMode(6);
		}
	}
	else if (m_nMode == 4 || m_nMode == 5)   // the multiplayer board: the winner's intro ends -> its loop, then 3 seconds (or any key) -> done
	{
		if (m_nMode == 5)
		{
			m_fTimer += 1.0f;
			if ((180.0f <= m_fTimer || any) && onDone) onDone();
		}
		if (m_nWinCount == 1)
		{
			if (pAnita((uint)m_nRank + 7u)->IsState(ANIM_END))
			{
				pAnita((uint)m_nRank + 7u)->SetAnimNo(m_nRank * 2 + 7, true);
				pAnita((uint)m_nRank + 7u)->SetState(1u, true);
				SetMode(5);
			}
		}
		else if (m_nMode == 4 && pAnita(0xbu)->IsState(ANIM_END)) SetMode(5);
	}
	else if (m_nMode == 6)
	{
		const bool talking = m_Voice.voice >= 0 && snd::IsPlaying(m_Voice.voice);
		if (!talking && KingdomGlobal::WaniWaniSys->bNewRecord())
		{
			KingdomGlobal::WaniWaniSys->bNewRecord() = false;
			if (!(m_Hiscore.voice >= 0 && snd::IsPlaying(m_Hiscore.voice))) WiiSnd::Play(m_Hiscore, "m00se_06Hiscore");
		}
		m_fTimer += 1.0f;
		if ((180.0f <= m_fTimer || any) && onDone) onDone();   // FLOAT_8047cc04
	}
	return cTaskAnita::Execute(pParam);
}

// ---- cTaskGameConsole ----

// DOL 0x801b07f0 [audited: ctor incl. priorities (3/3/2/3/3/8/5/7/6) checked vs disassembly] + 0x801b0808 (single player)
cTaskGameConsole::cTaskGameConsole()
{
	SetName("g04::cTaskGameConsole");
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	for (int i = 0; i < n && i < 4; i++)
	{
		kingdom::cTaskCursor* c = new kingdom::cTaskCursor();   // kingdom::cTaskMenuCursor (0x800951f4)
		c->Init((uint)i, "COMMON_ANITA_POINTER", i * 2);       // 0x8037d398[2i]
		c->AddButton(0x860u, i * 2 + 1);
		c->nDrawPriority(0x80000007u);
		ConnectChild(c);
		c->SetPadNo(n == 1 ? 0 : KingdomGlobal::WaniWaniSys->nWork(i));
		m_pCursor[i] = c;
	}
	if (n == 1) m_pTutorial = new cTaskConsMenuTutorial(m_pCursor[0]);   // both get cursor 0 (+0x1f0)
	else m_pTutorial = new cTaskConsMenuMultiTutorial(m_pCursor[0]);
	m_pTutorial->nDrawPriority(0x80000005u);
	ConnectChild(m_pTutorial);
	if (n == 1)
	{
		m_pMenu = new cTaskConsMenu();
		m_pMenu->nDrawPriority(0x80000003u);
		m_pMenu->onDecide = [this] { m_bMenuDecided = true; };   // the cMenuMessageListener (+0x1b4) +0xc = DOL 0x801b1a70 [audited] (+0x208 = 1)
		ConnectChild(m_pMenu);
	}
	else
	{
		m_pMultiMenu = new cTaskConsMultiMenu();
		m_pMultiMenu->nDrawPriority(0x80000003u);
		m_pMultiMenu->onDecide = [this] { m_bMenuDecided = true; };
		ConnectChild(m_pMultiMenu);
	}
	m_pResult = new cTaskConsResult();
	m_pResult->nDrawPriority(0x80000002u);
	m_pResult->onDone = [this] { ResultDone(); };
	ConnectChild(m_pResult);
	m_pScore = new cTaskConsScore();
	m_pScore->nDrawPriority(0x80000003u);
	ConnectChild(m_pScore);
	if (n != 1)
	{
		m_pMultiScore = new cTaskConsMultiScore();   // +0x1e8
		m_pMultiScore->nDrawPriority(0x80000003u);
		ConnectChild(m_pMultiScore);
	}
	m_pReadly = new cTaskConsReadly();
	m_pReadly->nDrawPriority(0x80000008u);
	ConnectChild(m_pReadly);
	if (n == 1)
	{
		m_pRetry = new kingdom::game::cTaskConsRetry("COMMON_ANITA_RETRY", "COMMON_ANITA_POINTER", 1u);   // 0x97668b20 / 0x1a3f6a03
		m_pRetry->nDrawPriority(0x80000006u);
		ConnectChild(m_pRetry);
	}
	SetMode(0);
	m_bMenuDecided = false;
	m_bResultDone = false;
}

// DOL 0x801b1434 [audited]: the menu pointers live in modes 2 / 3 / 6; the tutorial's end returns to the menu (3)
bool cTaskGameConsole::Execute(cJetTask* pParam)
{
	cAppG04WaniWaniGame* app = cAppG04WaniWaniGame::Instance();
	if (app->IsPause() || app->IsHome()) return true;
	const int n = KingdomGlobal::WaniWaniSys->nPlayers();
	// +0x1e1 of each cursor: modes 1 / 4 off, 2 / 3 / 6 on; mode 5 (and 0 / 7 / 8) leave it as it is
	const bool on = m_nMode == 2 || m_nMode == 3 || m_nMode == 6;
	if (m_nMode >= 1 && m_nMode <= 6 && m_nMode != 5)
		for (int i = 0; i < n && i < 4; i++)
			if (m_pCursor[i] != nullptr) m_pCursor[i]->SetEnable(on);
	if (m_nMode == 2)
	{
		m_pTutorial->Select();   // vtable +0x58 (0x801465a4)
		if (m_pTutorial->IsDeleteEnd())   // vtable +0x60
		{
			// multiplayer: the Mii icon manager's FUN_801b1f44 (app +0x2b0): NOT PORTED (Mii icons wait for RFL)
			SetMode(3);
		}
	}
	return cTaskKingdom::Execute(pParam);
}

// DOL 0x801b1678 [audited]
void cTaskGameConsole::SetMode(int mode)
{
	m_nMode = mode;
	auto base = [this] {
		MenuSetMode(0);   // interface +0x24 (0)
		m_pReadly->SetMode(0);
		if (m_pMultiScore == nullptr) m_pScore->SetMode(0);
		else m_pMultiScore->SetMode(0);
		m_pResult->SetMode(0);
	};
	switch (mode)
	{
	case 0: base(); break;
	case 1:
		base();
		if (m_pMultiScore == nullptr) m_pScore->SetMode(2);   // 0x801a668c
		else m_pMultiScore->SetMode(1);                      // 0x801a34c0
		break;
	case 2:
		base();
		m_pTutorial->Start();   // vtable +0x5c
		m_bMenuDecided = false;
		break;
	case 3:
		base();
		m_bMenuDecided = false;
		MenuSetMode(2);
		if (m_pMultiScore == nullptr)
		{
			m_pScore->SetHigh((uint)KingdomGlobal::WaniWaniSys->nHighScore());
			m_pScore->SetMode(1);
		}
		break;
	case 4:
		base();
		if (m_pMultiScore == nullptr) m_pScore->SetMode(3);   // 0x801a6694
		else m_pMultiScore->SetMode(2);                      // 0x801a34c8
		break;
	case 5:
		if (m_pMultiScore == nullptr)
		{
			m_pResult->SetScore(m_pScore->Score());
			m_pScore->SetMode(6);   // 0x801a6d38
		}
		else
		{
			uint mask = 0;
			int count = 0;
			m_pMultiScore->Leaders(mask, count);
			m_pResult->SetWinners(mask, count);   // 0x801a6000
			m_pMultiScore->Finish();              // 0x801a4078
		}
		m_pResult->SetMode(1);
		break;
	case 6: m_pResult->SetMode(0); break;
	case 8:
		if (m_pMultiScore == nullptr) m_pScore->SetMode(7);   // 0x801a6d48
		else m_pMultiScore->SetMode(5);                      // 0x801a4114
		break;
	default: break;
	}
}

// DOL 0x801b13b4 [audited: checked (+0x204 cleared: unread)]
void cTaskGameConsole::Restart()
{
	m_bResultDone = false;
	m_bMenuDecided = false;
	if (m_pMultiScore == nullptr) m_pScore->Reset();
	else m_pMultiScore->Reset();   // 0x801a34d0
	if (m_pMenu != nullptr) m_pMenu->Reset();   // interface +0xc
	else m_pMultiMenu->Reset();
	m_pResult->Reset();
	m_pReadly->Reset();
}

sScoreListener* cTaskGameConsole::pListener()
{
	if (m_pMultiScore != nullptr) return m_pMultiScore;
	return m_pScore;
}

// DOL 0x801b1a1c [audited]
uint cTaskGameConsole::Hits() const
{
	if (m_pMultiScore != nullptr) return m_pMultiScore->Hits();   // 0x801a3fc0
	return m_pScore->nHits();
}

// DOL 0x801b1afc [audited] (single): the rank of the score
int cTaskGameConsole::ResultRank() const
{
	if (m_pMultiScore != nullptr)   // the single winner's index, 4 = a tie
	{
		uint mask = 0;
		int count = 0;
		m_pMultiScore->Leaders(mask, count);
		if (count != 1) return 4;
		for (int i = 0; i < 4; i++, mask >>= 1)
			if ((mask & 1u) != 0) return i;
	}
	return m_pResult->Rank(m_pScore->Score());
}

// DOL 0x801b1c0c [audited]
bool cTaskGameConsole::MenuClosed()
{
	return m_pMenu != nullptr ? m_pMenu->IsClosed() : m_pMultiMenu->IsClosed();
}

// DOL 0x801b1c20 [audited]
void cTaskGameConsole::StartGame()
{
	if (m_pMenu != nullptr) m_pMenu->Close();   // single player only
	SetMode(7);
}

// DOL 0x801b1ab4 [audited]
int cTaskGameConsole::MultiTime() const
{
	return m_pMultiMenu == nullptr ? 0xb4 : m_pMultiMenu->MultiTime();
}

bool cTaskGameConsole::MenuIsSel(int i) const
{
	return m_pMenu != nullptr ? m_pMenu->IsSel(i) : m_pMultiMenu->IsSel(i);
}

void cTaskGameConsole::MenuSetMode(int mode)
{
	if (m_pMenu != nullptr) m_pMenu->SetMode(mode);
	else m_pMultiMenu->SetMode(mode);
}

}
