#include "kingdom/game/g03/cSaveLoadMan.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "KingdomGlobal.hpp"
#include <cstdio>

#include "WiiSystem.hpp"
#include "platform/devtools.hpp"

namespace kingdom::game::g03
{

namespace
{
// UNVERIFIED: where the DOL keeps the Pac-N-Roll progress inside the 0xe000 game-data block of museum.bin was not traced yet, so the
// progress lives in its own file next to the save instead (docs/OPTIONS.md, "Pac-N-Roll"). The Unity port kept it in GameStateManager.
std::string ProgressPath() { return WiiSystem::SaveDir() + "/pacnroll.bin"; }
constexpr uint PROGRESS_MAGIC = 0x504e5231; // "PNR1"
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

cSaveLoadMan::cSaveLoadMan()
{
	Init();
}

cSaveLoadMan::~cSaveLoadMan()
{
	Term();
}

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

void cSaveLoadMan::SaveData()
{
	if (devtools::PreviewAll()) return; // devtools: --preview-all never persists (not 1:1)
	m_SaveData.nValidate = 3u;
	std::FILE* f = std::fopen(ProgressPath().c_str(), "wb");
	if (f == nullptr) return;
	uint hdr[7] = {PROGRESS_MAGIC, m_SaveData.nValidate, m_SaveData.nLife, m_SaveData.nMaxHP, m_SaveData.bAllClear ? 1u : 0u, 26u, 0u};
	std::fwrite(hdr, sizeof(hdr), 1, f);
	for (const auto& r : m_SaveData.StageRecord)
	{
		uint w[5] = {r.nTime, r.nGhost, r.nCookie, r.nCookieMax,
			(r.bEatPizza ? 1u : 0u) | (r.bFree ? 2u : 0u)};
		std::fwrite(w, sizeof(w), 1, f);
	}
	std::fclose(f);
}

void cSaveLoadMan::LoadData()
{
	std::FILE* f = std::fopen(ProgressPath().c_str(), "rb");
	bool ok = false;
	if (f != nullptr)
	{
		uint hdr[7];
		if (std::fread(hdr, sizeof(hdr), 1, f) == 1 && hdr[0] == PROGRESS_MAGIC && hdr[1] == 3u && hdr[5] == 26u)
		{
			ok = true;
			m_SaveData.nValidate = hdr[1];
			m_SaveData.nLife = hdr[2];
			m_SaveData.nMaxHP = hdr[3];
			m_SaveData.bAllClear = hdr[4] != 0;
			for (auto& r : m_SaveData.StageRecord)
			{
				uint w[5];
				if (std::fread(w, sizeof(w), 1, f) != 1) { ok = false; break; }
				r.nTime = w[0]; r.nGhost = w[1]; r.nCookie = w[2]; r.nCookieMax = w[3];
				r.bEatPizza = (w[4] & 1u) != 0; r.bFree = (w[4] & 2u) != 0;
			}
		}
		std::fclose(f);
	}
	if (!ok) InitData();
}

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
