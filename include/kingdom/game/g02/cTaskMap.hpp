#pragma once

#include <random>
#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g02/cTaskGimmick.hpp"
#include "kingdom/game/g02/cTaskItem.hpp"

// Rally-X Remix (g02) stage map. The stage archive holds MAPOO_MAP (the cell grid) and MAPOO_OBJ (objects); g02game.lzs holds MAPOO_PNL.
// All three are big-endian, magic "MAPOO" + "MAP " / "OBJ " / "PNL ", a date word at +8.
//   MAPOO_MAP: +0x0c layer count, +0x10 width, +0x14 height (40 x 40), +0x20 s16 cells per layer. Cell values: 1 outside, 2 road,
//              3 wall, 4 stone, 5 oil, 6 / 7 mud (big / small, shaped by the neighbours), 8 spike.
//   MAPOO_OBJ: +0x20 count, +0x24 offset of 0x40-byte records {type, x, z, ?, number, ...}; x / z in 1/16 cell with the cell centre at
//              x = 16 * cell + 8, z = 16 * cell + 16. Types: 0 player start, 1 enemy start, 2 fuel, 3 paint, 4 armor, 5 smoke tank,
//              6 flag candidate, 7 special flag, 8 lucky flag, 9 numbered flag (record word 4 = 1..20).
// A cell (x, z) is the world point (x - 4, 0, z - 8) (cTaskGimmick::Execute FUN_800b1b38).
// The gimmick classes and their manager are in cTaskGimmick.hpp / cTaskItem.hpp.
namespace kingdom::game::g02
{
class cTaskUnit;
class cTaskConsole;
class cTaskFlagMan;    // kingdom::game::g02::cTaskFlagMan (0x1ec, vtable 0x80391e78): agent W2
class cTaskSmokeMan;   // kingdom::game::g02::cTaskSmokeMan (0x1c0, vtable 0x80393be0): agent W2

// kingdom::game::g02::cTaskMap (0x22c bytes, vtable 0x80393918, a cTaskPushBack; ctor FUN_8012490c, Init 0x80076cb0, Execute 0x80079ae0,
// dtor FUN_8007a084)
class cTaskMap : public kingdom::cTaskKingdom
{
public:
	// The flag / smoke manager calls of the DOL (W2's classes). The integrator assigns them once W2's files are in (see the agent report);
	// while a hook is null the map runs the same DOL logic on its own lists of the gimmicks it gave the manager (marked "fallback").
	struct sHooks
	{
		cTaskFlagMan* (*NewFlagMan)(cTaskMap* map){};      // new 0x1ec + FUN_80070040; must ConnectChild it to the map (DOL order: after the gimmick man)
		cTaskSmokeMan* (*NewSmokeMan)(cTaskMap* map){};    // new 0x1c0 (cTaskSmokeMan ctor); must ConnectChild it to the map
		void (*FlagAdd)(cTaskFlagMan* fm, cTaskItemFlag* flag){};   // FUN_8007013c
		int (*FlagCount)(cTaskFlagMan* fm){};              // FUN_800705ec (+0x1b8)
		bool (*FlagDrop)(cTaskFlagMan* fm, int n, const float pTo[4], const float pFrom[4]){};   // FUN_800704c0 (flag +0x5c on n flags)
		bool (*FlagAt)(cTaskFlagMan* fm, const float pPos[4]){};    // FUN_80070554 (a flag flying to pPos)
		cTaskItemFlag* (*FlagGet)(cTaskFlagMan* fm, int index){};   // FUN_800705f4 (the DOL returns pModel(flag, 0); nullptr past the end)
		void (*SmokeAdd)(cTaskSmokeMan* sm, cTaskGimmickSmoke* smoke){};   // FUN_8007c0e8
		// FUN_8007c47c: 0 = this owner has a smoke on the cell or nMax of them alive, 1 = put (cTaskGimmickSmoke::Put), < 0 = no free smoke
		int (*SmokePut)(cTaskSmokeMan* sm, int nOwner, int nMax, const int pCell[2], int nIndex){};
		void (*SmokeReset)(cTaskSmokeMan* sm){};           // FUN_8007c400 (every active smoke +0x54)
		bool (*SmokeFree)(cTaskSmokeMan* sm, int nOwner){};   // FUN_8007c59c (no active smoke of nOwner)
	};
	static sHooks Hooks;

	cTaskMap();
	bool Execute(cJetTask* pParam) override;   // DOL 0x80079ae0 (return 1)

