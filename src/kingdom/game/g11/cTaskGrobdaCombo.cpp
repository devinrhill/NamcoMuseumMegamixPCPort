#include "kingdom/game/g11/cTaskGrobdaCombo.hpp"

#include <algorithm>
#include <cstring>

#include "WiiSnd.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g11/cTaskGrobdaEffect.hpp"

namespace kingdom::game::g11
{

// DOL 0x8002fbb0 + 0x8002fd7c
cTaskGrobdaBomb::cTaskGrobdaBomb(cTaskGrobdaCombo* combo, int kind)
	: m_nKind(kind)
{
	SetName("cTaskGrobdaBomb");
	m_vPos[0] = m_vPos[1] = m_vPos[2] = m_vPos[3] = 0.0f;
	TimerReset(0);
	uint slot;
	if (kind == 0)
	{
		slot = 0x18;
		m_nGrow = 0x708;
		m_nHold = 0x708;
		m_fMin = 0.5f;    // FLOAT_80479fe4
		m_fMax = 2.5f;    // FLOAT_80479fe8
	}
	else
	{
		m_nGrow = 1000;
		slot = 0x19;
		m_nHold = 0x708;
		m_fMin = 1.5f;    // FLOAT_80479fec
		m_fMax = 4.5f;    // FLOAT_80479ff0
	}
	m_fRadius = m_fMin;   // param_1[9] = param_1[0x2d1]
	m_pHnd = new kingdom::cKingdomPtkHnd();
	m_pHnd->Generate(slot, false, true, true, 0.0f, Vector4(0.0f, -100.0f, 0.0f, 1.0f));
	if (m_pHnd->IsValid())
	{
		m_pHnd->Inactive();
		m_pHnd->Invisible();
	}
	SetFlags(0);
	m_pCombo = combo;   // param_1[0x2d3] = param_2 (the tag)
}

cTaskGrobdaBomb::~cTaskGrobdaBomb()
{
	delete m_pHnd;
}

// DOL 0x80030194: only a fresh bomb (flag word 0) starts: phase 1, the owner's type, the effect shown at the place
void cTaskGrobdaBomb::Start(uint type, const float pos[3])
{
	if (nFlags() != 0) return;
	m_nPhase = 1;
	m_bEnter = true;
	SetType(type);
	for (int i = 0; i < 3; i++) m_vPos[i] = pos[i];
	if (m_pHnd != nullptr && m_pHnd->IsValid())
	{
		m_pHnd->Active();    // FUN_8008a590
		m_pHnd->Visible();   // FUN_8008a5b0
		m_pHnd->vSetTrans(Vector4(pos[0], pos[1], pos[2], 1.0f));
	}
}

// DOL 0x8002ff10
bool cTaskGrobdaBomb::Execute(cJetTask* pParam)
{
	(void)pParam;
	UpdateTimers(!cTaskGrobdaCollision::Running());
	if (!cTaskGrobdaCollision::Running()) return true;
	auto reg = [&]() { if (cTaskGrobdaCollision::Instance() != nullptr) cTaskGrobdaCollision::Instance()->Register(3, this, m_pCombo); };
	if (m_nPhase == 1)
	{
		if (m_bEnter) { m_bEnter = false; TimerReset(0); TimerStart(0); }
		const uint t = TimerMs(0);
		if (t < m_nGrow)
		{
			float r = m_fMax * ((float)t / (float)m_nGrow);
			if (r < m_fMin) r = m_fMin;
			m_fRadius = r;
		}
		else
		{
			m_fRadius = m_fMax;
			SetFlags(1);
			m_nPhase = 2;
			m_bEnter = true;
		}
		reg();
	}
	else if (m_nPhase == 2)
	{
		if (m_bEnter) m_bEnter = false;
		const uint t = TimerMs(0);
		if (t < m_nHold)
			reg();
		else
		{
			SetFlags(2);
			m_nPhase = 3;
			m_bEnter = true;
		}
	}
	else if (m_nPhase == 3)
	{
		if (m_bEnter) { m_bEnter = false; TimerReset(0); TimerStart(0); }
		const uint t = TimerMs(0);
		if (m_pHnd == nullptr || !m_pHnd->IsValid() || t >= 5000)
		{
			if (m_pHnd != nullptr && m_pHnd->IsValid())
			{
				m_pHnd->Inactive();    // FUN_8008a59c
				m_pHnd->Invisible();   // FUN_8008a5bc
			}
			SetFlags(3);
			m_nPhase = 4;
			m_bEnter = true;
		}
	}
	return true;
}

// DOL 0x80034224
cTaskGrobdaCombo::cTaskGrobdaCombo(uint type)
	: m_nType(type)
{
	SetName("cTaskGrobdaCombo");
}

// DOL 0x8003458c: the blast (kind 2: the owner's type + 5, the fortress blast), the count, the list
void cTaskGrobdaCombo::Hit(const float pos[3], int kind, bool count)
{
	cTaskGrobdaBomb* b = new cTaskGrobdaBomb(this, kind);
	ConnectChild(b);
	uint type = m_nType;
	if (kind == 2)
	{
		if (type == 0x4000001u) type = 0x4000006u;
		else if (type == 0x4000002u) type = 0x4000007u;
		else if (type == 0x4000003u) type = 0x4000008u;
		else if (type == 0x4000004u) type = 0x4000009u;
	}
	if (count) m_nCount++;
	b->Start(type, pos);
	m_vBomb.push_back(b);
}

// DOL 0x8003440c: while active the finished bombs (flag word 3) are released; with none left the count is back to 0 (an empty list ends the combo: mode 2)
bool cTaskGrobdaCombo::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (cTaskGrobdaCollision::Running() && m_nMode == 1)
	{
		if (m_vBomb.empty())
			m_nMode = 2;
		else
		{
			for (size_t i = m_vBomb.size(); i-- > 0;)
				if (m_vBomb[i]->IsDone())
				{
					m_vBomb[i]->Release();   // cJetNode::Release(+0x97c)
					m_vBomb.erase(m_vBomb.begin() + (long)i);
				}
			if (m_vBomb.empty()) m_nCount = 0;
		}
	}
	return true;
}

}
