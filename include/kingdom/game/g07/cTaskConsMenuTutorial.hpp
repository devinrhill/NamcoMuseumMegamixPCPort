#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
private:
	kingdom::game::menu::sTutorialPage* TUTORIAL_PAGE[4]{};

	kingdom::game::menu::sTutorialData* TUTORIAL_DATA{};

public:
	cTaskConsMenuTutorial();

	virtual ~cTaskConsMenuTutorial();
};

}