	int nWidth() const;
	int nHeight() const;
	int Cell(int x, int z, int layer = 0) const;   // the MAPOO_MAP value
	u32 Attr(int x, int z) const;      // +0x1e8: bit 0 road, 0x8000 mud (FUN_80077b54)
	bool PlayerStart(int n, int& x, int& z) const;  // the n-th type 0 object
	int nEnemy() const { return m_nEnemy; }          // the type 1 objects (+0x1f4; the app makes one enemy car each)
	int nFlagTotal() const;                           // DOL 0x80079ad0 (the flag manager's count)
	// DOL 0x800780f4 via 0x800780e4 / 0x800780ec: put a car on its start record (type 0 = player: the record whose word +0x10 is `index`,
	// 1 = enemy: the index-th type 1 record); bReset = 0x800780ec (the reset between lives skips the red car's initial flags)
	bool PlaceCar(cTaskUnit* pCar, int nType, int nIndex, bool bReset);
	// the console the DOL ctor gets (+0x1ec); registers every flag made so far and every later one (FUN_800662c8)
	void RegistFlags(cTaskConsole* pConsole);
	void ResetGimmicks();                             // DOL 0x80079fbc (smoke manager reset FUN_8007c400)

	// the car side (cRallyUnitHost::Map* in cTaskUnit.hpp)
	void Contact(cTaskUnit* pUnit, int x, int z);     // DOL 0x80079b50 (-> cTaskGimmickMan::Contact(unit, x, z, 1))
	bool DropFlag(int n, const float pPos[4], int nDir);   // DOL 0x80079b5c (the red car: n = 1)
	bool PutSmoke(int nOwner, int nMax, const float pPos[4], int nIndex);   // DOL 0x80079c1c (the car: nMax = 3, index = +0x1cc)
	bool SmokeFree(int nOwner);                       // DOL 0x80079fc4
	bool NextFlag(int nIndex, float out[4]) const;    // DOL 0x80079fcc (the red car passes its taken count +0x2d0); false past the end
	bool FindDropCell(float out[4], const float pPos[4], int nDir);   // DOL 0x80078574 (true: out moved to a free road cell)

	// the multiplayer item spawn points (record types 3 paint / 4 armor / 2 fuel / 5 smoke; DOL +0x200 / +0x20c / +0x218 / +0x224 with the counts
	// +0x1fc / +0x208 / +0x214 / +0x220: 0x8007a064.. / 0x80079fe4 ..)
	int nSpawnCount(int nRecType) const { return (unsigned)nRecType < 6u ? (int)m_Spawn[nRecType].size() : 0; }
	bool SpawnCell(int nRecType, int nIdx, int cell[2]) const;
	cTaskGimmickMan* pGimmickMan() const { return m_pGimmickMan; }   // +0x1dc
	cTaskFlagMan* pFlagMan() const { return m_pFlagMan; }            // +0x1e0
	cTaskSmokeMan* pSmokeMan() const { return m_pSmokeMan; }         // +0x1e4

private:
	void BuildCells();                 // DOL 0x80077190 (first half) / 0x80077b54
	void PlaceObjects();               // DOL 0x80077190 (flags) / 0x80077530 / 0x80077e48 / the multiplayer 0x8007765c + 0x80078020
	static void ObjCell(const u8* rec, int& x, int& z);
	bool IsRoad(int x, int z) const { return (Attr(x, z) & 1u) != 0; }   // the inlined attr test of 0x80078574 (0 off the map)
	void AddFlag(cTaskGimmick* g);     // FlagMan FUN_8007013c + console FUN_800662c8 (map +0x1ec)
	void AddSmoke();                   // cTaskGimmickMan::Create(0) + smoke manager FUN_8007c0e8

	const u8* m_pPnl{};                // +0x1c4
	const u8* m_pMap{};                // +0x1cc
	const u8* m_pObj{};                // +0x1d4
	cTaskGimmickMan* m_pGimmickMan{};  // +0x1dc
	cTaskFlagMan* m_pFlagMan{};        // +0x1e0
	cTaskSmokeMan* m_pSmokeMan{};      // +0x1e4
	std::vector<u32> m_Attr;           // +0x1e8
	cTaskConsole* m_pConsole{};        // +0x1ec
	int m_nPlayer{};                   // +0x1f0: type 0 objects
	int m_nEnemy{};                    // +0x1f4: type 1 objects
	std::mt19937 m_Rand{0x20061206u};  // +0x1f8 cKingdomRand (init 0x801412b4 + genrand 0x8014146c = standard MT19937)
	// native: what the map gave the managers (the fallbacks while the W2 hooks are null)
	std::vector<std::pair<int, int>> m_Spawn[6];   // the multiplayer spawn points by record type (2..5)
	std::vector<cTaskItemFlag*> m_vFlags;
	std::vector<cTaskGimmickSmoke*> m_vSmokes;
};
}
