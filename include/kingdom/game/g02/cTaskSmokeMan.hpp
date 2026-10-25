#pragma once

#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"

namespace kingdom
{
class cTaskModel;
}

// Rally-X Remix (g02) smoke screen manager: kingdom::game::g02::cTaskSmokeMan (0x1c0 bytes, vtable 0x80393be0, parent cTaskKingdom), built
// inline by cTaskMap Init 0x80076cb0 (cTaskKingdom ctor, vector +0x1b4 = 0, Init 0x8007c0dc (empty)) and connected to the map as +0x1e4. The
// map then makes 0x20 smoke gimmicks (cTaskGimmickMan::Create 0x800736cc(gimmick man, type 0, 0, 0)) and adds each one (0x8007c0e8); when
// every smoke is out, the map's put 0x80079c1c adds more the same way (its "< 0" retry branches are dead code: Put returns only 0 / 1).
//
// The smoke gimmick (cTaskGimmickSmoke) is W1's: the manager reaches it through kingdom::cTaskModel* and the sHooks below.
//   gimmick +0x1cc / +0x1d0 = its cell (FUN_800b1cdc), +0x1dc = live byte (FUN_800b1d6c), +0x1e0 = the unit id that laid it (FUN_800b1d00).
namespace kingdom::game::g02
{
class cTaskSmokeMan : public kingdom::cTaskKingdom
{
public:
	struct sHooks
	{
		bool (*IsLive)(kingdom::cTaskModel* g){};                                     // gimmick +0x1dc byte (FUN_800b1d6c); fallback false
		u32 (*Owner)(kingdom::cTaskModel* g){};                                       // smoke +0x1e0 (FUN_800b1d00); fallback 0
		void (*Cell)(kingdom::cTaskModel* g, int out[2]){};                           // gimmick +0x1cc / +0x1d0 (FUN_800b1cdc)
		void (*Put)(kingdom::cTaskModel* g, u32 owner, const int cell[2], int arg){};   // FUN_800b2658 (cTaskGimmickSmoke: lay it)
		void (*Off)(kingdom::cTaskModel* g){};                                        // gimmick vtable +0x54 (0x800b1d54 / the smoke's override)
	};
	static sHooks Hooks;

	cTaskSmokeMan();                                   // the inline ctor in DOL 0x80076cb0 + Init 0x8007c0dc (empty)
	bool Execute(cJetTask* pParam) override;           // DOL 0x8007c0e0 (returns 1, nothing else)

	bool Add(kingdom::cTaskModel* g);                  // DOL 0x8007c0e8 (false for nullptr)
	// DOL 0x8007c47c (caller cTaskMap 0x80079c1c(map, owner, max, pos, arg) / 0x80079e3c with cell = ((int)(x +- 0.5) + 4, (int)(z +- 0.5) + 8)):
	// lay a smoke for `owner` unless one of its live smokes already sits on `cell` or it has `nMax` live ones; uses the first smoke that is
	// not live. Returns 1 when one was laid, else 0 (also when every smoke is live).
	int Put(u32 nOwner, u32 nMax, const int cell[2], int nArg);
	bool Reset();                                      // DOL 0x8007c400 (via cTaskMap 0x80079fbc): every live smoke off (vtable +0x54)
	bool IsFree(u32 nOwner) const;                     // DOL 0x8007c59c (via cTaskMap 0x80079fc4): no live smoke of nOwner
	u32 nSmoke() const { return (u32)m_vSmoke.size(); }

private:
	static bool IsLive(kingdom::cTaskModel* g);
	static u32 Owner(kingdom::cTaskModel* g);

	std::vector<kingdom::cTaskModel*> m_vSmoke;        // +0x1b4 (MSL vector: data, size +0x1b8, capacity +0x1bc)
};
}
