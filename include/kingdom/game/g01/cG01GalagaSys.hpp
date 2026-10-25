#pragma once

#include "common.hpp"

// Galaga Remix (g01) game system: the object every Galaga job shares (KingdomGlobal::GalagaSys, DOL r13 - 0x5760), created by job 176
// (0x0b000000). Ported 1:1 from main.dol; class and method names from maps/game_g01_galaga.map, DOL addresses on every function.
// Stage numbers: 1..3 world 1, 4..6 world 2, 7..10 world 3 (10 = the boss stage, cAppG01GalagaBoss), 12 = the multiplayer stage.
namespace kingdom::game::g01
{
class cDemoMan;
class cTaskCamera;
class cTaskCollisionMan;

// One stage's record (0x2c bytes, the save layout; cRecordMan::pStageRecord). Field names from the game code that reads them
// once ported; offsets are the save file's.
struct sStageRecord
{
	s32 n00;        // +0x00
	s32 n04;        // +0x04
	s32 n08;        // +0x08
	s32 n0c;        // +0x0c
	s32 n10;        // +0x10
	s32 n14;        // +0x14
	s32 n18;        // +0x18
	s32 n1c;        // +0x1c
	s32 n20;        // +0x20
	s32 bOpen;      // +0x24  1 = the stage can be chosen (cRecordMan::nOpenStage, cG01GalagaSys::AllOpen)
	s32 n28;        // +0x28  (not cleared by cRecordStageAccess::Clear)
};
static_assert(sizeof(sStageRecord) == 0x2c);

// One stage's multiplayer record (0x38 bytes; cRecordMan::pMultiRecord)
struct sMultiRecord
{
	s32 n[14];      // +0x00..+0x34
};
static_assert(sizeof(sMultiRecord) == 0x38);

// The Galaga save block (block 0 of the save data, 0x454 bytes; DOL cRecordMan, 0x458 bytes with its vtable at +0x454).
// Kept in host byte order; Load / Save swap it to and from the big-endian save.
class cRecordMan
{
public:
	static constexpr uint DATA_SIZE = 0x454;
	static constexpr s32 VERSION = 2;

	cRecordMan();                                  // DOL 0x800b0a84
	void Reset();                                  // DOL 0x800b0ad4
	int nOpenStage() const;                        // DOL 0x800b0ba4: 1 + the number of opened stages in a row from stage 1 (max 10)
	bool bAllOpen() const { return m_bAllOpen != 0; }   // DOL 0x800b0c4c
	void bAllOpen(bool b) { m_bAllOpen = b; }      // DOL 0x800b0c54
	sStageRecord* pStageRecord(int stage) { return &m_Stage[stage]; }   // DOL 0x800b0c60
	sMultiRecord* pMultiRecord(int stage) { return &m_Multi[stage]; }   // DOL 0x800b0c6c
	s32 nVersion() const { return m_nVersion; }    // DOL 0x800b0c7c

	// pData (DOL 0x800b0c5c) handed the object itself to the save copies; the port converts the byte order on the way.
	void LoadFrom(const u8* be);
	void SaveTo(u8* be) const;

private:
	sStageRecord m_Stage[11]{};                    // +0x000, stages 1..10 (0 unused)
	sMultiRecord m_Multi[11]{};                    // +0x1e4
	s32 m_nVersion{};                              // +0x44c
	u8 m_bAllOpen{};                               // +0x450
	u8 m_pad[3]{};
};

// cG01GalagaSys::pStageWork (0x2c bytes, DOL +0x1c) and pMultiWork (0x38 bytes, +0x20): the running game's numbers, laid out like the
// records (cRecordStageAccess / cRecordMultiAccess::Clear clear them).
using sStageWork = sStageRecord;
using sMultiWork = sMultiRecord;

// cRecordStageAccess / cRecordMultiAccess (DOL vtables 0x803963e8 / 0x803963dc): a pointer to a record or a work area plus a vtable
// that only has a destructor; Clear zeroes the fields the game uses.
struct cRecordStageAccess
{
	sStageRecord* p;
	void Clear();                                  // DOL 0x800b094c
};
struct cRecordMultiAccess
{
	sMultiRecord* p;
	void Clear();                                  // DOL 0x800b09d8
};

// ScoreNorma.csv row STAGE<n> (0x20 bytes, cG01GalagaSys::pScoreNorma). Column names from the CSV's Shift-JIS header row.
struct sScoreNorma
{
	s32 nMaxKill1;  // +0x00 col 3 最大撃破数1
	s32 nMaxKill2;  // +0x04 col 6 最大撃破数2
	s32 nMaxKill3;  // +0x08 col 7 最大撃破数3
	s32 nMaxKill4;  // +0x0c col 8 最大撃破数4
	s32 nKill;      // +0x10 col 2 撃破数
	s32 nHitRate;   // +0x14 col 4 命中率 0..1000
	s32 nLife;      // +0x18 col 5 ライフ 0..7
	s32 nScore;     // +0x1c col 1 スコア
};
static_assert(sizeof(sScoreNorma) == 0x20);

// Matching data (0x154 bytes, DOL +0x4): number of players and per player 0x54 bytes
struct sGalagaPlayer
{
	char szName[0x44];  // +0x00 ("test" until the matching menu fills it)
	s32 n44;            // +0x44 = 6
	s32 nIndex;         // +0x48 = player index
	s32 nWork;          // +0x4c = player index (cG01GalagaSys::nWork)
	s32 n50;            // +0x50
};
struct sGalagaMatching
{
	s32 nPlayer;                // +0x00
	sGalagaPlayer player[4];    // +0x04
};
static_assert(sizeof(sGalagaPlayer) == 0x54 && sizeof(sGalagaMatching) == 0x154);

class cG01GalagaSys
{
public:
	static constexpr int CURRENT = -1;  // "the current stage" argument of the stage getters (0xffffffff in the DOL)

