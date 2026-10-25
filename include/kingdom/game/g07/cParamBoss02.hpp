#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

class cParamBoss02 : public cKingdomCSV
{
public:
	enum PL_PARAM
	{
		eCSV_LEVEL = 0,
		eCSV_COL_HP = 1,
		eCSV_COL_BOMB_MAX = 2,
		eCSV_COL_BOMB_TIME = 3,
		eCSV_COL_ROT_SPEED = 4,
		eCSV_COL_RONDOM_ACCEL = 5,
		eCSV_COL_RANDOM_SPEED = 6,
		eCSV_COL_RANDOM_FRAME = 7,
		eCSV_COL_DOWN_FRAME = 8,
		eCSV_COL_PIYO_FRAME = 9,
		eCSV_COL_RING_HP = 10,
		eCSV_COL_SCORE = 11,
		eCSV_COL_MAX = 12,
		eCSV_ROW_LEVEL = eCSV_LEVEL,
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
	cParamBoss02();

	cParamBoss02(string key);

	virtual ~cParamBoss02();

	bool RegistData(string key);

	float GetValueF32(int row, int col);

	int GetValueS32(int row, int col);
};

}
