#include "kingdom/game/g03/cSaveLoadMan.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomSys.hpp"
#include <cstdio>
#include <cstring>

#include "WiiSystem.hpp"
#include "platform/devtools.hpp"

namespace kingdom::game::g03
{

namespace
{
// Game data block 2 of museum.bin holds the manager's whole data object as the DOL lays it out (0x3c0 bytes, big-endian; every field a word):
//   +0x00 nValidate (3), +0x04 nDefaultLife, +0x08 nDefaultHP, +0x0c nLife, +0x10 nMaxHP, +0x14 bAllClear,
//   +0x18 26 round records of 0x24: nTime (ms; 599999 = none, 0 for boss rounds), nGhost, nCookie, nCookieMax, bEatPizza, bTimeNew,
//         bGhostNew, bCookieNew, bFree
// (Init 0x8012aab0, GetStageRecord 0x8012ac80, SetStageRecord 0x8012acd4, SaveData 0x8012ad20, LoadData 0x8012ad64, ForceAllClear 0x8012adf0)
constexpr uint SAVE_BLOCK = 2u;
constexpr uint SAVE_SIZE = 0x3c0;
constexpr uint RECORD_SIZE = 0x24;

u32 GetBe(const u8* p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3]; }
void PutBe(u8* p, u32 v) { p[0] = (u8)(v >> 24); p[1] = (u8)(v >> 16); p[2] = (u8)(v >> 8); p[3] = (u8)v; }

void Pack(const SSaveDataPacnRoll& d, u8* b)
{
	std::memset(b, 0, SAVE_SIZE);
	const u32 head[6] = {d.nValidate, d.nDefaultLife, d.nDefaultHP, d.nLife, d.nMaxHP, d.bAllClear ? 1u : 0u};
	for (int i = 0; i < 6; i++) PutBe(b + 4 * i, head[i]);
	for (std::size_t n = 0; n < d.StageRecord.size() && n < 26; n++)
	{
		const SSaveDataPacnRoll::sStageRecord& r = d.StageRecord[n];
		const u32 w[9] = {r.nTime, r.nGhost, r.nCookie, r.nCookieMax, r.bEatPizza ? 1u : 0u, r.bTimeNew ? 1u : 0u, r.bGhostNew ? 1u : 0u,
		                  r.bCookieNew ? 1u : 0u, r.bFree ? 1u : 0u};
		for (int i = 0; i < 9; i++) PutBe(b + 0x18 + n * RECORD_SIZE + 4 * i, w[i]);
	}
}

void Unpack(const u8* b, SSaveDataPacnRoll& d)
{
	d.nValidate = GetBe(b);
	d.nDefaultLife = GetBe(b + 4);
	d.nDefaultHP = GetBe(b + 8);
	d.nLife = GetBe(b + 0xc);
	d.nMaxHP = GetBe(b + 0x10);
	d.bAllClear = GetBe(b + 0x14) != 0;
	d.StageRecord.resize(26);
	for (std::size_t n = 0; n < 26; n++)
	{
		const u8* r = b + 0x18 + n * RECORD_SIZE;
		SSaveDataPacnRoll::sStageRecord& s = d.StageRecord[n];
		s.nTime = GetBe(r);
		s.nGhost = GetBe(r + 4);
		s.nCookie = GetBe(r + 8);
		s.nCookieMax = GetBe(r + 0xc);
		s.bEatPizza = GetBe(r + 0x10) != 0;
		s.bTimeNew = GetBe(r + 0x14) != 0;
		s.bGhostNew = GetBe(r + 0x18) != 0;
		s.bCookieNew = GetBe(r + 0x1c) != 0;
		s.bFree = GetBe(r + 0x20) != 0;
	}
}

// native (not 1:1): earlier port builds kept the progress in their own file next to the save (pacnroll.bin, host order: 7 header words
// {"PNR1", 3, life, max HP, all clear, 26, 0}, then 26 x {time, ghosts, cookies, cookie max, bit 0 pizza | bit 1 free}). An empty block 2
// takes that file's progress once; the next save writes it into museum.bin and the file is no longer read.
bool ImportOldProgress(SSaveDataPacnRoll& d)
{
	std::FILE* f = std::fopen((WiiSystem::SaveDir() + "/pacnroll.bin").c_str(), "rb");
	if (f == nullptr) return false;
	uint hdr[7];
	bool ok = std::fread(hdr, sizeof(hdr), 1, f) == 1 && hdr[0] == 0x504e5231 && hdr[1] == 3u && hdr[5] == 26u;
	SSaveDataPacnRoll tmp = d;
	for (std::size_t n = 0; ok && n < 26; n++)
	{
		uint w[5];
		if (std::fread(w, sizeof(w), 1, f) != 1) { ok = false; break; }
		SSaveDataPacnRoll::sStageRecord& r = tmp.StageRecord[n];
		r.nTime = w[0]; r.nGhost = w[1]; r.nCookie = w[2]; r.nCookieMax = w[3];
		r.bEatPizza = (w[4] & 1u) != 0; r.bFree = (w[4] & 2u) != 0;
	}
	std::fclose(f);
	if (!ok) return false;
	tmp.nLife = hdr[2];
	tmp.nMaxHP = hdr[3];
	tmp.bAllClear = hdr[4] != 0;
	d = tmp;
	return true;
}
}

bool cSaveLoadMan::IsAllClear()
{
	return m_SaveData.bAllClear;
}

void cSaveLoadMan::ValidateAllClear()
{
	m_SaveData.bAllClear = true;
}

uint cSaveLoadMan::nDefaultLife()
{
	return m_SaveData.nDefaultLife;
}

