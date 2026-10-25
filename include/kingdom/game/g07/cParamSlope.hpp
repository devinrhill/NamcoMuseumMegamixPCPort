#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g07
{

class cParamSlope : public cKingdomCSV
{
public:
	enum PL_PARAM
	{
		eCSV_COL_SLOPE_PAT_NO = 0,
		eCSV_COL_SLOPE_ROT = 1,
		eCSV_COL_SLOPE_TIME = 2,
		eCSV_COL_SLOPE_STOP01 = 3,
		eCSV_COL_SLOPE_REVTIME = 4,
		eCSV_COL_SLOPE_STOP02 = 5,
		eCSV_COL_SLOPE_NUM = 6,
		eCSV_COL_COMMENT = 7,
		eCSV_COL_MAX = 8,
		eCSV_ROW_STAGE_ID = eCSV_COL_SLOPE_PAT_NO,
		eCSV_ROW_PATERN_0 = eCSV_COL_SLOPE_ROT,
		eCSV_ROW_PATERN_1 = eCSV_COL_SLOPE_TIME,
		eCSV_ROW_PATERN_2 = eCSV_COL_SLOPE_STOP01,
		eCSV_ROW_PATERN_3 = eCSV_COL_SLOPE_REVTIME,
		eCSV_ROW_PATERN_4 = eCSV_COL_SLOPE_STOP02,
		eCSV_ROW_PATERN_5 = eCSV_COL_SLOPE_NUM,
		eCSV_ROW_PATERN_6 = eCSV_COL_COMMENT,
		eCSV_ROW_PATERN_7 = eCSV_COL_MAX,
		eCSV_ROW_PATERN_8 = 9,
		eCSV_ROW_PATERN_9 = 10,
		eCSV_ROW_PATERN_10 = 11,
		eCSV_ROW_PATERN_11 = 12,
		eCSV_ROW_PATERN_12 = 13,
		eCSV_ROW_PATERN_13 = 14,
		eCSV_ROW_PATERN_14 = 15,
		eCSV_ROW_PATERN_15 = 16,
		eCSV_ROW_PATERN_16 = 17,
		eCSV_ROW_PATERN_17 = 18,
		eCSV_ROW_PATERN_18 = 19,
		eCSV_ROW_PATERN_19 = 20,
		eCSV_ROW_PATERN_20 = 21,
		eCSV_ROW_PATERN_21 = 22,
		eCSV_ROW_PATERN_22 = 23,
		eCSV_ROW_PATERN_23 = 24,
		eCSV_ROW_PATERN_24 = 25,
		eCSV_ROW_PATERN_31 = 26,
		eCSV_ROW_PATERN_32 = 27,
		eCSV_ROW_PATERN_33 = 28,
		eCSV_ROW_PATERN_34 = 29,
		eCSV_ROW_PATERN_35 = 30,
		eCSV_ROW_PATERN_36 = 31,
		eCSV_ROW_PATERN_37 = 32,
		eCSV_ROW_PATERN_99 = 33,
		eCSV_ROW_MAX = 34
	};

protected:
	void Init() override;

	void Term() override;

public:
	cParamSlope();

	cParamSlope(string key);

	virtual ~cParamSlope();

	bool RegistData(string key);

	float GetValueF32(int row, int col);

	int GetValueS32(int row, int col);

	int GetRow();
};

}
