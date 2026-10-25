#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g07
{

class cParamAxisAttack : public cKingdomCSV
{
private:
	enum eAxisFormat
	{
		eCSV_ROW_HEAD = 0,
		eCSV_ROW_STOP = 1,
		eCSV_ROW_FRONT = 2,
		eCSV_ROW_FRONT_SIDE = 3,
		eCSV_ROW_SIDE = 4,
		eCSV_ROW_BACK_SIDE = 5,
		eCSV_ROW_BACK = 6,
		eCSV_ROW_MAX = 7,
		eCSV_COL_LABEL = eCSV_ROW_HEAD,
		eCSV_COL_NORMAL = eCSV_ROW_STOP,
		eCSV_COL_EASY = eCSV_ROW_FRONT,
		eCSV_COL_HARD = eCSV_ROW_FRONT_SIDE,
		eCSV_COL_MAX = eCSV_ROW_SIDE,
		eFACTER_MAX = eCSV_ROW_BACK,
		eLEVEL_MAX = eCSV_ROW_FRONT_SIDE
	};

	float m_aaAxisParam[3][6]{};

protected:
	void Init() override;

	void Term() override;

private:
	void LoadValue();

public:
	cParamAxisAttack();

	cParamAxisAttack(string key);

	virtual ~cParamAxisAttack();

	bool RegistData(string key);

	float* pAxisAttackLevel(uint level);

	float pAxisAttackLevelFacter(uint level, uint facter);
};

}
