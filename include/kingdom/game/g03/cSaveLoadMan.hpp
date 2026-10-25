#pragma once

#include "common.hpp"
#include <SSaveDataPacnRoll.hpp>

namespace kingdom::game::g03
{

class cSaveLoadMan
{
private:
	enum EValidate
	{
		eVALIDATE_NODATA = 0,
		eVALIDATE_20070810 = 1,
		eVALIDATE_20070823 = 2,
		eVALIDATE_20070824 = 3,
		eVALIDATE_LAST = eVALIDATE_20070824
	};

	SSaveDataPacnRoll m_SaveData{};

public:
	bool IsAllClear();

	void ValidateAllClear();

	uint nDefaultLife();

	uint nDefaultHP();

	uint& nLife();

	uint& nMaxHP();

	cSaveLoadMan();

	virtual ~cSaveLoadMan();

private:
	void Init();

	void Term();

	void InitData();

public:
	bool GetStageRecord(SSaveDataPacnRoll::sStageRecord& _out, uint round);

	bool SetStageRecord(SSaveDataPacnRoll::sStageRecord set, uint round);

	void SaveData();

	void LoadData();

	void ForceAllClear();
};

}
