#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

class cParamBoss05 : public cKingdomCSV
{
public:
	enum PL_PARAM
	{
		eCSV_COL_LEVEL = 0,
		eCSV_COL_HP = 1,
		eCSV_COL_BOMB_MAX = 2,
		eCSV_COL_BOMB_TIME = 3,
		eCSV_COL_RUSH_SPEED = 4,
		eCSV_COL_RUSH_ACCEL = 5,
		eCSV_COL_PIYO_FRAME = 6,
		eCSV_COL_RING_HP = 7,
		eCSV_COL_SLOPE_DEG = 8,
		eCSV_COL_SCORE = 9,
		eCSV_COL_LEG_HIT_POWER = 10,
		eCSV_COL_DODAI_HIT_POWER = 11,
		eCSV_COL_DODAI_HIT_SIZE = 12,
		eCSV_COL_MAX = 13,
		eCSV_ROW_LEVEL = eCSV_COL_LEVEL,
		eCSV_ROW_NORMAL = eCSV_COL_HP,
		eCSV_ROW_EASY = eCSV_COL_BOMB_MAX,
		eCSV_ROW_HARD = eCSV_COL_BOMB_TIME,
		eCSV_ROW_MAX = eCSV_COL_RUSH_SPEED
	};

private:
	MOTOS_VS_PARAM* m_VSInfo{};

protected:
	void Init() override;

	void Term() override;

public:
	cParamBoss05();

	cParamBoss05(string key);

	virtual ~cParamBoss05();

	bool RegistData(string key);

	float GetValueF32(int row, int col);

	int GetValueS32(int row, int col);
};

}
