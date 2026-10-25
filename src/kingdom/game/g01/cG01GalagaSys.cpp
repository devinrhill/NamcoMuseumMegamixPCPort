#include "platform/devtools.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"

#include <cstdio>
#include <cstring>
#include <new>

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/KingdomJob.hpp"
#include "kingdom/game/g01/cDemoMan.hpp"
#include <cstdlib>
#include <typeinfo>
#include "GamePad.hpp"
#include "wii/Archive.hpp"

namespace kingdom::game::g01
{
namespace
{
// sdata2 strings (r2 - 0x6cb4 .. - 0x6ca0)
const char* const RESIDENT_ARC = "g01_galaga/g01resident.lzs";
const char* const MENU_ARC = "g01_galaga/g01menu.lzs";
const char* const WORLD_ARC = "g01_galaga/g01world%02d.lzs";
const char* const STAGE_ARC = "g01_galaga/g01stage%02d.lzs";
const char* const STAGE_XX_ARC = "g01_galaga/g01stagexx.lzs";
const char* const SCORE_NORMA_CSV = "ScoreNorma.csv";
constexpr int MULTI_STAGE = 12;   // AttachStageArc & co.: the multiplayer stage uses world 1 and g01stagexx

u32 Be32(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
void PutBe32(u8* p, u32 v) { p[0] = (u8)(v >> 24); p[1] = (u8)(v >> 16); p[2] = (u8)(v >> 8); p[3] = (u8)v; }
}

// ---- cDemoMan ----------------------------------------------------------------------------------------------------------------

// DOL 0x800b0650: frees the vector storage (the demo objects belong to their owners)
cDemoMan::~cDemoMan() = default;

// DOL 0x801be964 (the vector push_back)
void cDemoMan::Regist(cDemoInterface* p)
{
	m_Demo.push_back(p);
}

// DOL 0x801becd8
void cDemoMan::Start()
{
	for (cDemoInterface* d : m_Demo) d->DemoStart();
	m_fTimer = 0.0f;   // FLOAT_8047ced0
}

// DOL 0x801bedb8
bool cDemoMan::IsAllDone() const
{
	bool all = true;
	for (const cDemoInterface* d : m_Demo)
		if (!d->IsDemoDone()) all = false;
	if (std::getenv("MEGAMIX_G01DBG") && (int)m_fTimer % 120 == 0)
	{
		std::fprintf(stderr, "g01 demo t %.0f:", m_fTimer);
		for (const cDemoInterface* d : m_Demo) std::fprintf(stderr, " %s=%d", typeid(*d).name(), (int)d->IsDemoDone());
		std::fprintf(stderr, "\n");
	}
	return all;
}

// DOL 0x801bec4c
bool cDemoMan::IsSkip() const
{
	if (m_fTimer < 60.0f) return false;
	for (int i = 0; i < 4; i++)
		if (GamePad::Get(i).trig & 0x860u) return true;
	return false;
}

// DOL 0x801bed48
void cDemoMan::StartGoal()
{
	for (cDemoInterface* d : m_Demo) d->DemoGoalStart();
	m_fTimer = 0.0f;   // FLOAT_8047ced0
}

// DOL 0x801bedf8
bool cDemoMan::IsAllGoalDone() const
{
	for (const cDemoInterface* d : m_Demo)
		if (!d->IsGoalDone()) return false;
	return true;
}

// DOL 0x801bee48
void cDemoMan::Finish()
{
	for (cDemoInterface* d : m_Demo) d->DemoFinish();
}

// ---- records -------------------------------------------------------------------------------------------------------------------

// DOL 0x800b094c
void cRecordStageAccess::Clear()
{
	std::memset(p, 0, sizeof *p);
	p->n00 = 0; p->n04 = 0; p->n08 = 0; p->n10 = 0; p->n0c = 0; p->n14 = 0; p->n18 = 0; p->n1c = 0; p->n20 = 0; p->bOpen = 0;
}

// DOL 0x800b09d8
void cRecordMultiAccess::Clear()
{
	std::memset(p, 0, sizeof *p);
	for (int i : {0, 1, 10, 11, 12, 13, 2, 3, 4, 5, 6, 7, 8, 9}) p->n[i] = 0;
}

// DOL 0x800b0a84 (only installs the vtable; the data is filled by the save copy in cG01GalagaSys::Init)
cRecordMan::cRecordMan() {}

// DOL 0x800b0ad4
void cRecordMan::Reset()
{
	std::memset(m_Stage, 0, sizeof m_Stage);     // 0x1e4 bytes
	std::memset(m_Multi, 0, sizeof m_Multi);     // 0x268 bytes
	m_nVersion = 0;
	m_bAllOpen = 0;
	for (int i = 1; i < 11; i++)
	{
		cRecordStageAccess{&m_Stage[i]}.Clear();
		cRecordMultiAccess{&m_Multi[i]}.Clear();
	}
	m_Stage[1].bOpen = 1;                        // +0x50: stage 1 is open
	m_nVersion = VERSION;
}

// DOL 0x800b0ba4: walks stages 1..9 while they are open; the result is the first closed one (10 when 1..9 are open)
int cRecordMan::nOpenStage() const
{
	if (devtools::PreviewAll()) return 10; // devtools: --preview-all (not 1:1)
	int n = 0;
	for (int s = 1; s <= 9; s++)
	{
		if (m_Stage[s].bOpen != 1) return n;
		n = s;
	}
	return 10;
}

void cRecordMan::LoadFrom(const u8* be)
{
	s32* w = reinterpret_cast<s32*>(this);
	for (uint i = 0; i < 0x450 / 4; i++) w[i] = (s32)Be32(be + i * 4);
	m_bAllOpen = be[0x450];
}

void cRecordMan::SaveTo(u8* be) const
{
	const s32* w = reinterpret_cast<const s32*>(this);
	for (uint i = 0; i < 0x450 / 4; i++) PutBe32(be + i * 4, (u32)w[i]);
	be[0x450] = m_bAllOpen;
	be[0x451] = be[0x452] = be[0x453] = 0;
}
static_assert(sizeof(cRecordMan) == cRecordMan::DATA_SIZE, "cRecordMan must be the save layout");

// ---- cG01GalagaSys ---------------------------------------------------------------------------------------------------------------

// DOL 0x800afc3c
cG01GalagaSys::cG01GalagaSys()
{
	Init();
}

// DOL 0x800afc78
cG01GalagaSys::~cG01GalagaSys()
{
	Term();
}

// DOL 0x800afd5c (jump table 0x803963b0): 1..3 within the world (stage 10 = 4, the boss)
int cG01GalagaSys::nStageInWorld(int stage) const
{
	if (stage == CURRENT) stage = m_nStage;
	static const int TABLE[11] = {1, 1, 2, 3, 1, 2, 3, 1, 2, 3, 4};
	return (uint)stage <= 10 ? TABLE[stage] : 0;
}

// DOL 0x800afdb4
int cG01GalagaSys::nWorld(int stage) const
{
	if (stage == CURRENT) stage = m_nStage;
	const uint s = (uint)stage;
	if (s <= 3) return 1;
	if (s - 7 <= 3) return 3;
	if (s - 4 <= 2) return 2;
	return 0;
}

// DOL 0x800afe5c
sStageRecord* cG01GalagaSys::pStageRecord(int stage)
{
	if (stage == CURRENT) stage = m_nStage;
	return m_pRecord->pStageRecord(stage);
}

// DOL 0x800afe9c
sMultiRecord* cG01GalagaSys::pMultiRecord(int stage)
{
	if (stage == CURRENT) stage = m_nStage;
	return m_pRecord->pMultiRecord(stage);
}

// DOL 0x800afedc
sScoreNorma* cG01GalagaSys::pScoreNorma(int stage)
{
	if (stage == CURRENT) stage = m_nStage;
	return &m_pScoreNorma[stage];
}

// inlined in Init (0x800b0470) and ResetWork (0x800aff0c): clear both work areas, then the start values
void cG01GalagaSys::ClearWork()
{
	cRecordStageAccess{m_pStageWork}.Clear();
	m_pStageWork->n0c = 7;
	m_pStageWork->n08 = -1;
	cRecordMultiAccess{m_pMultiWork}.Clear();
	m_pMultiWork->n[1] = 7;                       // +0x04
	m_pMultiWork->n[6] = -1;                      // +0x18
	m_pMultiWork->n[7] = -1;                      // +0x1c
	m_pMultiWork->n[8] = -1;                      // +0x20
	m_pMultiWork->n[9] = -1;                      // +0x24
}

// DOL 0x800aff0c
void cG01GalagaSys::ResetWork()
{
	ClearWork();
}

// DOL 0x800b0030: the record into save block 0 (written to NAND by the next save)
void cG01GalagaSys::SaveData()
{
	if (devtools::PreviewAll()) return; // devtools: --preview-all never persists (not 1:1)
	u8 be[cRecordMan::DATA_SIZE];
	m_pRecord->SaveTo(be);
	KingdomGlobal::KingSys->SaveGameData(0, be, sizeof be);
}

// DOL 0x800b0074: opens stages 1..9 and sets the all-open flag
void cG01GalagaSys::AllOpen()
{
	for (int s = 1; s < 10; s++) pStageRecord(s)->bOpen = 1;
	m_pRecord->bAllOpen(true);
}

// DOL 0x800b00ec: ScoreNorma.csv (g01resident) rows STAGE1..STAGE9 -> pScoreNorma(1..9); stage 0 and 10 stay 0
void cG01GalagaSys::LoadScoreNorma()
{
	m_pScoreNorma = static_cast<sScoreNorma*>(::operator new(sizeof(sScoreNorma) * 11));   // FUN_8013b9f4(0x160)
	std::memset(m_pScoreNorma, 0, sizeof(sScoreNorma) * 11);
	kingdom::cKingdomCSV csv;
	csv.RegistData(SCORE_NORMA_CSV);              // cKingdomCSV::RegistData 0x8000bcd0
	for (int s = 1; s < 10; s++)
	{
		char key[0x80];
		std::snprintf(key, sizeof key, "STAGE%d", s);   // r13 - 0x7380
		const int row = csv.SearchRow(0, key, 0, 0);    // 0x80139928
		int score = 0, kill = 0, max1 = 0, rate = 0, life = 0, max2 = 0, max3 = 0, max4 = 0;
		csv.S32(row, 1, score);    // cJetCSV::S32 0x80139528
		csv.S32(row, 4, rate);
		csv.S32(row, 2, kill);
		csv.S32(row, 3, max1);
		csv.S32(row, 6, max2);
		csv.S32(row, 7, max3);
		csv.S32(row, 8, max4);
		csv.S32(row, 5, life);
		sScoreNorma& n = m_pScoreNorma[s];
		n.nScore = score; n.nHitRate = rate; n.nKill = kill; n.nMaxKill1 = max1; n.nMaxKill2 = max2; n.nMaxKill3 = max3; n.nMaxKill4 = max4;
		n.nLife = life;
	}
}

// DOL 0x800b032c
void cG01GalagaSys::Init()
{
	m_pDemoMan = nullptr;
	m_pTaskCollisionMan = nullptr;
	m_pTaskCamera = nullptr;
	m_bNewStageApear = false;
	m_bUnk9 = false;
	KingdomGlobal::KingSys->AttachArcData(RESIDENT_ARC, true);
	// FUN_80137648(KingSys + 0xa54, "GROUP_galaga_SE"): loads the sound group; the port plays cues straight from the archive
	m_nStage = 1;
	m_pMatching = new sGalagaMatching{};
	m_pMatching->nPlayer = 1;
	for (int i = 0; i < 4; i++)
	{
		sGalagaPlayer& p = m_pMatching->player[i];
		std::strcpy(p.szName, "test");               // 0x8036f714
		p.n44 = 6;
		p.nIndex = i;
		p.nWork = i;
		p.n50 = 0;
	}
	m_pRecord = new cRecordMan();
	m_pStageWork = new sStageWork{};
	m_pMultiWork = new sMultiWork{};
	ClearWork();
	u8 be[cRecordMan::DATA_SIZE]{};
	KingdomGlobal::KingSys->LoadGameData(0, be, sizeof be);
	m_pRecord->LoadFrom(be);
	if (m_pRecord->nVersion() == 0 || m_pRecord->nVersion() != cRecordMan::VERSION) m_pRecord->Reset();
	LoadScoreNorma();
}

// DOL 0x800b0558
void cG01GalagaSys::Term()
{
	// FUN_801376d8(KingSys + 0xa54): releases the sound group (see Init)
	delete m_pDemoMan;
	m_pDemoMan = nullptr;
	delete m_pMatching;
	m_pMatching = nullptr;
	delete m_pRecord;
	m_pRecord = nullptr;
	delete m_pStageWork;
	m_pStageWork = nullptr;
	delete m_pMultiWork;
	m_pMultiWork = nullptr;
	::operator delete(m_pScoreNorma);
	m_pScoreNorma = nullptr;
	KingdomGlobal::KingSys->DetachArcData(RESIDENT_ARC);
}

// DOL 0x800b06e4
void cG01GalagaSys::AttachStageArc(int world, int stage)
{
	char path[0x80];
	if (stage == MULTI_STAGE)
	{
		std::snprintf(path, 0x7f, WORLD_ARC, 1);
		KingdomGlobal::KingSys->AttachArcData(path, true);
		KingdomGlobal::KingSys->AttachArcData(STAGE_XX_ARC, true);
		return;
	}
	std::snprintf(path, 0x7f, WORLD_ARC, world);
	KingdomGlobal::KingSys->AttachArcData(path, true);
	std::snprintf(path, 0x7f, STAGE_ARC, stage);
	KingdomGlobal::KingSys->AttachArcData(path, true);
}

// DOL 0x800b07a0
void cG01GalagaSys::DetachStageArc(int world, int stage)
{
	char path[0x80];
	if (stage == MULTI_STAGE)
	{
		std::snprintf(path, 0x7f, WORLD_ARC, 1);
		KingdomGlobal::KingSys->DetachArcData(path);
		KingdomGlobal::KingSys->DetachArcData(STAGE_XX_ARC);
		return;
	}
	std::snprintf(path, 0x7f, WORLD_ARC, world);
	KingdomGlobal::KingSys->DetachArcData(path);
	std::snprintf(path, 0x7f, STAGE_ARC, stage);
	KingdomGlobal::KingSys->DetachArcData(path);
}

// DOL 0x800b084c: asynchronous load (the port loads at once, like every RegistArcDataAS)
void cG01GalagaSys::RegistStageArcAS(int world, int stage)
{
	char path[0x80];
	if (stage == MULTI_STAGE)
	{
		std::snprintf(path, 0x7f, WORLD_ARC, 1);
		KingdomGlobal::KingSys->RegistArcDataAS(path);
		KingdomGlobal::KingSys->RegistArcDataAS(STAGE_XX_ARC);
		return;
	}
	std::snprintf(path, 0x7f, WORLD_ARC, world);
	KingdomGlobal::KingSys->RegistArcDataAS(path);
	std::snprintf(path, 0x7f, STAGE_ARC, stage);
	KingdomGlobal::KingSys->RegistArcDataAS(path);
}

// DOL 0x800b08f8
void cG01GalagaSys::AttachMenuArc()
{
	KingdomGlobal::KingSys->AttachArcData(MENU_ARC, true);
}

// DOL 0x800b0924
void cG01GalagaSys::DetachMenuArc()
{
	KingdomGlobal::KingSys->DetachArcData(MENU_ARC);
}

// DOL 0x800b0cf4 (job 176, 0x0b000000): create the system, stage 1, then the stage select (job 178)
void cG01GalagaSys::JobInit()
{
	KingdomGlobal::GalagaSys = new cG01GalagaSys();
	KingdomGlobal::GalagaSys->nStage(1);
	Kingdom_SetJobNo(0x0b200000u, false, 0);
}
}
