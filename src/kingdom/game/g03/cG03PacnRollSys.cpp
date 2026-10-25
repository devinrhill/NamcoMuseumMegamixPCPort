#include "kingdom/game/g03/cG03PacnRollSys.hpp"

#include <limits>

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cSaveLoadMan.hpp"
#include "KingdomGlobal.hpp"
#include <SSaveDataPacnRoll.hpp>

#include "wii/Archive.hpp"

namespace kingdom::game::g03
{

const string cG03PacnRollSys::arcKeys[28] = {"g03_pacnroll/arcsv/g03w01s01", "g03_pacnroll/arcsv/g03w01s02", "g03_pacnroll/arcsv/g03w01s03", "g03_pacnroll/arcsv/g03w01s04", "g03_pacnroll/arcsv/g03w01s05", "g03_pacnroll/arcsv/g03w02s01", "g03_pacnroll/arcsv/g03w02s02", "g03_pacnroll/arcsv/g03w02s03", "g03_pacnroll/arcsv/g03w02s04", "g03_pacnroll/arcsv/g03w03s01", "g03_pacnroll/arcsv/g03w03s02", "g03_pacnroll/arcsv/g03w03s03", "g03_pacnroll/arcsv/g03w03s04", "g03_pacnroll/arcsv/g03w03s05", "g03_pacnroll/arcsv/g03w04s01", "g03_pacnroll/arcsv/g03w04s02", "g03_pacnroll/arcsv/g03w04s03", "g03_pacnroll/arcsv/g03w04s04", "g03_pacnroll/arcsv/g03w04s05", "g03_pacnroll/arcsv/g03w04s06", "g03_pacnroll/arcsv/g03w05s01", "g03_pacnroll/arcsv/g03w05s02", "g03_pacnroll/arcsv/g03w05s03", "g03_pacnroll/arcsv/g03w05s04", "g03_pacnroll/arcsv/g03w05s05", "g03_pacnroll/arcsv/g03w05s06", "g03_pacnroll/arcsv/g03wXXs03", "g03_pacnroll/arcsv/g03wXXs04"};

cG03PacnRollSys::cG03PacnRollSys()
{
	Init();
}

cG03PacnRollSys::cG03PacnRollSys(bool game)
{
	Init();
	SetTopMenuMode(game);
}

cG03PacnRollSys::~cG03PacnRollSys()
{
	Free();
}

uint& cG03PacnRollSys::nRound()
{
	return m_nRound;
}

bool& cG03PacnRollSys::IsRetry()
{
	return m_bRetry;
}

cG03PacnRollSys::sRetryData*& cG03PacnRollSys::pRetryData()
{
	return m_RetryData;
}

cSaveLoadMan* cG03PacnRollSys::pSaveLoadMan()
{
	return m_pSaveLoadMan;
}

void cG03PacnRollSys::SetTopMenuMode(bool game)
{
	m_bTopMenuMode = !game;
}

void cG03PacnRollSys::ResetDeadEnemyCnt()
{
	DeadEnemyCnt = 0;
}

void cG03PacnRollSys::AddDeadEnemyCnt(int add)
{
	DeadEnemyCnt += add;
}

void cG03PacnRollSys::ResetGetItemCnt()
{
	GetItemCnt = 0;
}

void cG03PacnRollSys::AddGetItemCnt(int add)
{
	GetItemCnt += add;
}

void cG03PacnRollSys::Init()
{
	// DOL 0x80129dc8 (cG03PacnRollSys ctor): AttachArcData("g03_pacnroll/g03resident.lzs"), then the sound group GROUP_pacn_SE (the PC
	// mixer needs no group load), the save manager and the retry data
	wii::ArchiveStore::Attach("g03_pacnroll/g03resident.lzs");
	m_nRound = 0u;
	m_bRetry = false;
	InitRetryData();
	SetTopMenuMode(false);
	bAllClear(false);
	bOpenStage(false);
	m_pSaveLoadMan = new cSaveLoadMan();
	m_pSaveLoadMan->LoadData();
	m_bAllClear = m_pSaveLoadMan->IsAllClear();
	m_RetryData->vecItem.reserve(255);
	m_RetryData->vecPoint.reserve(16);
	m_RetryData->vecGate.reserve(16);
	m_RetryData->vecBox.reserve(128);
	m_RetryData->vecGhost.reserve(16);
}

void cG03PacnRollSys::Term()
{
}

void cG03PacnRollSys::Free()
{
	Term();
	wii::ArchiveStore::Detach("g03_pacnroll/g03resident.lzs");
}

string cG03PacnRollSys::pStageArcKey(uint round)
{
	CustomKingdomUtil::Assert(28 > round, "error");
	return arcKeys[round];
}

uint cG03PacnRollSys::nWorld(uint nWorldStage)
{
	if (nWorldStage == std::numeric_limits<uint>::max())
	{
		nWorldStage = m_nRound;
	}
	switch (nWorldStage)
	{
	case 0u:
	case 1u:
	case 2u:
	case 3u:
	case 4u:
		return 0u;
	case 5u:
	case 6u:
	case 7u:
	case 8u:
		return 1u;
	case 9u:
	case 10u:
	case 11u:
	case 12u:
	case 13u:
		return 2u;
	case 14u:
	case 15u:
	case 16u:
	case 17u:
	case 18u:
	case 19u:
		return 3u;
	case 20u:
	case 21u:
	case 22u:
	case 23u:
	case 24u:
	case 25u:
		return 4u;
	default:
		CustomKingdomUtil::Assert("error");
		return 0u;
	}
}

uint cG03PacnRollSys::nStage(uint nWorldStage)
{
	if (nWorldStage == std::numeric_limits<uint>::max())
	{
		nWorldStage = m_nRound;
	}
	switch (nWorldStage)
	{
	case 0u:
	case 5u:
	case 9u:
	case 14u:
	case 20u:
		return 0u;
	case 1u:
	case 6u:
	case 10u:
	case 15u:
	case 21u:
		return 1u;
	case 2u:
	case 7u:
	case 11u:
	case 16u:
	case 22u:
		return 2u;
	case 3u:
	case 8u:
	case 12u:
	case 17u:
	case 23u:
		return 3u;
	case 4u:
	case 13u:
	case 18u:
	case 24u:
		return 4u;
	case 19u:
	case 25u:
		return 5u;
	default:
		CustomKingdomUtil::Assert("error");
		return 0u;
	}
}

uint cG03PacnRollSys::nStageMax(uint nWorldNo)
{
	if (nWorldNo == std::numeric_limits<uint>::max())
	{
		nWorldNo = nWorld();
	}
	switch (nWorldNo)
	{
	case 0u:
		return 5u;
	case 1u:
		return 4u;
	case 2u:
		return 5u;
	case 3u:
		return 6u;
	case 4u:
		return 6u;
	default:
		CustomKingdomUtil::Assert("error");
		return 0u;
	}
}

uint cG03PacnRollSys::nWorldStage(uint nWorldNo, uint nStageNo)
{
	uint num = nStageMax(nWorldNo);
	uint num2 = std::numeric_limits<uint>::max();
	switch (nWorldNo)
	{
	case 0u:
		if (nStageNo <= num)
		{
			num2 = nStageNo;
		}
		break;
	case 1u:
		if (nStageNo <= num)
		{
			num2 = 5 + nStageNo;
		}
		break;
	case 2u:
		if (nStageNo <= num)
		{
			num2 = 9 + nStageNo;
		}
		break;
	case 3u:
		if (nStageNo <= num)
		{
			num2 = 14 + nStageNo;
		}
		break;
	case 4u:
		if (nStageNo <= num)
		{
			num2 = 20 + nStageNo;
		}
		break;
	}
	CustomKingdomUtil::Assert(num2 != std::numeric_limits<uint>::max(), "error");
	return num2;
}

void cG03PacnRollSys::nSetRound(uint nWorldNo, uint nStageNo)
{
	nRound() = nWorldStage(nWorldNo, nStageNo);
}

bool cG03PacnRollSys::IsBossStage()
{
	return IsBossStage(m_nRound);
}

bool cG03PacnRollSys::IsBossStage(uint round)
{
	switch (round)
	{
	case 8u:
	case 13u:
	case 19u:
	case 24u:
	case 25u:
		return true;
	default:
		return false;
	}
}

bool cG03PacnRollSys::IsTopMenuMode()
{
	return m_bTopMenuMode;
}

uint cG03PacnRollSys::nFreeStage()
{
	uint result = 0u;
	SSaveDataPacnRoll::sStageRecord _out = SSaveDataPacnRoll::sStageRecord();
	for (uint num = 0u; num < 26; num++)
	{
		if (KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, num) && _out.bFree)
		{
			result = num;
		}
	}
	return result;
}

bool cG03PacnRollSys::IsAllClear()
{
	return m_bAllClear;
}

void cG03PacnRollSys::bAllClear(bool bFlag)
{
	m_bAllClear = bFlag;
}

bool cG03PacnRollSys::IsOpenStage()
{
	return m_bOpenStage;
}

void cG03PacnRollSys::bOpenStage(bool bFlag)
{
	m_bOpenStage = bFlag;
}

void cG03PacnRollSys::InitRetryData()
{
	m_RetryData = new cG03PacnRollSys::sRetryData();
	m_RetryData->nLife = 0u;
	m_RetryData->nRetryPointIdx = -1;
	m_RetryData->nAteCookie = 0u;
	m_RetryData->nHaveCookie = 0u;
	m_RetryData->vecItem.clear();
	m_RetryData->vecPoint.clear();
	m_RetryData->vecGate.clear();
	m_RetryData->vecBox.clear();
	m_RetryData->vecGhost.clear();
}

}
