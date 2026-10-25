#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g07/MOTOS_VS_PARAM.hpp"

namespace kingdom::game::g07
{

class cParamVS : public cKingdomCSV
{
public:
	enum PL_PARAM
	{
		eCSV_COL_NAME = 0,
		eCSV_COL_PARAM_A = 1,
		eCSV_COL_PARAM_B = 2,
		eCSV_COL_COMMENT = 3,
		eCSV_COL_MAX = 4,
		eCSV_ROW_NAME = eCSV_COL_NAME,
		eCSV_ROW_ENEMY_TOUCH = eCSV_COL_PARAM_A,
		eCSV_ROW_MOTOS_TOUCH = eCSV_COL_PARAM_B,
		eCSV_ROW_LAND_POINT = eCSV_COL_COMMENT,
		eCSV_ROW_ROB_POINT = eCSV_COL_MAX,
		eCSV_ROW_FALL_POINT = 5,
		eCSV_ROW_POINT_DECEL = 6,
		eCSV_ROW_PANEL_REBIRTH = 7,
		eCSV_ROW_PLAYER_MUTEKI = 8,
		eCSV_ROW_SP_CHARGE = 9,
		eCSV_ROW_SP_PIYO = 10,
		eCSV_ROW_MAX = 11
	};

private:
	MOTOS_VS_PARAM* m_VSInfo{};

protected:
	void Init() override;

	void Term() override;

private:
	void LoadValue();

public:
	cParamVS();

	cParamVS(string key);

	virtual ~cParamVS();

	bool RegistData(string key);

	MOTOS_VS_PARAM* pVSInfo();
};

}
