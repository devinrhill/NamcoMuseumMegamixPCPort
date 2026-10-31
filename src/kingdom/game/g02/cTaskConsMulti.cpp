#include "kingdom/game/g02/cTaskConsMulti.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "gfx/FontWriter.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cPBView.hpp"
#include "kingdom/game/g02/cParamGame.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"

// Rally-X Remix (g02) the multiplayer HUD, ported 1:1 from main.dol (no C# exists for Rally-X).
namespace kingdom::game::g02
{

namespace
{
const char* const CAR01 = "ANITA_CAR01";       // crc 0x9d901d89
const char* const RESULT = "ANITA_RESULT";     // crc 0x7bc6cf12

// the freeze test of every part of this file (the DOL spells it out in each Execute): the app paused / stage end / HOME held
bool Frozen(const cTaskConsole* c) { return c != nullptr && c->IsFrozen(); }

constexpr int DIGIT_ANM = 0x24;                // table 0x8036b9a0: digit d = 0x24 + 3 d
}

// ---- cTaskConsPBView ----------------------------------------------------------------------------------------------------------------------

// the cTaskPBAnita ctor (DOL 0x80064afc [audited] ff.: priority 0x8000000, hidden flag 0x6d = 0)
cTaskConsPBView::cTaskConsPBView(cTaskConsole* pConsole, int nCamera) : m_pConsole(pConsole), m_nCamera(nCamera)
{
	SetName("kingdom::cTaskPBAnita");
	nDrawPriority(0x8000000u);   // the push-back priority: after the scene (131072), before the HUD (0x80000001..)
}

kingdom::cKingdomAnita* cTaskConsPBView::Regist(uint idx, const char* name, int anim)
{
	kingdom::cKingdomAnita* a = pAnita(idx);
	a->RegistData(name);
	a->SetAnimNo(anim, true, 0.0f);
	a->vTrans()[0] = 0.0f;
	a->vTrans()[1] = 0.0f;
	a->vTrans()[2] = 0.0f;
	return a;
}

// DOL 0x80123320 [audited] (PBMultiNO) / 0x80066cdc (BG): unless HOME is held, the layout steps while the app is not paused / not at the stage end
bool cTaskConsPBView::Execute(cJetTask* pParam)
{
	(void)pParam;
	if ((GamePad::Get(0).hold & GamePad::PAD_HOME) == 0 && !Frozen(m_pConsole)) AnitaVector->UpdateFrame();   // 0x80014d58
	return true;
}

// DOL 0x801235cc [audited]
void cTaskConsPBView::Draw(int nStatus, cJetTask* pParam)
{
	ScopedPBView view(m_nCamera);
	if (!view.ok()) return;
	cTaskAnita::Draw(nStatus, pParam);
}

// ---- cTaskConsPBMultiNO / BG --------------------------------------------------------------------------------------------------------------

// DOL 0x80064afc: anim {0x128, 0x129, 0x12a, 0x12b}[player] (table 0x8036be38), the layer not animating (state bit 2 cleared) until the start
cTaskConsPBMultiNO::cTaskConsPBMultiNO(cTaskConsole* pConsole, int nPlayer) : cTaskConsPBView(pConsole, nPlayer)
{
	SetName("g02::cTaskConsPBMultiNO");
	static const int ANM[4] = {0x128, 0x129, 0x12a, 0x12b};
	Regist(0, CAR01, ANM[nPlayer & 3]);
	pAnita(0)->SetState(2u, false);
}

// DOL 0x80066b5c [audited]: the plate starts to play in the start signal (app state 2) and hides when it has played
bool cTaskConsPBMultiNO::Execute(cJetTask* pParam)
{
	if (m_pConsole->nState() == 2) pAnita(0)->SetState(2u, true);
	if (pAnita(0)->IsState(0x10000u)) pAnita(0)->SetState(1u, false);
	return cTaskConsPBView::Execute(pParam);
}

// DOL 0x80065b24 [audited]: ANITA_CAR01 anim 0x117 in camera nCamera (3)
cTaskConsPBMultiBG::cTaskConsPBMultiBG(cTaskConsole* pConsole, int nCamera) : cTaskConsPBView(pConsole, nCamera)
{
	SetName("g02::cTaskConsPBMultiBG");
	Regist(0, CAR01, 0x117);
}

// ---- cTaskConsInfoMulti -------------------------------------------------------------------------------------------------------------------

namespace
{
// the anita numbers of the per-player parts (tables 0x8036ba30 ..): the car mark, the leader mark, the plate, the flag tens / ones, the
// nitro mark, the hit mark
const uint ANITA_MARK[4] = {1, 2, 3, 4};
const uint ANITA_LEAD[4] = {0x11, 0x12, 0x13, 0x14};
const uint ANITA_PLATE[4] = {5, 6, 7, 8};
const uint ANITA_TENS[4] = {9, 10, 11, 12};
const uint ANITA_ONES[4] = {13, 14, 15, 16};
const uint ANITA_NITRO[4] = {0x15, 0x16, 0x17, 0x18};
const uint ANITA_HIT[4] = {0x19, 0x1a, 0x1b, 0x1c};
constexpr uint ANITA_HINT = 0x1d;

// the layout patterns by player count (tables PTR 0x8036baa0 .. 0x8036bb00): index N (2..4) -> the pattern of each player
const int PAT_MARK[5][4] = {{}, {}, {0xc, 0xd}, {0x21, 0x22, 0x23}, {0x21, 0x22, 0x23, 0x2e}};            // 0x80476d40 / 0x80390f28 / 0x80390f38
const int PAT_LEAD[5][4] = {{}, {}, {0x1e, 0x1f}, {0x1e, 0x2c, 0x1f}, {0x1e, 0x2c, 0x1f, 0x37}};          // 0x80476d68 / 0x80390fa8 / 0x80390fb8
const int PAT_PLATE[5][4] = {{}, {}, {0x1c, 0x1d}, {0x1c, 0x2b, 0x1d}, {0x1c, 0x2b, 0x1d, 0x36}};         // 0x80476d58 / 0x80390f68 / 0x80390f78
const int PAT_DIGIT[5][4] = {{}, {}, {0x49, 0x4a}, {0x49, 0x4b, 0x4a}, {0x49, 0x4b, 0x4a, 0x4c}};         // 0x80476d60 / 0x80390f88 / 0x80390f98
const int PAT_NITRO[5][4] = {{}, {}, {0x4e, 0x4e}, {0x4f, 0x4f, 0x4f}, {0x4f, 0x4f, 0x4f, 0x4f}};         // 0x80476d70 / 0x80390fc8
const int FRAME_ANM[5] = {0, 0x92, 0x108, 0x10b, 0x10e};                                                  // 0x8036b9c8 [players]
}

// DOL 0x80064d68 [audited] (the cTaskRallyAnitaBase part first): ANITA_CAR01, every anita hidden
cTaskConsInfoMulti::cTaskConsInfoMulti(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsInfoMulti");
	nDrawPriority(0x80000001u);
	m_nMode = 0;
	m_bHint = false;
	m_nPlayerNum = cTaskConsole::SysPlayerNum();   // sys 0x800c5c68
	struct sInit
	{
		uint idx;
		int anim;
	};
	static const sInit INIT[] = {{0, 0},    {1, 0xfc},  {2, 0xff},  {3, 0x102}, {4, 0x105}, {5, 0xa3},  {6, 0xa3},  {7, 0xa3},  {8, 0xa3},
	                             {9, 0x24}, {0xd, 0x24}, {10, 0x24}, {0xe, 0x24}, {0xb, 0x24}, {0xf, 0x24}, {0xc, 0x24}, {0x10, 0x24},
	                             {0x11, 0x111}, {0x12, 0x111}, {0x13, 0x111}, {0x14, 0x122}, {0x15, 0x122}, {0x16, 0x122}, {0x17, 0x122},
	                             {0x18, 0x122}, {0x19, 0x122}, {0x1a, 0x122}, {0x1b, 0x122}, {0x1c, 0x122}, {0x1d, 0x125}};
	for (const sInit& i : INIT)
	{
		Regist(i.idx, CAR01, i.anim);
		Show(i.idx, false);
	}
}

// DOL 0x800666e8 [audited]: the hit mark of a player shows the player's nitro state clip {0x121, 0x124, 0x11e}[state]
void cTaskConsInfoMulti::ShowHit(int nPlayer)
{
	if ((uint)nPlayer >= 4u) return;
	static const int ANM[3] = {0x121, 0x124, 0x11e};   // 0x8036ba20
	const uint a = ANITA_HIT[nPlayer];
	pAnita(a)->SetAnimNo(ANM[(uint)m_Player[nPlayer].nNitro < 3u ? m_Player[nPlayer].nNitro : 0], true);
	pAnita(a)->SetState(1u, true);
}

// DOL 0x80069fa8 [audited]. Only the pause byte gates it (the DOL tests KingSys app +0x1b4 == 1).
bool cTaskConsInfoMulti::Execute(cJetTask* pParam)
{
	if (m_pConsole->IsPaused()) return true;
	const int n = m_nPlayerNum;
	switch (m_nMode)
	{
	case 0:
		// every part gets the layout position of its player (the tables are by the player count; 2..4 players)
		if (n >= 2 && n <= 4)
		{
			for (int i = 0; i < n; i++)
			{
				SetPosLayer(ANITA_MARK[i], PAT_MARK[n][i], 0);
				SetPosLayer(ANITA_LEAD[i], PAT_LEAD[n][i], 0);
				SetPosLayer(ANITA_PLATE[i], PAT_PLATE[n][i], 0);
				SetPosLayer(ANITA_TENS[i], PAT_DIGIT[n][i], 1);
				SetPosLayer(ANITA_ONES[i], PAT_DIGIT[n][i], 0);
				SetPosLayer(ANITA_NITRO[i], PAT_NITRO[n][i], (uint)i);
				SetPosLayer(ANITA_HIT[i], PAT_NITRO[n][i], (uint)i);
			}
		}
		m_nMode++;
		break;
	case 1:   // from the start signal on: the frame of the player count
		if (1 < m_pConsole->nState())
		{
			pAnita(0)->SetAnimNo(FRAME_ANM[n < 5 ? n : 4], true);
			Show(0, true);
			m_nMode++;
		}
		break;
	case 2:
		if (IsEnd(0))
		{
			for (int i = 0; i < n; i++)
			{
				pAnita(ANITA_MARK[i])->SetAnimNo(0xfc + 3 * i, true);
				pAnita(ANITA_LEAD[i])->SetAnimNo(DIGIT_ANM, true);
				pAnita(ANITA_PLATE[i])->SetAnimNo(0xa3, true);
				pAnita(ANITA_TENS[i])->SetAnimNo(DIGIT_ANM, true);
				pAnita(ANITA_ONES[i])->SetAnimNo(DIGIT_ANM, true);
				pAnita(ANITA_NITRO[i])->SetAnimNo(0x122, true);
				pAnita(ANITA_HIT[i])->SetAnimNo(0x122, true);
				Show(ANITA_MARK[i], true);
				Show(ANITA_LEAD[i], true);
				Show(ANITA_PLATE[i], true);
				Show(ANITA_TENS[i], true);
				Show(ANITA_ONES[i], true);
				Show(ANITA_NITRO[i], true);
			}
			m_nMode++;
		}
		if (cTaskConsole::SysPlayerNum() == 4)   // the hint is shown at once (and every frame until the frame has played)
		{
			Show(ANITA_HINT, true);
			pAnita(ANITA_HINT)->SetAnimNo(0x125, true);
		}
		break;
	case 3:
	{
		// the flag counts: tens and ones digit clips follow the car's flags
		for (int i = 0; i < n; i++)
		{
			sConsCar c;
			cTaskConsole::CarState(m_Player[i].pCar, c);
			const int flags = c.nFlags;
			if (m_Player[i].nFlags != flags)
			{
				m_Player[i].nFlags = flags;
				pAnita(ANITA_TENS[i])->SetAnimNo(DIGIT_ANM + 3 * ((flags / 10) % 10), true);
				pAnita(ANITA_ONES[i])->SetAnimNo(DIGIT_ANM + 3 * (flags % 10), true);
			}
		}
		// the nitro marks: the clip of a new state, then its loop clip
		static const int NITRO_START[3] = {0x11f, 0x122, 0x11c};   // 0x8036bb04
		static const int NITRO_LOOP[3] = {0x120, 0x123, 0x11d};    // 0x8036bb10
		for (int i = 0; i < n; i++)
		{
			sConsCar c;
			cTaskConsole::CarState(m_Player[i].pCar, c);
			const int state = c.nNitroState;
			if (m_Player[i].nNitro == state)
			{
				if (IsEnd(ANITA_NITRO[i])) SetAnim(ANITA_NITRO[i], NITRO_LOOP[(uint)state < 3u ? state : 0]);
			}
			else
			{
				m_Player[i].nNitro = state;
				pAnita(ANITA_NITRO[i])->SetAnimNo(NITRO_START[(uint)state < 3u ? state : 0], true);
			}
		}
		// the leaders (most flags; equal counts share): mark 0x111, the others 0x113
		int best = m_Player[0].nFlags;
		u32 mask = 1u;
		for (int p = 1; p < n; p++)
		{
			const int v = m_Player[p].nFlags;
			if (best == v)
				mask |= 1u << p;
			else if (best < v)
			{
				mask = 1u << p;
				best = v;
			}
		}
		for (int i = 0; i < n; i++) SetAnim(ANITA_LEAD[i], (mask & (1u << i)) != 0 ? 0x111 : 0x113);
		if (cTaskConsole::SysPlayerNum() == 4)
		{
			// the "hold B" hint: shown until a pad holds B (0x40), hidden in the result states
			m_bHint = false;
			for (int i = 0; i < 4; i++)
				if ((GamePad::Get(i).hold & 0x40u) != 0) m_bHint = true;
			const int st = m_pConsole->nState();
			if (st == 8 || st == 9) m_bHint = true;
			kingdom::cKingdomAnita* h = pAnita(ANITA_HINT);
			if (h->IsState(1u))
			{
				if (!m_bHint)
				{
					const int anm = h->GetAnimNo();
					if (anm == 0x125)
					{
						if (h->IsState(0x10000u)) SetAnim(ANITA_HINT, 0x126);
					}
					else if (anm == 0x126)
						h->SetState(1u, true);
					else if (anm == 0x127)
						SetAnim(ANITA_HINT, 0x125);
				}
				else if ((uint)(h->GetAnimNo() - 0x125) < 2u)
				{
					SetAnim(ANITA_HINT, 0x127);
				}
			}
		}
		break;
	}
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// ---- cTaskConsWinner / cTaskConsDraw ----------------------------------------------------------------------------------------------------

namespace
{
// one name of the font writer: the shadow (the player's shadow colour, one pixel right / down), then the name; the writer's position is the
// anita layer's (screen = centre + x, centre - y). UNVERIFIED: centred (the DOL's writer flags are not traced), RESIDENT_FONT_MII at its size.
template <class T>
void QueueName(std::vector<T>& v, const cTaskConsole* con, int player, float x, float y)
{
	const cParamGame* g = con->pParam();
	const std::string& name = cG02RallyXSysExt::szMatchName[player & 3];
	const u32 color = g != nullptr ? g->m_nFontColor[player & 3] : 0xffffffffu;
	const u32 shadow = g != nullptr ? g->m_nFontShadow[player & 3] : 0x000000ffu;
	v.push_back({(float)(int)(1.0f + x), (float)(int)(1.0f + y), name, shadow});
	v.push_back({(float)(int)x, (float)(int)y, name, color});
}

template <class T>
void DrawNames(const std::vector<T>& v)
{
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_MII";
	st.vorigin = gfx::TextStyle::VOrigin::Middle;
	for (const T& t : v)
	{
		st.color = t.color;
		gfx::DrawText(t.x, t.y, t.text, st, gfx::TextAlign::Center);
	}
}
}

// DOL 0x8006546c [audited]: ANITA_RESULT anitas 0..4, hidden
cTaskConsWinner::cTaskConsWinner(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsWinner");
	nDrawPriority(0x80000005u);
	m_nMode = 0;
	m_fTimer = 0.0f;
	for (uint i = 0; i < 5; i++)
	{
		Regist(i, RESULT, 0);
		Show(i, false);
	}
}

// DOL 0x800679e8 [audited]
void cTaskConsWinner::Draw(int nStatus, cJetTask* pParam)
{
	cTaskRallyAnitaBase::Draw(nStatus, pParam);
	DrawNames(m_Texts);
	m_Texts.clear();   // 0x800172c0 empties the queue
}

// DOL 0x80067a24 [audited] (the freeze test is skipped in mode 4: it comes after the names are queued)
bool cTaskConsWinner::Execute(cJetTask* pParam)
{
	if (m_nMode != 4 && Frozen(m_pConsole)) return true;
	static const int LABEL_START[4] = {0x2a, 0x2d, 0x30, 0x33};   // 0x8036bde8
	static const int LABEL_LOOP[4] = {0x2b, 0x2e, 0x31, 0x34};    // 0x8036bdf8
	static const int LABEL_END[4] = {0x2c, 0x2f, 0x32, 0x35};     // 0x8036be08
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	const int winner = sys->nFirstWinner(cTaskConsole::SysPlayerNum());   // 0x800c58d0
	switch (m_nMode)
	{
	case 0:
		if (m_pConsole->nState() == 8)
		{
			pAnita(0)->SetAnimNo(0x3c, true);
			Show(0, true);
			pAnita(3)->SetAnimNo(0x1e, true);
			Show(3, true);
			m_nMode++;
		}
		break;
	case 1:
		if (IsEnd(0) && IsEnd(3))
		{
			pAnita(0)->SetAnimNo(0x3d, true);
			pAnita(3)->SetAnimNo(0x1f, true);
			m_nMode = 2;
			m_fTimer = 0.0f;
		}
		break;
	case 2:
		if (cTaskUnit::Inc1(m_pConsole->fWinnerFlagWait(), m_fTimer))   // app +0x280 +0x3c
		{
			Show(1, true);
			Show(2, true);
			Show(4, true);
			pAnita(1)->SetAnimNo(0x39, true);
			pAnita(2)->SetAnimNo(0x36, true);
			pAnita(3)->SetAnimNo(0x20, true);
			pAnita(4)->SetAnimNo(LABEL_START[winner & 3], true);
			SetPosLayer(4, 2, 0);
			m_nMode++;
		}
		break;
	case 3:
		if (IsEnd(1) && IsEnd(2) && IsEnd(4))
		{
			pAnita(1)->SetAnimNo(0x3a, true);
			pAnita(2)->SetAnimNo(0x37, true);
			pAnita(4)->SetAnimNo(LABEL_LOOP[winner & 3], true);
			m_nMode = 4;
		}
		break;
	case 4:
	{
		// the winner's name at layer 0 of pattern 3 of the label anita: the screen position = centre + (x, -y)
		Vector4 pos{};
		GetLayerPos(pos, 4, 3, 0);
		QueueName(m_Texts, m_pConsole, winner, 0.5f * 640.0f + pos[0], 0.5f * 480.0f - pos[1]);
		if (Frozen(m_pConsole)) return true;
		break;
	}
	case 5:   // the leaving clips (nothing starts it: the app leaves at mode 4)
		pAnita(0)->SetAnimNo(0x3e, true);
		pAnita(1)->SetAnimNo(0x3b, true);
		pAnita(2)->SetAnimNo(0x38, true);
		pAnita(3)->SetAnimNo(0x20, true);
		pAnita(4)->SetAnimNo(LABEL_END[winner & 3], true);
		m_nMode++;
		break;
	case 6:
		if (IsEnd(0) && IsEnd(1) && IsEnd(2) && IsEnd(3) && IsEnd(4)) m_nMode++;
		break;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

// DOL 0x8006579c [audited]: ANITA_RESULT anitas 0..6, hidden
cTaskConsDraw::cTaskConsDraw(cTaskConsole* pConsole) : cTaskRallyAnitaBase(pConsole)
{
	SetName("g02::cTaskConsDraw");
	nDrawPriority(0x80000005u);
	m_nMode = 0;
	m_fTimer = 0.0f;
	for (uint i = 0; i < 7; i++)
	{
		Regist(i, RESULT, 0);
		Show(i, false);
	}
}

// DOL 0x80067190 [audited]
void cTaskConsDraw::Draw(int nStatus, cJetTask* pParam)
{
	cTaskRallyAnitaBase::Draw(nStatus, pParam);
	DrawNames(m_Texts);
	m_Texts.clear();
}

// DOL 0x800671cc [audited]
bool cTaskConsDraw::Execute(cJetTask* pParam)
{
	if (m_nMode != 4 && Frozen(m_pConsole)) return true;
	static const int LABEL_START[4] = {0x2a, 0x2d, 0x30, 0x33};   // 0x8036be18
	static const int LABEL_LOOP[4] = {0x2b, 0x2e, 0x31, 0x34};    // 0x8036be28
	cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	const int n = cTaskConsole::SysPlayerNum();
	switch (m_nMode)
	{
	case 0:
		if (m_pConsole->nState() == 9)
		{
			pAnita(0)->SetAnimNo(0x3c, true);
			Show(0, true);
			pAnita(2)->SetAnimNo(0x1e, true);
			Show(2, true);
			m_nMode++;
		}
		break;
	case 1:
		if (IsEnd(0) && IsEnd(2))
		{
			pAnita(0)->SetAnimNo(0x3d, true);
			pAnita(2)->SetAnimNo(0x1f, true);
			m_nMode = 2;
			m_fTimer = 0.0f;
		}
		break;
	case 2:
		if (cTaskUnit::Inc1(m_pConsole->fDrawFlagWait(), m_fTimer))   // app +0x280 +0x40
		{
			pAnita(2)->SetAnimNo(0x20, true);
			pAnita(1)->SetAnimNo(0x21, true);
			Show(1, true);
			uint k = 0;
			for (int p = 0; p < n; p++)
			{
				if (!sys->IsPlayerActive(p)) continue;   // 0x800c58b0: the tied players
				Show(3 + k, true);
				pAnita(3 + k)->SetAnimNo(LABEL_START[p & 3], true);
				SetPosLayer(3 + k, n == 3 ? 8 : 5, k);
				k++;
			}
			m_nMode++;
		}
		break;
	case 3:
		if (IsEnd(1))
		{
			pAnita(1)->SetAnimNo(0x22, true);
			uint k = 0;
			for (int p = 0; p < n; p++)
			{
				if (!sys->IsPlayerActive(p)) continue;
				pAnita(3 + k)->SetAnimNo(LABEL_LOOP[p & 3], true);
				k++;
			}
			m_nMode = 4;
		}
		break;
	case 4:
	{
		uint k = 0;
		for (int p = 0; p < n; p++)
		{
			if (!sys->IsPlayerActive(p)) continue;
			Vector4 pos{};
			GetLayerPos(pos, 3 + k, n == 3 ? 9 : 6, k);
			QueueName(m_Texts, m_pConsole, p, 0.5f * 640.0f + pos[0], 0.5f * 480.0f - pos[1]);
			k++;
		}
		if (Frozen(m_pConsole)) return true;
		break;
	}
	case 6:
		pAnita(0)->SetAnimNo(0x3e, true);
		m_nMode++;
		break;
	case 7:
		if (IsEnd(0) && IsEnd(1) && IsEnd(2)) m_nMode++;
		break;
	}
	return cTaskRallyAnitaBase::Execute(pParam);
}

}