	cG01GalagaSys();                   // DOL 0x800afc3c
	~cG01GalagaSys();                  // DOL 0x800afc78

	cTaskCollisionMan*& pTaskCollisionMan() { return m_pTaskCollisionMan; }  // DOL 0x800afcd8
	cTaskCamera*& pTaskCamera() { return m_pTaskCamera; }                    // DOL 0x800afce0
	cDemoMan*& pDemoMan() { return m_pDemoMan; }                             // DOL 0x800afce8
	int nStage() const { return m_nStage; }                                  // DOL 0x800afcf0
	static int nStageIdx(int world, int stage) { return world * 3 + stage + 1; }   // DOL 0x800afcf8
	void nStage(int stage) { m_nStage = stage; }                             // DOL 0x800afd0c
	int nPlayer() const { return m_pMatching->nPlayer; }                     // DOL 0x800afd14
	void nPlayer(int n) { m_pMatching->nPlayer = n; }                        // DOL 0x800afd20
	int nWork(int player) const { return m_pMatching->player[player].nWork; }   // DOL 0x800afd2c
	// DOL 0x800afd40 (named "IsSingle" in maps/game_g01_galaga.map, but the code is `n != 1 ? n << clz(n ^ 1) >> 31 : 0` = 1 for 2..4 players)
	bool IsMulti() const { const u32 n = (u32)m_pMatching->nPlayer; return n != 1 && n != 0; }
	int nStageInWorld(int stage = CURRENT) const;                            // DOL 0x800afd5c
	int nWorld(int stage = CURRENT) const;                                   // DOL 0x800afdb4
	sGalagaMatching* pMatchingData() { return m_pMatching; }                 // DOL 0x800afe08
	char* pName(int player) { return m_pMatching->player[player].szName; }   // DOL 0x800afe10
	int nOpenStage() const { return m_pRecord->nOpenStage(); }               // DOL 0x800afe24
	// +0x09: set by Pac-Man's miss (0x800f5590) once the course stopped; bullets / enemies / missiles / boss missiles / effects then do nothing
	bool bUnk9() const { return m_bUnk9; }                                   // DOL 0x800afe2c
	void bUnk9(bool b) { m_bUnk9 = b; }                                      // DOL 0x800afe34
	// DAT_80479118: the pause flag, written at the end of the apps' Execute (0x801bc0fc / 0x801ba59c): set while the pause menu is open
	// (cTaskPause +0x1d4) or pad 0 holds HOME. 29 Galaga Executes / Draws do nothing while it is set.
	static bool IsPause() { return s_bPause; }
	static inline bool s_bPause = false;
	bool bAllOpen() const { return m_pRecord->bAllOpen(); }                  // DOL 0x800afe3c
	void bAllOpen(bool b) { m_pRecord->bAllOpen(b); }                        // DOL 0x800afe44
	bool bNewStageApear() const { return m_bNewStageApear; }                 // DOL 0x800afe4c
	void bNewStageApear(bool b) { m_bNewStageApear = b; }                    // DOL 0x800afe54
	sStageRecord* pStageRecord(int stage = CURRENT);                         // DOL 0x800afe5c
	sStageRecord* pStageRecord(int world, int stage) { return pStageRecord(nStageIdx(world, stage)); }   // DOL 0x800afe74
	sMultiRecord* pMultiRecord(int stage = CURRENT);                         // DOL 0x800afe9c
	sMultiRecord* pMultiRecord(int world, int stage) { return pMultiRecord(nStageIdx(world, stage)); }   // DOL 0x800afeb4
	sScoreNorma* pScoreNorma(int stage = CURRENT);                           // DOL 0x800afedc
	sStageWork* pStageWork() { return m_pStageWork; }                        // DOL 0x800afefc
	sMultiWork* pMultiWork() { return m_pMultiWork; }                        // DOL 0x800aff04
	void ResetWork();                                                        // DOL 0x800aff0c
	void SaveData();                                                         // DOL 0x800b0030
	void AllOpen();                                                          // DOL 0x800b0074
	void LoadScoreNorma();                                                   // DOL 0x800b00ec

	static void AttachStageArc(int world, int stage);                        // DOL 0x800b06e4
	static void DetachStageArc(int world, int stage);                        // DOL 0x800b07a0
	static void RegistStageArcAS(int world, int stage);                      // DOL 0x800b084c
	static void AttachMenuArc();                                             // DOL 0x800b08f8
	static void DetachMenuArc();                                             // DOL 0x800b0924

	static void JobInit();                                                   // DOL 0x800b0cf4 (job 176)

private:
	void Init();                                                             // DOL 0x800b032c
	void Term();                                                             // DOL 0x800b0558
	void ClearWork();                                                        // inlined twice: Init 0x800b0470, ResetWork

	int m_nStage{};                          // +0x00
	sGalagaMatching* m_pMatching{};          // +0x04
	bool m_bNewStageApear{};                 // +0x08
	bool m_bUnk9{};                          // +0x09
	cDemoMan* m_pDemoMan{};                  // +0x0c
	cTaskCamera* m_pTaskCamera{};            // +0x10
	cTaskCollisionMan* m_pTaskCollisionMan{};   // +0x14
	cRecordMan* m_pRecord{};                 // +0x18
	sStageWork* m_pStageWork{};              // +0x1c
	sMultiWork* m_pMultiWork{};              // +0x20
	sScoreNorma* m_pScoreNorma{};            // +0x24 (11 rows, 1..9 from the CSV)
};
}
