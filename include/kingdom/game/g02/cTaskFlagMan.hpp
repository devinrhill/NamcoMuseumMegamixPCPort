#pragma once

#include <array>
#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"

namespace kingdom
{
class cTaskModel;
class cKingdomModel;
}

// Rally-X Remix (g02) flag manager: kingdom::game::g02::cTaskFlagMan (0x1ec bytes, vtable 0x80391e78, parent cTaskKingdom), built inline by
// cTaskMap Init 0x80076cb0 (cTaskKingdom ctor, vector +0x1b4 = 0, cJetHash +0x1cc ctor (its vtable at +0x1e0, slot +0x0c), then Init
// 0x80070040) and connected to the map as +0x1e0. It keeps every flag gimmick (cTaskItemFlag, gimmick types 11..33) in placement order, and
// the numbered flags ITEM_FLAG_01..20 (types 12..31) in a 20-entry hash by number - 1: only the current numbered flag is out, the next one is
// shown (cTaskItemFlag vtable +0x58) once the current one is taken (+0x1e4).
//
// The flags are W1's cTaskGimmick / cTaskItemFlag (not complete here): the manager reaches them through kingdom::cTaskModel* (cTaskGimmick
// derives from cTaskModel) and the sHooks below, which the integrator fills with the gimmick classes' members (see the report / NOTES).
namespace kingdom::game::g02
{
class cTaskFlagMan : public kingdom::cTaskKingdom
{
public:
	struct sHooks
	{
		int (*Type)(kingdom::cTaskModel* g){};                                      // gimmick +0x1d4 (FUN_800b1ce4); fallback -1
		bool (*IsTaken)(kingdom::cTaskModel* g){};                                  // cTaskItemFlag +0x1e4 byte; fallback false
		void (*Show)(kingdom::cTaskModel* g){};                                     // cTaskItemFlag vtable +0x58 (0x800b31e0)
		bool (*Drop)(kingdom::cTaskModel* g, const float* pos, const float* from){};   // cTaskItemFlag vtable +0x5c (0x800b30ac)
		bool (*IsDroppedAt)(kingdom::cTaskModel* g, const float* pos){};            // +0x1e5 byte && +0x1f0 xyz == pos xyz (exact compare)
	};
	static sHooks Hooks;

	cTaskFlagMan();                                            // the inline ctor in DOL 0x80076cb0 + Init 0x80070040
	bool Execute(cJetTask* pParam) override;                   // DOL 0x800700a0

	void Init();                                               // DOL 0x80070040
	// DOL 0x8007013c (caller cTaskMap 0x80077e48 for every flag object it places): false when g is not a flag (the DOL's dynamic_cast to
	// cTaskItemFlag fails); numbered flags also go into the hash, ITEM_FLAG_01 becomes the current one
	bool Add(kingdom::cTaskModel* g);
	// DOL 0x800704c0 (caller cTaskMap 0x80079b5c(map, n, pos, dir) with n = 1, pos = the free cell 0x80078574 found, from = the car's
	// position): offers the drop to every flag in order until n accepted it; true when all n were dropped
	bool Drop(u32 n, const float* pos, const float* from);
	bool IsDroppedAt(const float* pos) const;                  // DOL 0x80070554 (caller cTaskMap 0x80078574)
	u32 nFlag() const { return (u32)m_vFlag.size(); }          // DOL 0x800705ec (via cTaskMap 0x80079ad0)
	kingdom::cKingdomModel* pFlagModel(u32 n) const;           // DOL 0x800705f4 (via cTaskMap 0x80079fcc): model 0 of the n-th flag
	kingdom::cTaskModel* pFlag(u32 n) const { return n < m_vFlag.size() ? m_vFlag[n] : nullptr; }

private:
	static int Type(kingdom::cTaskModel* g);
	static bool IsTaken(kingdom::cTaskModel* g);

	std::vector<kingdom::cTaskModel*> m_vFlag;                 // +0x1b4 (MSL vector: data, size +0x1b8, capacity +0x1bc)
	bool m_bNumbered{};                                        // +0x1c0: a numbered flag was added
	u32 m_nNumber{};                                           // +0x1c4: number - 1 of the current numbered flag
	bool m_bStarted{};                                         // +0x1c8 (set by Execute, never read)
	std::array<kingdom::cTaskModel*, 0x14> m_Number{};         // +0x1cc cJetHash (Init 0x14 slots, Insert vtable +0x30, Find +0x3c)
	u32 m_nNumberMax{};                                        // +0x1e4: the highest number - 1
	kingdom::cTaskModel* m_pCurrent{};                         // +0x1e8: the current numbered flag
};
}
