#include "kingdom/game/g03/cTaskConsMenuTutorial.hpp"

#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"
#include "MotosFrontUIManager.hpp"
#include "MotosUIRuleImage.hpp"
#include "MotosUIRuleText.hpp"

namespace kingdom::game::g03
{

cTaskConsMenuTutorial::cTaskConsMenuTutorial()
	: TUTORIAL_DATA([&]() { kingdom::game::menu::sTutorialData* _o1 = new kingdom::game::menu::sTutorialData(); _o1->nPageMax = 3u; _o1->TextDataName = "BIN_MENU_TEXT_TUTORIAL"; _o1->pPage = {[&]() { kingdom::game::menu::sTutorialPage* _o2 = new kingdom::game::menu::sTutorialPage(); _o2->nAnmNo = 0; _o2->nTextRow = 1; _o2->Key = "BIN_ANT_TUTORIAL"; return _o2; }(), [&]() { kingdom::game::menu::sTutorialPage* _o3 = new kingdom::game::menu::sTutorialPage(); _o3->nAnmNo = 1; _o3->nTextRow = 2; _o3->Key = "BIN_ANT_TUTORIAL"; return _o3; }(), [&]() { kingdom::game::menu::sTutorialPage* _o4 = new kingdom::game::menu::sTutorialPage(); _o4->nAnmNo = -1; _o4->nTextRow = 3; _o4->Key = "BIN_ANT_TUTORIAL"; return _o4; }()}; return _o1; }())
{
	// Unity overlay text (MotosUIRuleText, DisplayManager messages) scrapped: the page text comes from BIN_MENU_TEXT_TUTORIAL (see base).
	_cTaskConsMenuTutorialBase("BIN_COMMON_ANT_TUTORIAL", TUTORIAL_DATA);
}

cTaskConsMenuTutorial::~cTaskConsMenuTutorial()
{
}

}
