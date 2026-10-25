#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
private:
	kingdom::game::menu::sTutorialData* TUTORIAL_DATA{};

public:
	cTaskConsMenuTutorial();

	virtual ~cTaskConsMenuTutorial();
};

}
