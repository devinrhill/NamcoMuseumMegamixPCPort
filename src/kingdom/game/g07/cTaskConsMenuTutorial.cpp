#include "kingdom/game/g07/cTaskConsMenuTutorial.hpp"

#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"
#include "MotosFrontUIManager.hpp"
#include "MotosUIRuleImage.hpp"
#include "MotosUIRuleText.hpp"
#include <DisplayManager.hpp>
#include <ELocalizeDefine.hpp>

namespace kingdom::game::g07
{

cTaskConsMenuTutorial::cTaskConsMenuTutorial()
	: TUTORIAL_DATA([&]() { kingdom::game::menu::sTutorialData* _o5 = new kingdom::game::menu::sTutorialData(); _o5->nPageMax = 4u; _o5->TextDataName = "TEXT_MENU_TUTORIAL"; _o5->pPage = {[&]() { kingdom::game::menu::sTutorialPage* _o6 = new kingdom::game::menu::sTutorialPage(); _o6->nAnmNo = 0; _o6->nTextRow = 1; _o6->Key = "ANT_MENU_TUTORIAL"; return _o6; }(), [&]() { kingdom::game::menu::sTutorialPage* _o7 = new kingdom::game::menu::sTutorialPage(); _o7->nAnmNo = 1; _o7->nTextRow = 2; _o7->Key = "ANT_MENU_TUTORIAL"; return _o7; }(), [&]() { kingdom::game::menu::sTutorialPage* _o8 = new kingdom::game::menu::sTutorialPage(); _o8->nAnmNo = -1; _o8->nTextRow = 3; _o8->Key = "ANT_MENU_TUTORIAL"; return _o8; }(), [&]() { kingdom::game::menu::sTutorialPage* _o9 = new kingdom::game::menu::sTutorialPage(); _o9->nAnmNo = 2; _o9->nTextRow = 4; _o9->Key = "ANT_MENU_TUTORIAL"; return _o9; }()}; return _o5; }())
{
	TUTORIAL_PAGE[0] = [&]() { kingdom::game::menu::sTutorialPage* _o1 = new kingdom::game::menu::sTutorialPage(); _o1->nAnmNo = 0; _o1->nTextRow = 1; _o1->Key = "ANT_MENU_TUTORIAL"; return _o1; }();
	TUTORIAL_PAGE[1] = [&]() { kingdom::game::menu::sTutorialPage* _o2 = new kingdom::game::menu::sTutorialPage(); _o2->nAnmNo = 1; _o2->nTextRow = 2; _o2->Key = "ANT_MENU_TUTORIAL"; return _o2; }();
	TUTORIAL_PAGE[2] = [&]() { kingdom::game::menu::sTutorialPage* _o3 = new kingdom::game::menu::sTutorialPage(); _o3->nAnmNo = -1; _o3->nTextRow = 3; _o3->Key = "ANT_MENU_TUTORIAL"; return _o3; }();
	TUTORIAL_PAGE[3] = [&]() { kingdom::game::menu::sTutorialPage* _o4 = new kingdom::game::menu::sTutorialPage(); _o4->nAnmNo = 2; _o4->nTextRow = 4; _o4->Key = "ANT_MENU_TUTORIAL"; return _o4; }();
	this->UnityUIImage = MotosFrontUIManager::Get<MotosUIRuleImage>(MotosFrontUIManager::EUIIndex::MotosRuleImage);
	this->UnityUIText = MotosFrontUIManager::Get<MotosUIRuleText>(MotosFrontUIManager::EUIIndex::MotosRuleText);
	StringBuilder stringBuilder = StringBuilder();
	string array[5]{};
	stringBuilder.clear();
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_006)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_007)).append("\n");
	array[0] = stringBuilder;
	stringBuilder.clear();
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_009)).append("\n");
	array[1] = stringBuilder;
	stringBuilder.clear();
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_016)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_017)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_018)).append("\n");
	array[2] = stringBuilder;
	stringBuilder.clear();
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_019)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_020)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_021)).append("\n");
	array[3] = stringBuilder;
	if (this->UnityUIText != nullptr)
	{
		this->UnityUIText->Init(array);
	}
	_cTaskConsMenuTutorialBase("ANT_MENU_COMMON_TUTORIAL", TUTORIAL_DATA);
}

cTaskConsMenuTutorial::~cTaskConsMenuTutorial()
{
}

}
