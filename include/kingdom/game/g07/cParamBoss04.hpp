#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

class cParamBoss04 : public cKingdomCSV
{
public:
	enum PL_PARAM
	{
		eCSV_COL_LEVEL = 0,
		eCSV_COL_HP = 1,
		eCSV_COL_BOMB_MAX = 2,
		eCSV_COL_BOMB_TIME = 3,
		eCSV_COL_ROT_SPEED = 4,
		eCSV_COL_RANDOM_ACCEL = 5,
		eCSV_COL_RANDOM_SPEED = 6,
		eCSV_COL_SLOPE_ACCEL = 7,
		eCSV_COL_SLOPE_SPEED = 8,
		eCSV_COL_RANDOM_FRAME = 9,
		eCSV_COL_DOWN_FRAME = 10,
		eCSV_COL_PIYO_FRAME = 11,
		eCSV_COL_RING_HP = 12,
		eCSV_COL_SCORE = 13,
		eCSV_COL_SLOPE_TIME = 14,
		eCSV_COL_SLOPE_STOP_TIME = 15,
		eCSV_COL_SLOPE_DEG = 16,
		eCSV_COL_ROT_ACCELMAX = 17,
		eCSV_COL_ROT_ACCEL_FRAME = 18,
		eCSV_COL_MAX = 19,
		eCSV_ROW_LEVEL = eCSV_COL_LEVEL,
		eCSV_ROW_NORMAL = eCSV_COL_HP,
		eCSV_ROW_EASY = eCSV_COL_BOMB_MAX,
		eCSV_ROW_HARD = eCSV_COL_BOMB_TIME,
		eCSV_ROW_MAX = eCSV_COL_ROT_SPEED
	};

private:
	MOTOS_VS_PARAM* m_VSInfo{};

protected:
	void Init() override;

	void Term() override;

public:
	cParamBoss04();

	cParamBoss04(string key);

	virtual ~cParamBoss04();

	bool RegistData(string key);

	float GetValueF32(int row, int col);

	int GetValueS32(int row, int col);
};

}
