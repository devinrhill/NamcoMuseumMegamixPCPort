#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g03
{

class cGhostCommonParam : public cKingdomCSV
{
public:
	enum ECSVParam
	{
		eCSV_PARAM_START = 0,
		eCSV_PARAM_IJIKE_SPEED = 1,
		eCSV_PARAM_IJIKE_FLASH = 2,
		eCSV_PARAM_BOMB_DIRE = 3,
		eCSV_PARAM_BOMB_ATTACK = 4,
		eCSV_PARAM_END = 5
	};

private:
	bool m_bValid{};

	void Init();

	void Term();

public:
	cGhostCommonParam();

	cGhostCommonParam(string key);

	virtual ~cGhostCommonParam();

	bool RegistData(string key);

	bool IsValid();

	float GetGhostCommonIzikeSpeed();

	int GetGhostCommonIzikeFlash();

	float GetGhostBombDire();

	float GetGhostBombAttackArea();
};

}
