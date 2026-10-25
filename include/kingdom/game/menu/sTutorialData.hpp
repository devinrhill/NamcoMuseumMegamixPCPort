#pragma once

#include "common.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"

namespace kingdom::game::menu
{

class sTutorialData
{
public:
	uint nPageMax{};

	string TextDataName{};

	std::vector<sTutorialPage*> pPage;
};

}
