#include "kingdom/game/g11/cTaskGrobdaConsole.hpp"
#include <cstdlib>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "jet/JetUtil.hpp"
#include "jet/sDatLayP.hpp"
#include "kingdom/game/g11/cG11GrobdaSys.hpp"

namespace kingdom::game::g11
{

namespace
{
constexpr uint ANIM_END = 0x10000u;   // cJetAnita::JET_ANITA_STATE_ANIM_END

// the gate every Grobda console Execute starts with: app +0x1b4 == 0 and HOME (pad 0 hold & 0x400) not held
bool Running()
{
	kingdom::cAppKingdom* app = KingdomGlobal::KingSys != nullptr ? KingdomGlobal::KingSys->pAppMain() : nullptr;   // KingSys +0xa40
	if (app != nullptr && app->IsPause()) return false;
	return (GamePad::Get(0).hold & 0x400u) == 0;
}
}

// DOL 0x8012d738
cTaskGrobdaConsoleBase::cTaskGrobdaConsoleBase()
{
	SetName("cTaskGrobdaConsoleBase");
}

// DOL 0x8012d79c
bool cTaskGrobdaConsoleBase::Execute(cJetTask* pParam)
{
	if (Running()) return kingdom::cTaskAnita::Execute(pParam);   // FUN_80014d58(+0x1b4): the anita vector update
	return true;
}

// DOL 0x8012d800
void cTaskGrobdaConsoleBase::SetVisible(uint n, bool b)
{
	pAnita(n)->SetState(1u, b);
}

// DOL 0x8012d854
bool cTaskGrobdaConsoleBase::IsVisible(uint n)
{
	return pAnita(n)->IsState(1u);
}

// DOL 0x8012d880
bool cTaskGrobdaConsoleBase::IsAnimEnd(uint n)
{
	return pAnita(n)->IsState(ANIM_END);
}

// DOL 0x80130e54
cTaskGrobdaConsoleReadyGo::cTaskGrobdaConsoleReadyGo()
{
	SetName("cTaskGrobdaConsoleReadyGo");
	pAnita(0)->RegistData("m11c_game01.anp");   // FUN_80141708(anita, FUN_8000bc70("m11c_game01.anp"), layer)
	SetVisible(0, false);
}

// DOL 0x8013111c
void cTaskGrobdaConsoleReadyGo::Start()
{
	m_nMode = 1;
	pAnita(0)->SetAnimNo(0x32, true);
	SetVisible(0, true);
}

// DOL 0x80130f20
bool cTaskGrobdaConsoleReadyGo::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	kingdom::cKingdomAnita* a = pAnita(0);
	switch (m_nMode)
	{
	case 1:
		if (IsAnimEnd(0)) { m_nMode = 2; m_nWait = 60; a->SetAnimNo(0x33, true); }   // DAT_80477ec8
		break;
	case 2:
		if (--m_nWait < 1) { m_nMode = 3; a->SetAnimNo(0x34, true); }
		break;
	case 3:
		if (a->GetAnimNo() == 0x34)   // *(anita + 8)
		{
			if (IsAnimEnd(0)) { a->SetAnimNo(0x35, true); m_nWait = 30; }   // DAT_80477ecc
		}
		else if (--m_nWait < 1) { m_nMode = 4; a->SetAnimNo(0x36, true); }
		break;
	case 4:
		if (IsAnimEnd(0)) { m_nMode = 5; SetVisible(0, false); }
		break;
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// DOL 0x801326b8
cTaskGrobdaConsoleTime::cTaskGrobdaConsoleTime()
{
	SetName("cTaskGrobdaConsoleTime");
	const bool multi = false;   // FUN_8000ed74 (sys +4 matching block): multiplayer is NOT PORTED
	const int pat = multi ? 0x16 : 2;
	for (uint i = 0; i < 8; i++)
	{
		pAnita(i)->RegistData("m11c_game01.anp");
		SetVisible(i, false);
		Vector4 v;
		if (pAnita(i)->GetLayerPos(v, i, pat)) pAnita(i)->vTrans() = v;   // FUN_80014408(anita, +0x400, layer, pat)
	}
}

// DOL 0x80132b6c
int cTaskGrobdaConsoleTime::AnimNo(uint layer, int mode, int digit)
{
	if (layer - 2u < 6u)
	{
		if (mode >= 0 && mode <= 3) return digit * 4 + mode;
		return -1;
	}
	if (layer > 1u) return -1;
	switch (mode)
	{
	case 0: return 0x28;
	case 1: return 0x29;
	case 2: return 0x2a;
	case 3: return 0x2b;
	}
	return -1;
}

// DOL 0x8013295c
void cTaskGrobdaConsoleTime::ShowTime(int mode)
{
	const int sec = m_nTimeMs / 1000;
	const int min = sec / 60;
	const int cs = (m_nTimeMs / 10) % 100;
	const int d[8] = {0, 0, cs % 10, cs / 10, (sec % 60) % 10, (sec % 60) / 10, min % 10, (min / 10) % 10};   // UNVERIFIED: min tens wraps like the DOL's last division chain
	for (uint i = 0; i < 8; i++) pAnita(i)->SetAnimNo(AnimNo(i, mode, d[i]), false, 0.0f);   // FLOAT_8047c118 = start frame
}

// DOL 0x801327e0
bool cTaskGrobdaConsoleTime::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	auto allEnd = [&]() { for (uint i = 0; i < 8; i++) if (!IsAnimEnd(i)) return false; return true; };
	if (m_nMode == 1)
	{
		if (allEnd()) { m_nMode = 2; ShowTime(1); }
	}
	else if (m_nMode == 2)
		ShowTime(1);
	else if (m_nMode == 3)
	{
		if (allEnd())
			for (uint i = 0; i < 8; i++) { m_nMode = 4; SetVisible(i, false); }
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// DOL 0x80132c10
void cTaskGrobdaConsoleTime::Start()
{
	m_nMode = 1;
	ShowTime(0);
	for (uint i = 0; i < 8; i++) SetVisible(i, true);
}

// DOL 0x80132c7c
void cTaskGrobdaConsoleTime::End()
{
	m_nMode = 3;
	ShowTime(2);
}

// FUN_80014408 (+ the Score ctor's scale copy 0x80142d54 +0x28 / +0x2c): the position of layer `layer` of pattern `pat` into the anita's
// translation (+0x400), optionally its scale (+0x420 / +0x424)
bool cTaskGrobdaConsoleBase::PlaceLayer(uint anita, uint layer, int pat, bool scale)
{
	kingdom::cKingdomAnita* a = pAnita(anita);
	Vector4 v;
	if (!a->GetLayerPos(v, layer, pat)) return false;
	a->vTrans() = v;
	if (scale)
	{
		void* ptr = a->GetData().data();
		if (pat >= 0 && pat < jet::JetUtil::anpPatCount(ptr) && (int)layer < jet::JetUtil::anpPatLayCount(ptr, pat))
		{
			jet::sDatLayP* l = (jet::sDatLayP*)jet::JetUtil::anpPatLayPtr(ptr, pat, (int)layer);
			a->vScale()[0] = l->isx;
			a->vScale()[1] = l->isy;
		}
	}
	return true;
}

// ---- Finish ----

// DOL 0x8012df78
cTaskGrobdaConsoleFinish::cTaskGrobdaConsoleFinish()
{
	SetName("cTaskGrobdaConsoleFinish");
	pAnita(0)->RegistData("m11c_game01.anp");
	SetVisible(0, false);
}

// DOL 0x8012e1a4
void cTaskGrobdaConsoleFinish::Start()
{
	m_nMode = 1;
	pAnita(0)->SetAnimNo(0x3a, true);
	SetVisible(0, true);
}

// DOL 0x8012e20c
void cTaskGrobdaConsoleFinish::End()
{
	if (m_nMode == 2)
	{
		m_nMode = 3;
		pAnita(0)->SetAnimNo(0x3c, true);
	}
}

// DOL 0x8012e044
bool cTaskGrobdaConsoleFinish::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	if (m_nMode == 1)
	{
		if (IsAnimEnd(0))
		{
			m_nMode = 2;
			pAnita(0)->SetAnimNo(0x3b, true);
			m_nWait = cG11GrobdaSys::Instance()->IsMulti() ? 120 : 360;   // DAT_80477e94 / DAT_80477e90
		}
	}
	else if (m_nMode == 2)
	{
		if (--m_nWait < 1) End();   // vtable +0x4c
	}
	else if (m_nMode == 3)
	{
		if (IsAnimEnd(0)) { m_nMode = 4; SetVisible(0, false); }
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// ---- Score ----

// DOL 0x80131fd4
cTaskGrobdaConsoleScore::cTaskGrobdaConsoleScore(int player)
{
	SetName("cTaskGrobdaConsoleScore");
	m_bMulti = cG11GrobdaSys::Instance()->IsMulti();
	m_nPlayer = player;
	m_nScore = 0;
	const int pat = m_bMulti ? player + 0xe : 0;
	for (uint i = 0; i < Count(); i++)
	{
		pAnita(i)->RegistData("m11c_game01.anp");
		SetVisible(i, false);
		m_nDigit[i] = 0;
		m_nState[i] = 0;
		PlaceLayer(i, i, pat, true);
	}
}

// DOL 0x80132188
bool cTaskGrobdaConsoleScore::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	for (uint i = 0; i < Count(); i++)
	{
		if (m_nState[i] == 1 || m_nState[i] == 4)
		{
			if (IsAnimEnd(i))
			{
				m_nState[i] = 2;
				pAnita(i)->SetAnimNo(m_nDigit[i] * 4 + 1, false);   // FLOAT_8047c110 = start frame 0
			}
		}
		else if (m_nState[i] == 3 && IsAnimEnd(i))
		{
			m_nState[i] = 5;
			SetVisible(i, false);
		}
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// DOL 0x801322dc
void cTaskGrobdaConsoleScore::Start()
{
	m_nScore = 0;
	for (uint i = 0; i < Count(); i++)
	{
		m_nDigit[i] = 0;
		m_nState[i] = 1;
		pAnita(i)->SetAnimNo(m_nDigit[i] << 2, false);
		SetVisible(i, true);
	}
}

// DOL 0x80132390
void cTaskGrobdaConsoleScore::End()
{
	for (uint i = 0; i < Count(); i++)
	{
		m_nState[i] = 3;
		pAnita(i)->SetAnimNo(m_nDigit[i] * 4 + 2, false);
	}
}

// DOL 0x8013241c (the cap: DAT_80477ee8 = 999999 single, DAT_80477eec = 99999 multiplayer)
void cTaskGrobdaConsoleScore::SetScore(int score)
{
	const int cap = m_bMulti ? 99999 : 999999;
	m_nScore = score > cap ? cap : (score < 0 ? 0 : score);
	int n = m_nScore;
	for (uint i = 0; i < Count(); i++)
	{
		const int d = n % 10;
		if (d != m_nDigit[i])
		{
			m_nDigit[i] = d;
			m_nState[i] = 4;
			pAnita(i)->SetAnimNo(d * 4 + 3, false);
		}
		n /= 10;
	}
}

// DOL 0x80132524
void cTaskGrobdaConsoleScore::Flash()
{
	for (uint i = 0; i < Count(); i++) pAnita(i)->SetAnimNo(m_nDigit[i] * 4 + 3, true);
}

// ---- HighScore ----

// DOL 0x8012ec48
cTaskGrobdaConsoleHighScore::cTaskGrobdaConsoleHighScore()
{
	SetName("cTaskGrobdaConsoleHighScore");
	m_nHigh = cG11GrobdaSys::Instance()->nBest();   // *(sys +8)
	for (uint i = 0; i < 6; i++)
	{
		pAnita(i)->RegistData("m11c_game01.anp");
		SetVisible(i, false);
		m_nDigit[i] = 0;
		m_nState[i] = 0;
		PlaceLayer(i, i, 1, false);
	}
}

// DOL 0x8012ed50
bool cTaskGrobdaConsoleHighScore::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	for (uint i = 0; i < 6; i++)
	{
		if (m_nState[i] == 1 || m_nState[i] == 4)
		{
			if (IsAnimEnd(i)) { m_nState[i] = 2; pAnita(i)->SetAnimNo(m_nDigit[i] * 4 + 1, false); }   // FLOAT_8047c0d8
		}
		else if (m_nState[i] == 3 && IsAnimEnd(i))
		{
			m_nState[i] = 5;
			SetVisible(i, false);
		}
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// DOL 0x8012ee8c
void cTaskGrobdaConsoleHighScore::Start()
{
	int n = m_nHigh;
	for (uint i = 0; i < 6; i++)
	{
		m_nDigit[i] = n % 10;
		m_nState[i] = 1;
		pAnita(i)->SetAnimNo(m_nDigit[i] << 2, false);
		SetVisible(i, true);
		n /= 10;
	}
}

// DOL 0x8012ef50
void cTaskGrobdaConsoleHighScore::End()
{
	for (uint i = 0; i < 6; i++)
	{
		m_nState[i] = 3;
		pAnita(i)->SetAnimNo(m_nDigit[i] * 4 + 2, false);
	}
}

// DOL 0x8012efd4 (cap DAT_80477ea8 = 999999)
void cTaskGrobdaConsoleHighScore::SetNew(int score)
{
	if (m_nHigh >= score) return;
	m_nHigh = score > 999999 ? 999999 : (score < 0 ? 0 : score);
	int n = m_nHigh;
	for (uint i = 0; i < 6; i++)
	{
		m_nDigit[i] = n % 10;
		m_nState[i] = 4;
		pAnita(i)->SetAnimNo(m_nDigit[i] * 4 + 3, false);
		n /= 10;
	}
}

// ---- Combo ----

// DOL 0x8012d8f8
cTaskGrobdaConsoleCombo::cTaskGrobdaConsoleCombo()
{
	SetName("cTaskGrobdaConsoleCombo");
	for (uint i = 0; i < 3; i++)
	{
		pAnita(i)->RegistData("m11c_game01.anp");
		SetVisible(i, false);
	}
	PlaceLayer(1, 1, 4, false);
	PlaceLayer(2, 0, 4, false);
}

// DOL 0x8012dcf0
void cTaskGrobdaConsoleCombo::Set(int n)
{
	m_nMode = 1;
	m_nCombo = n < 100 ? (n < 0 ? 0 : n) : 99;
	pAnita(0)->SetAnimNo(0x37, true);
	SetVisible(0, true);
	const int tens = m_nCombo / 10;
	pAnita(1)->SetAnimNo((m_nCombo - tens * 10) * 3 + 0x66, true);
	SetVisible(1, true);
	if (tens < 1)
		SetVisible(2, false);
	else
	{
		pAnita(2)->SetAnimNo(tens * 3 + 0x66, true);
		SetVisible(2, true);
	}
}

// DOL 0x8012d9f4
bool cTaskGrobdaConsoleCombo::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	auto allEnd = [&]() { for (uint i = 0; i < 3; i++) if (!IsAnimEnd(i)) return false; return true; };
	auto digits = [&](int base)
	{
		const int tens = m_nCombo / 10;
		pAnita(1)->SetAnimNo((m_nCombo - tens * 10) * 3 + base, true);
		if (IsVisible(2)) pAnita(2)->SetAnimNo(tens * 3 + base, true);
	};
	if (m_nMode == 1)
	{
		if (allEnd())
		{
			m_nMode = 2;
			m_nWait = 180;   // DAT_80477e80
			pAnita(0)->SetAnimNo(0x38, true);
			digits(0x67);
		}
	}
	else if (m_nMode == 2)
	{
		if (--m_nWait < 0)
		{
			m_nMode = 3;
			pAnita(0)->SetAnimNo(0x39, true);
			digits(0x68);
		}
	}
	else if (m_nMode == 3)
	{
		if (allEnd())
		{
			m_nMode = 0;
			for (uint i = 0; i < 3; i++) { m_nCombo = 0; SetVisible(i, false); }
		}
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// ---- Gage ----

// DOL 0x8012e900. UNVERIFIED rows: layer 1 of multiplayer is the same for every player (read from the branch table)
int cTaskGrobdaConsoleGage::AnimNo(uint layer, int mode) const
{
	const bool multi = cG11GrobdaSys::Instance()->IsMulti();
	if (layer == 0)
	{
		if (mode < 0 || mode > 2) return -1;
		if (!multi)
		{
			static const int A[3] = {0x5e, 0x5f, 0x60};
			return A[mode];
		}
		static const int M[4][3] = {{0x84, 0x85, 0x86}, {0x8c, 0x8d, 0x8e}, {0x8f, 0x90, 0x91}, {0x92, 0x93, 0x94}};
		return m_nPlayer >= 0 && m_nPlayer < 4 ? M[m_nPlayer][mode] : -1;
	}
	if (layer == 1)
	{
		if (mode < 0 || mode > 4) return -1;
		static const int S[5] = {0x61, 0x63, 0x65, 0x62, 0x64};
		static const int M[5] = {0x87, 0x89, 0x8b, 0x88, 0x8a};
		return multi ? M[mode] : S[mode];
	}
	return -1;
}

// the "state &= ~0x20000; +0x14 = 1.0" the DOL repeats on layer 1 (the bar)
void cTaskGrobdaConsoleGage::ClearHold()
{
	pAnita(1)->SetState(0x20000u, false);
	pAnita(1)->SetFrameRate(1.0f);   // FLOAT_8047c0d0
}

// DOL 0x8012e358
cTaskGrobdaConsoleGage::cTaskGrobdaConsoleGage(int player)
{
	SetName("cTaskGrobdaConsoleGage");
	m_nPlayer = player;
	m_fGauge = 1.0f;
	const int pat = cG11GrobdaSys::Instance()->IsMulti() ? player + 0x12 : 3;
	for (uint i = 0; i < 2; i++)
	{
		pAnita(i)->RegistData("m11c_game01.anp");
		SetVisible(i, false);
		PlaceLayer(i, 0, pat, false);
	}
}

// DOL 0x8012e8d0
void cTaskGrobdaConsoleGage::SetGauge(float f)
{
	m_fGauge = f > 1.0f ? 1.0f : (f < 0.0f ? 0.0f : f);
}

// DOL 0x8012e728
void cTaskGrobdaConsoleGage::Start()
{
	m_nMode = 1;
	ClearHold();
	pAnita(0)->SetAnimNo(AnimNo(0, 0), false);
	pAnita(1)->SetAnimNo(AnimNo(1, 0), false);
	SetVisible(0, true);
	SetVisible(1, true);
}

// DOL 0x8012e818
void cTaskGrobdaConsoleGage::End()
{
	m_nMode = 3;
	ClearHold();
	pAnita(0)->SetAnimNo(AnimNo(0, 2), false);
	pAnita(1)->SetAnimNo(AnimNo(1, 2), false);
}

// DOL 0x8012e484
bool cTaskGrobdaConsoleGage::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	if (m_nMode == 1)
	{
		ClearHold();
		if (IsAnimEnd(0) && IsAnimEnd(1))
		{
			m_nMode = 2;
			pAnita(0)->SetAnimNo(AnimNo(0, 1), false);
			pAnita(1)->SetAnimNo(AnimNo(1, 1), false);
		}
	}
	else if (m_nMode == 2)
	{
		// the bar: anim 3 of layer 1, held at (length * gauge) (FUN_80141bd4 = GetFrameNum, FUN_80141c44(0) = AnimUpdate(0): UNVERIFIED mapping)
		const int a = AnimNo(1, 3);
		pAnita(1)->SetState(0x20000u, false);
		pAnita(1)->SetFrameRate(0.0f);   // FLOAT_8047c0d4
		pAnita(1)->SetAnimNo(a, false);
		pAnita(1)->SetFrame(pAnita(0)->GetFrameNum(a) * m_fGauge);
		pAnita(1)->AnimUpdate(0.0f);
	}
	else if (m_nMode == 3)
	{
		ClearHold();
		if (IsAnimEnd(0) && IsAnimEnd(1))
		{
			m_nMode = 4;
			SetVisible(0, false);
			SetVisible(1, false);
		}
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// ---- Icon (multiplayer) ----

// DOL 0x8012f1b4
cTaskGrobdaConsoleIcon::cTaskGrobdaConsoleIcon(int player)
{
	SetName("cTaskGrobdaConsoleIcon");
	m_nMode = 0;
	m_nPlayer = player;
	pAnita(0)->RegistData("m11c_game01.anp");
	SetVisible(0, false);
	// + the cTaskFaceIcon child (+0x1d0, hidden: +0x7c = 0) with the player's Mii (sys player block 0x8000ed90 +0x44 / +0x48), at layer 0
	// pattern player + 10, scale (0.5, 0.5): NOT PORTED (RFL)
}

// DOL 0x8012f5d0
void cTaskGrobdaConsoleIcon::Start()
{
	m_nMode = 1;
	pAnita(0)->SetAnimNo(m_nPlayer * 3 + 0x4f, true);
	SetVisible(0, true);
}

// DOL 0x8012f644
void cTaskGrobdaConsoleIcon::End()
{
	m_nMode = 3;
	pAnita(0)->SetAnimNo(m_nPlayer * 3 + 0x51, true);
	// face icon +0x7c = 0 (NOT PORTED)
}

// DOL 0x8012f498
bool cTaskGrobdaConsoleIcon::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	if (m_nMode == 1)
	{
		if (IsAnimEnd(0))
		{
			m_nMode = 2;
			pAnita(0)->SetAnimNo(m_nPlayer * 3 + 0x50, true);
			// face icon +0x7c = 1 (NOT PORTED)
		}
	}
	else if (m_nMode == 3 && IsAnimEnd(0))
	{
		m_nMode = 4;
		SetVisible(0, false);
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// ---- Result (multiplayer) ----

// DOL 0x80131288
cTaskGrobdaConsoleResult::cTaskGrobdaConsoleResult()
{
	SetName("cTaskGrobdaConsoleResult");
	m_nMode = 0;
	m_bDraw = false;
	m_nWait = 0;
	for (uint i = 0; i < 5; i++)
	{
		pAnita(i)->RegistData("m11c_game01.anp");
		SetVisible(i, false);
	}
	// + a cTaskFaceIcon child (+0x1dc) for the winners' Miis: NOT PORTED (RFL)
}

// DOL 0x80131604
void cTaskGrobdaConsoleResult::Start(const int* scores)
{
	const uint count = (uint)cG11GrobdaSys::Instance()->nPlayer();
	m_nMode = 1;
	int top = 0;
	for (uint i = 0; i < count; i++)
		if (top < scores[i]) top = scores[i];
	m_vWinner.clear();
	for (uint i = 0; i < count; i++)
		if (scores[i] == top) m_vWinner.push_back((int)i);
	const uint winners = (uint)m_vWinner.size();
	pAnita(0)->SetAnimNo(0x5b, true);
	SetVisible(0, true);
	if (count == winners)
	{
		m_bDraw = true;
		pAnita(1)->SetAnimNo(0x40, true);
		SetVisible(1, true);
	}
	else
	{
		pAnita(1)->SetAnimNo(0x3d, true);
		SetVisible(1, true);
		for (uint i = 0; i < winners; i++)
		{
			pAnita(i + 2)->SetAnimNo(m_vWinner[i] * 3 + 0x43, true);
			PlaceLayer(i + 2, i, (int)(winners + 0x16), false);   // FUN_80014408(anita, anita +0x400, i, winners + 0x16)
			SetVisible(i + 2, true);
			// the winner's Mii face at layer i pattern winners + 0x19, scale 0.5: NOT PORTED
		}
	}
}

// DOL 0x80131d7c
void cTaskGrobdaConsoleResult::Hold()
{
	m_nMode = 2;
	m_nWait = 180;   // DAT_80477ed8
	pAnita(0)->SetAnimNo(0x5c, true);
	if (!m_bDraw)
	{
		pAnita(1)->SetAnimNo(0x3e, true);
		for (uint i = 0; i < m_vWinner.size(); i++) pAnita(i + 2)->SetAnimNo(m_vWinner[i] * 3 + 0x44, true);   // + face icon i shown
	}
	else
		pAnita(1)->SetAnimNo(0x41, true);
}

// DOL 0x80131c7c
void cTaskGrobdaConsoleResult::End()
{
	m_nMode = 3;
	pAnita(0)->SetAnimNo(0x5d, true);
	if (!m_bDraw)
	{
		pAnita(1)->SetAnimNo(0x3f, true);
		for (uint i = 0; i < m_vWinner.size(); i++) pAnita(i + 2)->SetAnimNo(m_vWinner[i] * 3 + 0x45, true);   // + face icon i hidden
	}
	else
		pAnita(1)->SetAnimNo(0x42, true);
}

// DOL 0x8013147c
bool cTaskGrobdaConsoleResult::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	if (m_nMode == 1)
	{
		bool end = true;
		for (uint i = 0; end && i < 3; i++) end = IsAnimEnd(i);
		if (end) Hold();
	}
	else if (m_nMode == 2)
	{
		if (--m_nWait < 0) End();   // vtable +0x4c
	}
	else if (m_nMode == 3)
	{
		bool end = true;
		for (uint i = 0; end && i < 3; i++) end = IsAnimEnd(i);
		if (end)
			for (uint i = 0; i < 5; i++)
			{
				m_nMode = 4;
				SetVisible(i, false);
			}
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

// ---- Manager ----

// DOL 0x8012f7c8 (single player branch; priorities as the builder sets them)
cTaskGrobdaConsoleManager::cTaskGrobdaConsoleManager(int nPlayer)
{
	SetName("cTaskGrobdaConsoleManager");
	m_pReadyGo = new cTaskGrobdaConsoleReadyGo();
	m_pReadyGo->nDrawPriority(0x80000008u);
	ConnectChild(m_pReadyGo);
	m_pFinish = new cTaskGrobdaConsoleFinish();
	m_pFinish->nDrawPriority(0x80000009u);
	ConnectChild(m_pFinish);
	m_pTime = new cTaskGrobdaConsoleTime();
	m_pTime->nDrawPriority(0x80000006u);
	ConnectChild(m_pTime);
	for (int i = 0; i < 4 && i < nPlayer; i++)
	{
		m_pScore[i] = new cTaskGrobdaConsoleScore(i);
		m_pScore[i]->nDrawPriority(0x80000003u);
		ConnectChild(m_pScore[i]);
		m_pGage[i] = new cTaskGrobdaConsoleGage(i);
		m_pGage[i]->nDrawPriority(0x80000005u);
		ConnectChild(m_pGage[i]);
	}
	if (!cG11GrobdaSys::Instance()->IsMulti())   // FUN_8000ed74: high score (+0x210) and combo (+0x214) are single player only
	{
		m_pHigh = new cTaskGrobdaConsoleHighScore();
		m_pHigh->nDrawPriority(0x80000004u);
		ConnectChild(m_pHigh);
		m_pCombo = new cTaskGrobdaConsoleCombo();
		m_pCombo->nDrawPriority(0x80000007u);
		ConnectChild(m_pCombo);
	}
	else   // the result console (+0x1dc, priority 0x8000000a; its face icon 0x8000000b) and an icon panel per player (+0x1e0.., 0x80000002)
	{
		m_pResult = new cTaskGrobdaConsoleResult();
		m_pResult->nDrawPriority(0x8000000au);
		ConnectChild(m_pResult);
		for (int i = 0; i < 4 && i < nPlayer; i++)
		{
			m_pIcon[i] = new cTaskGrobdaConsoleIcon(i);
			m_pIcon[i]->nDrawPriority(0x80000002u);
			ConnectChild(m_pIcon[i]);
		}
	}
	pAnita(0)->RegistData("m11c_game01.anp");
	SetVisible(0, false);
	nDrawPriority(0x80000001u);
}

// DOL 0x80130a5c
void cTaskGrobdaConsoleManager::Start()
{
	m_nMode = 1;
	SetVisible(0, true);
	pAnita(0)->SetAnimNo(cG11GrobdaSys::Instance()->IsMulti() ? 0x2f : 0x2c, true);
	m_pTime->Start();
	for (int i = 0; i < 4; i++)
	{
		if (m_pScore[i]) m_pScore[i]->Start();
		if (m_pGage[i]) m_pGage[i]->Start();
		if (m_pIcon[i]) m_pIcon[i]->Start();
	}
	if (m_pHigh) m_pHigh->Start();
}

// DOL 0x80130b84
void cTaskGrobdaConsoleManager::End()
{
	m_nMode = 5;
	pAnita(0)->SetAnimNo(cG11GrobdaSys::Instance()->IsMulti() ? 0x31 : 0x2e, true);
	m_pTime->End();
	for (int i = 0; i < 4; i++)
	{
		if (m_pScore[i]) m_pScore[i]->End();
		if (m_pGage[i]) m_pGage[i]->End();
		if (m_pIcon[i]) m_pIcon[i]->End();
	}
	if (m_pHigh) m_pHigh->End();
}

// DOL 0x80130ca8
void cTaskGrobdaConsoleManager::Finish()
{
	m_nMode = 3;
	m_nFinishWait = 0x3c;
	m_pFinish->Start();
}

// DOL 0x80130c94
void cTaskGrobdaConsoleManager::StartReadyGo()
{
	m_pReadyGo->Start();
}

// DOL 0x80130d08
bool cTaskGrobdaConsoleManager::IsReadyDone() const
{
	return m_pReadyGo->Mode() > 2 && m_pFinish->Mode() < 1;
}

// DOL 0x80130778
bool cTaskGrobdaConsoleManager::Execute(cJetTask* pParam)
{
	if (!Running()) return true;
	if (m_nMode == 1)
	{
		if (IsAnimEnd(0))
		{
			m_nMode = 2;
			pAnita(0)->SetAnimNo(cG11GrobdaSys::Instance()->IsMulti() ? 0x30 : 0x2d, true);
		}
	}
	else if (m_nMode == 3)
	{
		if (!cG11GrobdaSys::Instance()->IsMulti())
		{
			if (!m_bHighChecked && --m_nFinishWait < 0)
			{
				m_bHighChecked = true;
				const int score = m_pScore[0]->nScore();
				if (m_pHigh->nHigh() < score)
				{
					m_pScore[0]->Flash();   // FUN_80132524
					m_pHigh->SetNew(score);
				}
			}
			if (m_bHighChecked && m_pFinish->Mode() == 2 && (GamePad::Get(0).trig & 0x860u) != 0) m_pFinish->End();
		}
		if (m_pFinish->Mode() == 4)
		{
			if (!cG11GrobdaSys::Instance()->IsMulti()) End();   // vtable +0x4c
			else
			{
				m_nMode = 4;
				int scores[4];
				for (int i = 0; i < 4; i++) scores[i] = m_pScore[i] != nullptr ? m_pScore[i]->nScore() : -1;   // +0x200.. +0x204
				m_pResult->Start(scores);   // FUN_80131604
			}
		}
	}
	else if (m_nMode == 4)
	{
		if (m_pResult->Mode() == 4) End();   // vtable +0x4c
	}
	else if (m_nMode == 5)
	{
		if (IsAnimEnd(0)) { m_nMode = 6; SetVisible(0, false); }
	}
	return cTaskGrobdaConsoleBase::Execute(pParam);
}

}
