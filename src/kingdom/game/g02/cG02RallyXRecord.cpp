#include "kingdom/game/g02/cG02RallyXRecord.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr u32 VERSION = 0x20070824u;
constexpr int BLOCK_SIZE = 0x250;

u32 GetBE(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
void PutBE(u8* p, u32 v)
{
	p[0] = (u8)(v >> 24);
	p[1] = (u8)(v >> 16);
	p[2] = (u8)(v >> 8);
	p[3] = (u8)v;
}
}

int cG02RallyXSysExt::nPlayer = 1;
int cG02RallyXSysExt::nCtrlType[4] = {2, 2, 2, 2};
int cG02RallyXSysExt::nPadNo[4] = {0, 1, 2, 3};
bool cG02RallyXSysExt::bNewStageApear = false;

// the sys ctor (DOL 0x800c54e0): +0xc = 1, per player {+0x14 = 2, +0x1c = i}, +0x1b8 = 0
void cG02RallyXSysExt::Reset()
{
	nPlayer = 1;
	for (int i = 0; i < 4; i++)
	{
		nCtrlType[i] = 2;
		nPadNo[i] = i;
	}
	bNewStageApear = false;
}

// DOL 0x800c5b24
void cG02RallyXSysExt::AttachMenuArc()
{
	KingdomGlobal::KingSys->AttachArcData("g02_rallyx/g02menu.lzs", true);   // PTR 0x8047b2e0
}

// DOL 0x800c5b50
void cG02RallyXSysExt::DetachMenuArc()
{
	KingdomGlobal::KingSys->DetachArcData("g02_rallyx/g02menu.lzs");
}

// DOL 0x800c5788
int cG02RallyXSysExt::WorldOf(int i)
{
	if (i >= 0x13) return i >= 0x1d ? 0 : (i >= 0x19 ? 0xb : 4);
	if (i >= 7) return i >= 0xd ? 3 : 2;
	return i >= 1 ? 1 : 0;
}

// DOL 0x800c5808
int cG02RallyXSysExt::StageInWorldOf(int idx)
{
	return idx == 0 ? 0 : (idx - 1) % 6 + 1;
}

// DOL 0x800c5bf8
bool cG02RallyXSysExt::IsBossStageOf(int idx)
{
	return StageInWorldOf(idx) == 6;
}

// DOL 0x800c5858
int cG02RallyXSysExt::StageIdxOf(int world, int stage)
{
	if (world == 0) return 0;
	if (world == 0xb) world = 5;
	return stage + (world - 1) * 6 + 1;
}

cG02RallyXRecord& cG02RallyXRecord::Get()
{
	static cG02RallyXRecord s_Record;   // DAT_804844a0
	if (!s_Record.m_bLoaded) s_Record.Load();
	return s_Record;
}

// DOL 0x800c54e0 (the save part of the sys ctor)
void cG02RallyXRecord::Load()
{
	m_bLoaded = true;
	u8 be[BLOCK_SIZE] = {};
	KingdomGlobal::KingSys->LoadGameData(1u, be, sizeof be);
	m_nValid = GetBE(be + 0);
	m_nVersion = GetBE(be + 4);
	m_nAllClear = GetBE(be + 8);
	for (int i = 0; i < STAGE_MAX; i++)
	{
		const u8* p = be + 0xc + 0x14 * i;
		m_Stage[i].nOpen = GetBE(p + 0x0);
		m_Stage[i].nTime = GetBE(p + 0x4);
		m_Stage[i].nScore = GetBE(p + 0x8);
		m_Stage[i].nNewScore = GetBE(p + 0xc);
		m_Stage[i].nNewTime = GetBE(p + 0x10);
	}
	if (m_nValid == 0u || m_nVersion != VERSION) Reset();
}

