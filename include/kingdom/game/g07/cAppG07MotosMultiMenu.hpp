#pragma once

#include "kingdom/cTaskCursor.hpp"

#include "common.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuMan.hpp"

namespace kingdom::game::g07
{

class cAppG07MotosMultiMenu : public cAppGameBaseMenu
{
private:
	kingdom::cTaskCursor* m_pCursor[4]{};   // +0x2a4

	enum EState
	{
		eSTATE_APPEAR = 0,
		eSTATE_MAIN = 1,
		eSTATE_MAX = 2
	};

	int m_nWorldMax{};

	cTaskConsMultiMenuMan* m_pTaskConsMultiMenuMan{};

protected:
	int& nWorldMax();

	cTaskConsMultiMenuMan*& pTaskConsMultiMenuMan();

public:
	cAppG07MotosMultiMenu();

	void Free() override;

private:
	void Init();

	void Term();

public:
	void FadeInEndCallBack() override;

	void TutorialOutCallBack() override;

	void MainExec() override;

};

}
