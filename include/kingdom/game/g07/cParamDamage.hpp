#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g07
{

class cParamDamage : public cKingdomCSV
{
public:
	enum PL_PARAM
	{
		eCSV_COL_LEVEL = 0,
		eCSV_COL_BOSS01 = 1,
		eCSV_COL_BOSS02 = 2,
		eCSV_COL_BOSS03 = 3,
		eCSV_COL_BOSS04 = 4,
		eCSV_COL_BOSS05 = 5,
		eCSV_COL_BOSS05LAST = 6,
		eCSV_COL_MAX = 7,
		eCSV_ROW_LEVEL = eCSV_COL_LEVEL,
		eCSV_ROW_LEVEL0 = eCSV_COL_BOSS01,
		eCSV_ROW_LEVEL1 = eCSV_COL_BOSS02,
		eCSV_ROW_LEVEL2 = eCSV_COL_BOSS03,
		eCSV_ROW_LEVEL3 = eCSV_COL_BOSS04,
		eCSV_ROW_LEVEL4 = eCSV_COL_BOSS05,
		eCSV_ROW_LEVEL5 = eCSV_COL_BOSS05LAST,
		eCSV_ROW_CHARGERATE = eCSV_COL_MAX,
		eCSV_ROW_MAX = 8
	};

protected:
	void Init() override;

	void Term() override;

public:
	cParamDamage();

	cParamDamage(string key);

	virtual ~cParamDamage();

	bool RegistData(string key);

	float GetValueF32(int row, int col);

	int GetValueS32(int row, int col);
};

}
