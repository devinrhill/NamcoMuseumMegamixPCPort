#include "kingdom/game/g07/cTaskConsMultiMenuTutorial.hpp"

#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"
#include "MotosFrontUIManager.hpp"
#include "MotosUIRuleImage.hpp"
#include "MotosUIRuleText.hpp"
#include <Constants.hpp>
#include <DisplayManager.hpp>
#include <ELocalizeDefine.hpp>

namespace kingdom::game::g07
{

const kingdom::game::menu::sTutorialPage* cTaskConsMultiMenuTutorial::TUTORIAL_PAGE[5] = {[]() { kingdom::game::menu::sTutorialPage* _o1 = new kingdom::game::menu::sTutorialPage(); _o1->nAnmNo = -1; _o1->nTextRow = 6; _o1->Key = "ANT_MENU_TUTORIAL"; return _o1; }(), []() { kingdom::game::menu::sTutorialPage* _o2 = new kingdom::game::menu::sTutorialPage(); _o2->nAnmNo = 1; _o2->nTextRow = 7; _o2->Key = "ANT_MENU_TUTORIAL"; return _o2; }(), []() { kingdom::game::menu::sTutorialPage* _o3 = new kingdom::game::menu::sTutorialPage(); _o3->nAnmNo = -1; _o3->nTextRow = 8; _o3->Key = "ANT_MENU_TUTORIAL"; return _o3; }(), []() { kingdom::game::menu::sTutorialPage* _o4 = new kingdom::game::menu::sTutorialPage(); _o4->nAnmNo = 2; _o4->nTextRow = 9; _o4->Key = "ANT_MENU_TUTORIAL"; return _o4; }(), []() { kingdom::game::menu::sTutorialPage* _o5 = new kingdom::game::menu::sTutorialPage(); _o5->nAnmNo = 3; _o5->nTextRow = 10; _o5->Key = "ANT_MENU_TUTORIAL"; return _o5; }()};
const kingdom::game::menu::sTutorialData* cTaskConsMultiMenuTutorial::TUTORIAL_DATA = []() { kingdom::game::menu::sTutorialData* _o6 = new kingdom::game::menu::sTutorialData(); _o6->nPageMax = (uint)::Constants::ARRAY_SIZE(TUTORIAL_PAGE); _o6->TextDataName = "TEXT_MENU_TUTORIAL"; for (auto* pg : TUTORIAL_PAGE) _o6->pPage.push_back(const_cast<kingdom::game::menu::sTutorialPage*>(pg)); return _o6; }();

cTaskConsMultiMenuTutorial::~cTaskConsMultiMenuTutorial()
{
}

cTaskConsMultiMenuTutorial::cTaskConsMultiMenuTutorial()
{
	this->UnityUIImage = MotosFrontUIManager::Get<MotosUIRuleImage>(MotosFrontUIManager::EUIIndex::MotosMultiRuleImage);
	this->UnityUIText = MotosFrontUIManager::Get<MotosUIRuleText>(MotosFrontUIManager::EUIIndex::MotosMultiRuleText);
	StringBuilder stringBuilder = StringBuilder();
	string array[5]{};
	stringBuilder.clear();
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_040)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_041)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_042)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_043)).append("\n");
	array[0] = stringBuilder;
	stringBuilder.clear();
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_044)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_045)).append("\n");
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
	stringBuilder.clear();
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_046)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_047)).append("\n");
	stringBuilder.append(DisplayManager::GetMessage(ELocalizeDefine::pacmotos_048)).append("\n");
	array[4] = stringBuilder;
	if (this->UnityUIText != nullptr)
	{
		this->UnityUIText->Init(array);
	}
	_cTaskConsMenuTutorialBase("ANT_MENU_COMMON_TUTORIAL", TUTORIAL_DATA);
}

}