// DOL 0x800c5ff0
void cG02RallyXRecord::Save() const
{
	u8 be[BLOCK_SIZE] = {};
	PutBE(be + 0, m_nValid);
	PutBE(be + 4, m_nVersion);
	PutBE(be + 8, m_nAllClear);
	for (int i = 0; i < STAGE_MAX; i++)
	{
		u8* p = be + 0xc + 0x14 * i;
		PutBE(p + 0x0, m_Stage[i].nOpen);
		PutBE(p + 0x4, m_Stage[i].nTime);
		PutBE(p + 0x8, m_Stage[i].nScore);
		PutBE(p + 0xc, m_Stage[i].nNewScore);
		PutBE(p + 0x10, m_Stage[i].nNewTime);
	}
	KingdomGlobal::KingSys->SaveGameData(1u, be, sizeof be);
}

// DOL 0x800c6024. Stages 1..24 take CSV_RECORD (cParamDefaultRecord, DAT_80479018; row = (i - 1) + (i - 1) / 6 + 1: one header row per
// world): column 1 score, columns 2 / 3 / 4 = minutes / seconds / ms. Index 0 and the multiplayer stages 25..28: time (u32)3599999.0f
// (FLOAT_8047b2d0), score 10000. Then stage 1 and stage 25 open (DAT_804844c0 / DAT_804846a0).
void cG02RallyXRecord::Reset()
{
	kingdom::cKingdomCSV csv;
	const bool bCsv = csv.RegistData("CSV_RECORD");   // 0x410233cd (g02resident.lzs)
	m_nAllClear = 0u;
	for (int i = 0; i < STAGE_MAX; i++)
	{
		sStage& s = m_Stage[i];
		const u32 n = (u32)(i - 1);
		if (n < 0x18u)
		{
			s.nOpen = 0u;
			const int row = (int)n + (int)n / 6 + 1;
			if (bCsv && row < 0x25)
			{
				s.nTime = (u32)(csv.S32(row, 2, 0) * 60000 + csv.S32(row, 3, 0) * 1000 + csv.S32(row, 4, 0));
				s.nScore = (u32)csv.S32(row, 1, 0);
			}
			else
			{
				s.nTime = 0u;
				s.nScore = 0u;
			}
			s.nNewScore = 0u;
			s.nNewTime = 0u;
		}
		else
		{
			s.nOpen = 0u;
			s.nTime = 3599999u;
			s.nScore = 10000u;
			s.nNewScore = 0u;
			s.nNewTime = 0u;
		}
	}
	m_Stage[1].nOpen = 1u;
	m_Stage[25].nOpen = 1u;
	m_nValid = 1u;
	m_nVersion = VERSION;
}

// DOL 0x800c6320: stage indexes 1..23; the boss stages (stage in world 6) are skipped; the result is the last open stage before the first
// closed one (0 when stage 1 is closed)
int cG02RallyXRecord::GetFreeStage() const
{
	int nFree = 0;
	for (int idx = 1; idx <= 0x17; idx++)
	{
		const int nStage = (idx - 1) % 6 + 1;   // inlined cG02RallyXSys::nStageInWorld (0x800c5808) of idx
		if (nStage == 6) continue;
		if (m_Stage[idx].nOpen == 0u) break;
		nFree = idx;
	}
	return nFree;
}

// DOL 0x800c63f0
void cG02RallyXRecord::SetOpen(int idx)
{
	if (Ok(idx)) m_Stage[idx].nOpen = 1u;
}

// DOL 0x800c6410
void cG02RallyXRecord::SetAllClear()
{
	m_nAllClear = 1u;
}

// DOL 0x800c6218
bool cG02RallyXRecord::SetRecord(int idx, u32 score, u32 time)
{
	if (!Ok(idx)) return false;
	sStage& s = m_Stage[idx];
	const u32 oldTime = s.nTime;
	if (time <= oldTime)
	{
		s.nTime = time;
		s.nNewTime = 1u;
	}
	if (score < s.nScore) return time <= oldTime;
	s.nScore = score;
	s.nNewScore = 1u;
	return true;
}

// DOL 0x800c6274
bool cG02RallyXRecord::SetTime(int idx, u32 time)
{
	if (!Ok(idx) || m_Stage[idx].nTime < time) return false;
	m_Stage[idx].nTime = time;
	m_Stage[idx].nNewTime = 1u;
	return true;
}

}
