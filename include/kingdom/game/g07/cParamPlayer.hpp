#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"

namespace kingdom::game::g07
{

class cParamPlayer : public cKingdomCSV
{
private:
	enum PL_PARAM
	{
		eCSV_ROW_HEAD = 0,
		eCSV_ROW_NORMAL_PARAM = 1,
		eCSV_ROW_EASY_PARAM = 2,
		eCSV_ROW_HARD_PARAM = 3,
		eCSV_ROW_MAX = 4,
		eCSV_COL_PLAYER_ID = eCSV_ROW_HEAD,
		eCSV_COL_WEIGHT = eCSV_ROW_NORMAL_PARAM,
		eCSV_COL_HERETIC_TIME = eCSV_ROW_EASY_PARAM,
		eCSV_COL_MAX_SPEED = eCSV_ROW_HARD_PARAM,
		eCSV_COL_ACCEL_SPEED = eCSV_ROW_MAX,
		eCSV_COL_DECEL_SPEED = 5,
		eCSV_COL_HIT_POW = 6,
		eCSV_COL_CHARGE_POW = 7,
		eCSV_COL_SLOPE_POW = 8,
		eCSV_COL_WALL_REF = 9,
		eCSV_COL_HIT_SIZE = 10,
		eCSV_COL_HIT_WSIZE = 11,
		eCSV_COL_POINT = 12,
		eCSV_COL_PARAM01 = 13,
		eCSV_COL_PARAM02 = 14,
		eCSV_COL_PARAM03 = 15,
		eCSV_COL_COMMENT = 16,
		eCSV_COL_MAX = 17
	};

	MO_CHAR_PARAMS* m_sParam{};

protected:
	void Init() override;

	void Term() override;

private:
	void LoadValue();

public:
	cParamPlayer();

	cParamPlayer(string key);

	virtual ~cParamPlayer();

	bool RegistData(string key);

	MO_CHAR_PARAMS* pMotosParams();
};

}
