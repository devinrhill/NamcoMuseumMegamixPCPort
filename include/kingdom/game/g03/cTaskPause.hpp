#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskGameBasePause.hpp"

namespace kingdom::game::g03
{

class cTaskPause : public cTaskGameBasePause
{
public:
	cTaskPause();

	virtual ~cTaskPause();

private:
	void Init();

	void Term();

protected:
	void ReleaseTerm() override { cTaskPause::Term(); cTaskGameBasePause::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
