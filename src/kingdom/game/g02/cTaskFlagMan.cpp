#include "kingdom/game/g02/cTaskFlagMan.hpp"

#include "kingdom/cTaskModel.hpp"

namespace kingdom::game::g02
{

cTaskFlagMan::sHooks cTaskFlagMan::Hooks;

int cTaskFlagMan::Type(kingdom::cTaskModel* g) { return g != nullptr && Hooks.Type != nullptr ? Hooks.Type(g) : -1; }
bool cTaskFlagMan::IsTaken(kingdom::cTaskModel* g) { return g != nullptr && Hooks.IsTaken != nullptr && Hooks.IsTaken(g); }

// the inline ctor in DOL 0x80076cb0 (cTaskKingdom ctor, vtable 0x80391e78, vector +0x1b4..+0x1bc = 0, cJetHash ctor) + Init 0x80070040
cTaskFlagMan::cTaskFlagMan()
{
	SetName("g02::cTaskFlagMan");
	Init();
}

// DOL 0x80070040 [audited] (only caller: cTaskMap Init 0x80076cb0, right after the ctor)
void cTaskFlagMan::Init()
{
	m_bNumbered = false;
	m_bStarted = false;
	m_nNumberMax = 0;
	m_nNumber = 0;
	m_Number.fill(nullptr);   // cJetHash Init (vtable +0x18) with 0x14 slots
	m_pCurrent = nullptr;
}

// DOL 0x800700a0 [audited]. Not gated by the pause (the DOL does not test it). Natively a missing number (the hash lookup returns nothing) stops the
// chain; the DOL would call through the null pointer (UNVERIFIED what cJetHash Find returns for a missing key).
bool cTaskFlagMan::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (m_bNumbered)
	{
		if (!m_bStarted) m_bStarted = true;
		if (IsTaken(m_pCurrent))              // the DOL reads +0x1e8 -> +0x1e4 unchecked
		{
			if (++m_nNumber <= m_nNumberMax)
			{
				m_pCurrent = m_nNumber < m_Number.size() ? m_Number[m_nNumber] : nullptr;   // cJetHash Find (vtable +0x3c)
				if (m_pCurrent != nullptr && Hooks.Show != nullptr) Hooks.Show(m_pCurrent);   // cTaskItemFlag vtable +0x58 (0x800b31e0)
			}
		}
	}
	return true;
}

// DOL 0x8007013c [audited]. The DOL dynamic_casts g to cTaskItemFlag (null for a non-flag) and reads the type of the cast result; natively "a flag" is
// a gimmick of type 11..33 (UNVERIFIED that the lucky / special flags are cTaskItemFlag subclasses; the factory 0x80391f48 suggests so).
// A second flag with the same number replaces the first in the hash (UNVERIFIED: cJetHash Insert with a used key).
bool cTaskFlagMan::Add(kingdom::cTaskModel* g)
{
	const int type = Type(g);
	if (g == nullptr || type < 11 || type > 33) return false;
	if (type >= 12 && type <= 31)
	{
		const u32 n = (u32)(type - 12);
		m_bNumbered = true;
		if (m_nNumberMax < n) m_nNumberMax = n;
		m_Number[n] = g;                      // cJetHash Insert (vtable +0x30)(hash, g, n, 0)
		if (type == 12)
		{
			m_pCurrent = g;
			m_nNumber = n;                        // = 0
		}
	}
	m_vFlag.push_back(g);
	return true;
}

// DOL 0x800704c0 [audited] (the DOL's cntlzw: returns n == 0 afterwards; n == 0 on entry returns true without a call)
bool cTaskFlagMan::Drop(u32 n, const float* pos, const float* from)
{
	if (n != 0)
		for (kingdom::cTaskModel* g : m_vFlag)
			if (Hooks.Drop != nullptr && Hooks.Drop(g, pos, from) && --n == 0) break;   // cTaskItemFlag vtable +0x5c (0x800b30ac)
	return n == 0;
}

// DOL 0x80070554 [audited]
bool cTaskFlagMan::IsDroppedAt(const float* pos) const
{
	for (kingdom::cTaskModel* g : m_vFlag)
		if (Hooks.IsDroppedAt != nullptr && Hooks.IsDroppedAt(g, pos)) return true;
	return false;
}

// DOL 0x800705f4 [audited] (kingdom::cTaskModel::pModel(flag, 0) 0x8009a430; nullptr past the end, like the DOL)
kingdom::cKingdomModel* cTaskFlagMan::pFlagModel(u32 n) const
{
	kingdom::cTaskModel* g = pFlag(n);
	return g != nullptr ? g->pModel(0) : nullptr;
}

}
