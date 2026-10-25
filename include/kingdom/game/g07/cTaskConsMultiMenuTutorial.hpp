#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuTutorial : public kingdom::game::menu::cTaskConsMenuTutorialBase
{
private:
	static const kingdom::game::menu::sTutorialPage* TUTORIAL_PAGE[5];

	static const kingdom::game::menu::sTutorialData* TUTORIAL_DATA;

public:
	virtual ~cTaskConsMultiMenuTutorial();

	cTaskConsMultiMenuTutorial();
};

}