uint cSaveLoadMan::nDefaultHP()
{
	return m_SaveData.nDefaultHP;
}

uint& cSaveLoadMan::nLife()
{
	return m_SaveData.nLife;
}

uint& cSaveLoadMan::nMaxHP()
{
	return m_SaveData.nMaxHP;
}

// DOL 0x8012aa04
cSaveLoadMan::cSaveLoadMan()
{
	Init();
}

cSaveLoadMan::~cSaveLoadMan()
{
	Term();
}

// DOL 0x8012aab0 (vtable slot 0): clear, defaults from the PAC_COMMON csv, cookie counts per round, round 0 open
void cSaveLoadMan::Init()
{
	m_SaveData = SSaveDataPacnRoll();
	m_SaveData.StageRecord.resize(26);
	for (int i = 0; i < (int)std::size(m_SaveData.StageRecord); i++)
	{
		m_SaveData.StageRecord[i] = SSaveDataPacnRoll::sStageRecord();
	}
	cKingdomCSV* cKingdomCSV2 = new cKingdomCSV();
	int _out = 0;
	cKingdomCSV2->RegistData("BIN_CSV_PACMAN_COMMON_PARAM");
	int num = cKingdomCSV2->SearchCol(0, "PAC_COMMON");
	CustomKingdomUtil::Assert(0 <= num, "error");
	int num2 = cKingdomCSV2->SearchRow(1, "DEFAULT_LIFE");
	CustomKingdomUtil::Assert(0 <= num2, "error");
	CustomKingdomUtil::Assert(cKingdomCSV2->S32(num2, num, _out), "error");
	m_SaveData.nDefaultLife = (uint)_out;
	m_SaveData.nLife = m_SaveData.nDefaultLife;
	num2 = cKingdomCSV2->SearchRow(1, "DEFAULT_HP");
	CustomKingdomUtil::Assert(0 <= num2, "error");
	CustomKingdomUtil::Assert(cKingdomCSV2->S32(num2, num, _out), "error");
	m_SaveData.nDefaultHP = (uint)_out;
	m_SaveData.nMaxHP = m_SaveData.nDefaultHP;
	m_SaveData.bAllClear = false;
	cKingdomCSV2->RegistData("BIN_COOKIE_NUM_CSV");
	for (int j = 0; j < 26; j++)
	{
		if (cG03PacnRollSys::IsBossStage((uint)j))
		{
			m_SaveData.StageRecord[j].nTime = 0u;
		}
		else
		{
			m_SaveData.StageRecord[j].nTime = 599999u;
		}
		cKingdomCSV2->S32(1 + j, 1, _out);
		m_SaveData.StageRecord[j].nCookieMax = (uint)_out;
	}
	m_SaveData.StageRecord[0].bFree = true;
}

void cSaveLoadMan::Term()
{
}

void cSaveLoadMan::InitData()
{
	Init();
	m_SaveData.nValidate = 3u;
}

// DOL 0x8012ac80
bool cSaveLoadMan::GetStageRecord(SSaveDataPacnRoll::sStageRecord& _out, uint round)
{
	CustomKingdomUtil::Assert(26 > round, "error");
	if (26 <= round)
	{
		return false;
	}
	_out = m_SaveData.StageRecord[round].Clone();
	if (devtools::PreviewAll()) _out.bFree = true; // devtools: --preview-all (not 1:1)
	return true;
}

// DOL 0x8012acd4
bool cSaveLoadMan::SetStageRecord(SSaveDataPacnRoll::sStageRecord set, uint round)
{
	CustomKingdomUtil::Assert(26 > round, "error");
	if (26 <= round)
	{
		return false;
	}
	m_SaveData.StageRecord[round] = set.Clone();
	return true;
}

// DOL 0x8012ad20: nValidate = 3, the data object into game data block 2 (museum.bin is written by the next SAVE job, cAppSaveLoad)
void cSaveLoadMan::SaveData()
{
	if (devtools::PreviewAll()) return; // devtools: --preview-all never persists (not 1:1)
	m_SaveData.nValidate = 3u;
	u8 b[SAVE_SIZE];
	Pack(m_SaveData, b);
	KingdomGlobal::KingSys->SaveGameData(SAVE_BLOCK, b, SAVE_SIZE);
}

// DOL 0x8012ad64: block 2 into the data object; an empty block (nValidate 0) or another version starts over (Init) as version 3
void cSaveLoadMan::LoadData()
{
	u8 b[SAVE_SIZE] = {};
	KingdomGlobal::KingSys->LoadGameData(SAVE_BLOCK, b, SAVE_SIZE);
	Unpack(b, m_SaveData);
	if (m_SaveData.nValidate == 0u)
	{
		Init();
		ImportOldProgress(m_SaveData);   // native: the progress of the port's old pacnroll.bin (not 1:1)
		m_SaveData.nValidate = 3u;
	}
	else if (m_SaveData.nValidate != 3u)
	{
		Init();
		m_SaveData.nValidate = 3u;
	}
}

// DOL 0x8012adf0: rounds 0..24 open, all clear (also the sys flag, 0x8012a994), then SaveData
void cSaveLoadMan::ForceAllClear()
{
	SSaveDataPacnRoll::sStageRecord _out = SSaveDataPacnRoll::sStageRecord();
	for (uint num = 0u; num < 25; num++)
	{
		GetStageRecord(_out, num);
		_out.bFree = true;
		SetStageRecord(_out, num);
	}
	m_SaveData.bAllClear = true;
	KingdomGlobal::PacnRollSys->bAllClear(true);
	SaveData();
}

}
