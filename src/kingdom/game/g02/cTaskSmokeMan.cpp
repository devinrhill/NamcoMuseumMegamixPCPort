#include "kingdom/game/g02/cTaskSmokeMan.hpp"

#include "kingdom/cTaskModel.hpp"

namespace kingdom::game::g02
{

cTaskSmokeMan::sHooks cTaskSmokeMan::Hooks;

bool cTaskSmokeMan::IsLive(kingdom::cTaskModel* g) { return g != nullptr && Hooks.IsLive != nullptr && Hooks.IsLive(g); }
u32 cTaskSmokeMan::Owner(kingdom::cTaskModel* g) { return g != nullptr && Hooks.Owner != nullptr ? Hooks.Owner(g) : 0u; }

// the inline ctor in DOL 0x80076cb0 (cTaskKingdom ctor, vtable 0x80393be0, vector +0x1b4..+0x1bc = 0) + Init 0x8007c0dc (empty)
cTaskSmokeMan::cTaskSmokeMan() { SetName("g02::cTaskSmokeMan"); }

// DOL 0x8007c0e0
bool cTaskSmokeMan::Execute(cJetTask* pParam)
{
	(void)pParam;
	return true;
}

// DOL 0x8007c0e8 (the DOL tests the argument, not the result of its dynamic_cast to the smoke class)
bool cTaskSmokeMan::Add(kingdom::cTaskModel* g)
{
	if (g == nullptr) return false;
	m_vSmoke.push_back(g);
	return true;
}

// DOL 0x8007c47c
int cTaskSmokeMan::Put(u32 nOwner, u32 nMax, const int cell[2], int nArg)
{
	u32 n = 0;
	for (kingdom::cTaskModel* g : m_vSmoke)
	{
		if (!IsLive(g) || Owner(g) != nOwner) continue;
		n++;
		int c[2] = {0, 0};
		if (Hooks.Cell != nullptr) Hooks.Cell(g, c);
		if (cell[0] == c[0] && cell[1] == c[1]) return 0;   // already a smoke of this unit on the cell
	}
	if (n >= nMax) return 0;
	for (kingdom::cTaskModel* g : m_vSmoke)
		if (!IsLive(g))
		{
			if (Hooks.Put != nullptr) Hooks.Put(g, nOwner, cell, nArg);   // FUN_800b2658 (its result is not tested)
			return 1;
		}
	return 0;
}

// DOL 0x8007c400
bool cTaskSmokeMan::Reset()
{
	for (kingdom::cTaskModel* g : m_vSmoke)
		if (IsLive(g) && Hooks.Off != nullptr) Hooks.Off(g);   // vtable +0x54
	return true;
}

// DOL 0x8007c59c
bool cTaskSmokeMan::IsFree(u32 nOwner) const
{
	for (kingdom::cTaskModel* g : m_vSmoke)
		if (IsLive(g) && Owner(g) == nOwner) return false;
	return true;
}

}
