#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseGameOver.hpp"

namespace kingdom::game::g07
{

class cAppG07MotosGameOver : public cAppGameBaseGameOver
{
private:
	enum EAnita
	{
		eANITA_MAIN = 0
	};

public:
	cAppG07MotosGameOver();

	void Free() override;

private:
	void Init();

	void Term();
};

}
