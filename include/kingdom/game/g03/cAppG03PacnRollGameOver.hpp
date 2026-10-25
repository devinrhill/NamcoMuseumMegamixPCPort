#pragma once

#include "common.hpp"
#include "kingdom/game/cAppGameBaseGameOver.hpp"

namespace kingdom::game::g03
{

class cAppG03PacnRollGameOver : public cAppGameBaseGameOver
{
private:
	enum EAnita
	{
		eANITA_MAIN = 0
	};

public:
	cAppG03PacnRollGameOver();

	void Free() override;

private:
	void Init();

	void Term();
};

}
