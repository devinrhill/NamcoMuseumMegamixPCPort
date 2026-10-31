#include "kingdom/game/cTaskConsRetry.hpp"

#include <cmath>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "GamePad.hpp"
#include "gfx/Gfx.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "snd/Mixer.hpp"
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>

namespace kingdom::game
{

namespace
{
constexpr uint ANIM_END = 0x10000u;   // cJetAnita state bit: the animation reached its end
const int CURSOR_IDLE[4] = {0, 2, 4, 6};    // DAT_80379c68
const int CURSOR_PRESS[4] = {1, 3, 5, 7};   // DAT_80379c78
const char* const SE_SELECT = "m00se_01idou";    // DAT_8047c2e0
const char* const SE_DECIDE = "m00se_02Kettei";  // DAT_8047c2e4
}

cTaskConsRetry::eStep& cTaskConsRetry::nStep()
{
	return m_nStep;
}

cTaskConsRetry::eSelect& cTaskConsRetry::nSelectBuf()
{
	return m_nSelectBuf;
}

cTaskConsRetry::eSelect& cTaskConsRetry::nSelect()
{
	return m_nSelect;
}

uint& cTaskConsRetry::nCursorMax()
{
	return m_nCursorMax;
}

cTaskConsRetry::cTaskConsRetry(string retry_key)
{
	Init(retry_key);
	SetInvisible();
}

cTaskConsRetry::cTaskConsRetry(string retry_key, string cursor_key, uint cursor_max)
{
	Init(retry_key);
	CreateCursor(cursor_key, cursor_max);
	SetInvisible();
}

cTaskConsRetry::~cTaskConsRetry()
{
}

bool cTaskConsRetry::IsYes()
{
	return m_nSelect == cTaskConsRetry::eSelect::eSELECT_YES;
}

bool cTaskConsRetry::IsNo()
{
	return m_nSelect == cTaskConsRetry::eSelect::eSELECT_NO;
}

bool cTaskConsRetry::IsNone()
{
	return m_nSelect == cTaskConsRetry::eSelect::eSELECT_NONE;
}

bool cTaskConsRetry::IsVisible()
{
	return m_nStep != cTaskConsRetry::eStep::eSTEP_INVISIBLE;
}

// DOL 0x80148c64 [audited]: the three anitas from the one layout (board anim 0, YES 3, NO 8), at the origin
void cTaskConsRetry::Init(string retry_key)
{
	CustomKingdomUtil::Assert(!retry_key.empty(), "nullpo");
	Setup(2u, retry_key, 0);
	Setup(0u, retry_key, 3);
	Setup(1u, retry_key, 8);
	m_nCursorMax = 0u;
	m_nStep = cTaskConsRetry::eStep::eSTEP_INVISIBLE;
	m_nSelectBuf = cTaskConsRetry::eSelect::eSELECT_NONE;
	m_nSelect = cTaskConsRetry::eSelect::eSELECT_NONE;
	for (kingdom::cTaskCursor*& c : m_pCursor) c = nullptr;
	m_nThroughCnt = 0u;
}

// DOL 0x80148ea4 [audited]: cursor i on pad i (the games set the pad afterwards, e.g. Galaga 0x801b9d0c: the player's work pad), idle anim 2 i,
// A / B / Z (0x860) plays 2 i + 1; a child drawn at 0x8ffffffe
void cTaskConsRetry::CreateCursor(string cursor_key, uint cursor_max)
{
	CustomKingdomUtil::Assert(!cursor_key.empty(), "nullpo");
	CustomKingdomUtil::Assert(cursor_max <= eCURSOR_MAX, "nullpo");
	for (kingdom::cTaskCursor*& c : m_pCursor) c = nullptr;
	m_nCursorMax = 0u;
	for (uint i = 0; i < cursor_max && i < eCURSOR_MAX; i++)
	{
		kingdom::cTaskCursor* c = new kingdom::cTaskCursor();   // 0x800953b0
		c->Init((int)i, cursor_key.c_str(), CURSOR_IDLE[i]);
		c->AddButton(0x860u, CURSOR_PRESS[i]);
		m_pCursor[m_nCursorMax++] = c;
		ConnectChild(c);
		c->nDrawPriority(0x8ffffffeu);
	}
}

// DOL 0x80149018 [audited]
void cTaskConsRetry::SetInvisible()
{
	pAnita(2u)->SetAnimNo(0);
	pAnita(0u)->SetAnimNo(3);
	pAnita(1u)->SetAnimNo(8);
	pAnita(2u)->SetState(1u, false);
	pAnita(0u)->SetState(1u, false);
	pAnita(1u)->SetState(1u, false);
	SetCursorInvisible();
	m_nStep = cTaskConsRetry::eStep::eSTEP_INVISIBLE;
}

bool cTaskConsRetry::IsAnim()
{
	return pAnita(2u)->IsState(ANIM_END) && pAnita(0u)->IsState(ANIM_END) && pAnita(1u)->IsState(ANIM_END);
}

void cTaskConsRetry::Focus(eSelect sel)
{
	const int yes = sel == eSELECT_YES ? 5 : 4, no = sel == eSELECT_NO ? 10 : 9;
	if (pAnita(0u)->GetAnimNo() != yes) pAnita(0u)->SetAnimNo(yes);
	if (pAnita(1u)->GetAnimNo() != no) pAnita(1u)->SetAnimNo(no);
	m_nSelectBuf = sel;
}

cKingdomAnita* cTaskConsRetry::Setup(uint n, string key, int ano, float x, float y, float z)
{
	pAnita(n)->RegistData(key);
	pAnita(n)->SetAnimNo(ano);
	pAnita(n)->vTrans()[0] = x;
	pAnita(n)->vTrans()[1] = y;
	pAnita(n)->vTrans()[2] = z;
	return pAnita(n);
}

void cTaskConsRetry::SetCursorInvisible()
{
	for (uint i = 0; i < m_nCursorMax; i++) m_pCursor[i]->SetEnable(false);   // cursor +0x1e1 = 0
}

void cTaskConsRetry::SetCursorVisible()
{
	for (uint i = 0; i < m_nCursorMax; i++) m_pCursor[i]->SetEnable(true);
}

// DOL 0x80149110 [audited: Execute compared in structure (HOME gate, 20-frame wait, A/B/Z decide, stick/D-pad/pointer select); stick source native]. Nothing at all (not even the anitas) while pad 0 holds HOME.
bool cTaskConsRetry::Execute(cJetTask* pParam)
{
	if ((GamePad::Get(0).hold & GamePad::PAD_HOME) != 0) return true;
	switch (m_nStep)
	{
	case eSTEP_IN:   // all three in-animations ended -> shown (board 1, YES 4, NO 9), nothing focused
		if (IsAnim())
		{
			pAnita(2u)->SetAnimNo(1);
			pAnita(0u)->SetAnimNo(4);
			pAnita(1u)->SetAnimNo(9);
			m_nStep = eSTEP_SELECT;
			m_nThroughCnt = 0u;
			Focus(eSELECT_NONE);
		}
		break;
	case eSTEP_SELECT:
	{
		if (m_nThroughCnt < eTHROUGH_WAIT)
		{
			m_nThroughCnt++;
			break;
		}
		SetCursorVisible();
		const eSelect prev = m_nSelectBuf;
		eSelect sel = eSELECT_NONE;
		bool decided = false;
		if ((GamePad::Get(0).trig & 0x860u) == 0)
		{
			// the stick (pad +0x10, dead zone 0.5) or the D-pad of each cursor's pad index
			for (uint i = 0; i < m_nCursorMax; i++)
			{
				float x = InGamePadManager::GetStick(InGamePadManager::GetPadNum((int)i), PadDefine::scLSTICK).x;   // UNVERIFIED: native stick for pad +0x10
				if (std::fabs(x) <= 0.5f) x = 0.0f;
				const uint trig = GamePad::Get((int)i).trig;
				if (x < 0.0f || (trig & GamePad::PAD_LEFT) != 0)
				{
					if (x <= 0.0f) sel = eSELECT_YES;
					break;
				}
				if (x > 0.0f || (trig & GamePad::PAD_RIGHT) != 0)
				{
					if (x >= 0.0f) sel = eSELECT_NO;
					break;
				}
			}
		}
		else
		{
			// A / B / Z on pad 0 decides the focused button (nothing focused: nothing happens)
			decided = true;
			if (m_nSelectBuf == eSELECT_NO)
			{
				snd::Play(SE_DECIDE);
				pAnita(0u)->SetAnimNo(4);
				pAnita(1u)->SetAnimNo(11);
				m_nStep = eSTEP_DECIDE;
				m_nSelect = eSELECT_NO;
			}
			else if (m_nSelectBuf == eSELECT_YES)
			{
				snd::Play(SE_DECIDE);
				pAnita(0u)->SetAnimNo(6);
				pAnita(1u)->SetAnimNo(9);
				m_nStep = eSTEP_DECIDE;
				m_nSelect = eSELECT_YES;
			}
		}
		// the pointer: when any cursor's pad points at the screen (HOME not held), pad i's pointer is tested against YES (pattern 0) and NO
		// (pattern 1) in layout space (x * 320, -y * 240, truncated)
		bool pointing = false;
		for (uint i = 0; i < m_nCursorMax; i++)
		{
			const GamePad::State& p = GamePad::Get(m_pCursor[i]->GetPadNo());
			if ((p.hold & GamePad::PAD_HOME) == 0 && p.pointerCount > 0)
			{
				pointing = true;
				break;
			}
		}
		if (pointing)
			for (uint i = 0; i < m_nCursorMax; i++)
			{
				const GamePad::State& p = GamePad::Get((int)i);
				const Vector4 pt((float)(int)(p.pointerX * (float)gfx::SCREEN_W * 0.5f), (float)(int)(-p.pointerY * (float)gfx::SCREEN_H * 0.5f), 0.0f, 1.0f);
				if (pAnita(0u)->IsHit(pt, 0))
				{
					sel = eSELECT_YES;
					break;
				}
				if (pAnita(1u)->IsHit(pt, 1))
				{
					sel = eSELECT_NO;
					break;
				}
			}
		if (sel != eSELECT_NONE && !decided)
		{
			Focus(sel);
			if (prev != m_nSelectBuf) snd::Play(SE_SELECT);
		}
		break;
	}
	case eSTEP_DECIDE:   // the cursors go; the decide animations ended -> out (board 2, YES 7, NO 12)
		SetCursorInvisible();
		if (IsAnim())
		{
			pAnita(2u)->SetAnimNo(2);
			pAnita(0u)->SetAnimNo(7);
			pAnita(1u)->SetAnimNo(12);
			m_nStep = eSTEP_OUT;
		}
		break;
	case eSTEP_OUT:   // ended -> hidden (board 0, YES 3, NO 8)
		if (IsAnim()) SetInvisible();
		break;
	default:
		break;
	}
	return cTaskAnita::Execute(pParam);   // 0x80014d58: the anitas
}

// DOL 0x80149b88 [audited]: only from hidden; in-animations (board 0, YES 3, NO 8) shown, no decision, YES focused
void cTaskConsRetry::Start()
{
	if (m_nStep != cTaskConsRetry::eStep::eSTEP_INVISIBLE) return;
	pAnita(2u)->SetAnimNo(0);
	pAnita(0u)->SetAnimNo(3);
	pAnita(1u)->SetAnimNo(8);
	pAnita(2u)->SetState(1u, true);
	pAnita(0u)->SetState(1u, true);
	pAnita(1u)->SetState(1u, true);
	m_nSelect = cTaskConsRetry::eSelect::eSELECT_NONE;
	m_nStep = cTaskConsRetry::eStep::eSTEP_IN;
	m_nSelectBuf = cTaskConsRetry::eSelect::eSELECT_YES;
}

}
