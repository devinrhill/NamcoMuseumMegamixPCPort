#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g07
{

class cParamPlayerLevel : public cKingdomCSV
{
private:
	enum LEVEL_PARAM
	{
		eCSV_ROW_HEAD = 0,
		eCSV_ROW_LEVEL0 = 1,
		eCSV_ROW_LEVEL1 = 2,
		eCSV_ROW_LEVEL2 = 3,
		eCSV_ROW_LEVEL3 = 4,
		eCSV_ROW_LEVEL4 = 5,
		eCSV_ROW_LEVEL5 = 6,
		eCSV_ROW_LEVEL6 = 7,
		eCSV_ROW_LEVEL7 = 8,
		eCSV_ROW_LEVEL8 = 9,
		eCSV_ROW_LEVEL9 = 10,
		eCSV_ROW_LEVEL10 = 11,
		eCSV_ROW_MAX = 12,
		eCSV_COL_LEVEL = eCSV_ROW_HEAD,
		eCSV_COL_HIT_POW = eCSV_ROW_LEVEL0,
		eCSV_COL_HOVER_TIME = eCSV_ROW_LEVEL1,
		eCSV_COL_CHARGE_TIME = eCSV_ROW_LEVEL2,
		eCSV_COL_CHARGE_POW = eCSV_ROW_LEVEL3,
		eCSV_COL_CHARGE_MYPOW = eCSV_ROW_LEVEL4,
		eCSV_COL_CHARGE_HERETIC = eCSV_ROW_LEVEL5,
		eCSV_COL_CHARGE_SPEED = eCSV_ROW_LEVEL6,
		eCSV_COL_CHARGE_WEIGHT = eCSV_ROW_LEVEL7,
		eCSV_COL_CHARGE_PIYO = eCSV_ROW_LEVEL8,
		eCSV_COL_MAX = eCSV_ROW_LEVEL9,
		eLEVEL_MAX = eCSV_ROW_LEVEL10
	};

	float m_PowerLevel2Power[11]{};

	short m_HoverLevel2Time[11]{};

	short m_ChargeLevel2Time[11]{};

	float m_ChargeLevel2Power[11]{};

	float m_ChargeLevel2MyRush[11]{};

	float m_ChargeLevel2Heretic[11]{};

	float m_ChargeLevel2Speed[11]{};

	short m_ChargeLevel2Weight[11]{};

	short m_ChargeLevel2PiyoTime[11]{};

protected:
	void Init() override;

	void Term() override;

private:
	void LoadValue();

public:
	cParamPlayerLevel();

	cParamPlayerLevel(string key);

	virtual ~cParamPlayerLevel();

	bool RegistData(string key);

	float PowerLevel2Power(int n);

	short HoverLevel2Time(int n);

	short ChargeLevel2Time(int n);

	float ChargeLevel2Power(int n);

	float ChargeLevel2MyRush(int n);

	float ChargeLevel2Heretic(int n);

	float ChargeLevel2Speed(int n);

	short ChargeLevel2Weight(int n);

	short ChargeLevel2PiyoTime(int n);

	int MaxLevel();
};

}
