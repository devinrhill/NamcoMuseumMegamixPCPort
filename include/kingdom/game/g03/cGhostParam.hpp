#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g03
{

class cGhostParam : public cKingdomCSV
{
public:
	enum ECSVKind
	{
		eCSV_KIND_START = 0,
		eCSV_KIND_CLYDE = 1,
		eCSV_KIND_PINKY = 2,
		eCSV_KIND_BLINKY = 3,
		eCSV_KIND_INKY = 4,
		eCSV_KIND_NONE_0 = 5,
		eCSV_KIND_RIDE_CAR = 6,
		eCSV_KIND_RIDE_UFO = 7,
		eCSV_KIND_RIDE_BOAT = 8,
		eCSV_KIND_RIDE_PLANE = 9,
		eCSV_KIND_RIDE_HOVER = 10,
		eCSV_KIND_RIDE_NONE = 11,
		eCSV_KIND_END = 12
	};

	enum ECSVParam
	{
		eCSV_PARAM_START = 0,
		eCSV_PARAM_SPEED = 1,
		eCSV_PARAM_ADD_SPEED = 2,
		eCSV_PARAM_TURN_SPEED = 3,
		eCSV_PARAM_HIT_SIZE = 4,
		eCSV_PARAM_END = 5
	};

private:
	bool m_bValid{};

	void Init();

	void Term();

public:
	cGhostParam();

	cGhostParam(string key);

	virtual ~cGhostParam();

	bool RegistData(string key);

	bool IsValid();

	int KindChange(int id);

	float GetGhostSpeed(int id);

	float GetGhostAddSpeed(int id);

	float GetGhostTurnSpeed(int id);

	float GetGhostHitSize(int id);
};

}
