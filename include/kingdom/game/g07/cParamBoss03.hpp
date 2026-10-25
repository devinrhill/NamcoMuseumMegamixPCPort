#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

class cParamBoss03 : public cKingdomCSV
{
public:
	enum PL_PARAM
	{
		eCSV_COL_LEVEL = 0,
		eCSV_COL_HP = 1,
		eCSV_COL_BOMB_MAX = 2,
		eCSV_COL_BOMB_TIME = 3,
		eCSV_COL_RUSH_ACCEL = 4,
		eCSV_COL_RUSH_SPEED = 5,
		eCSV_COL_ROT_SPEED = 6,
		eCSV_COL_ROT_ACCEL = 7,
		eCSV_COL_ROT_ACCELMAX = 8,
		eCSV_COL_ROT_ACCELMIN = 9,
		eCSV_COL_ROT_ACCEL_FRAME = 10,
		eCSV_COL_ROT_SLOWDOWN_FRAME = 11,
		eCSV_COL_ROT_STOP_FRAME = 12,
		eCSV_COL_SCORE = 13,
		eCSV_COL_MAX = 14,
		eCSV_ROW_LEVEL = eCSV_COL_LEVEL,
		eCSV_ROW_NORMAL = eCSV_COL_HP,
		eCSV_ROW_EASY = eCSV_COL_BOMB_MAX,
		eCSV_ROW_HARD = eCSV_COL_BOMB_TIME,
		eCSV_ROW_MAX = eCSV_COL_RUSH_ACCEL
	};

private:
	MOTOS_VS_PARAM* m_VSInfo{};

protected:
	void Init() override;

	void Term() override;

public:
	cParamBoss03();

	cParamBoss03(string key);

	virtual ~cParamBoss03();

	bool RegistData(string key);

	float GetValueF32(int row, int col);

	int GetValueS32(int row, int col);
};

}
