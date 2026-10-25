#pragma once

#include "kingdom/cTaskCursor.hpp"

#include "common.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/g07/cTaskConsMenuMan.hpp"

namespace kingdom::game::g07
{

class cAppG07MotosMenu : public cAppGameBaseMenu
{
private:
	kingdom::cTaskCursor* m_pCursor{};   // +0x2a4

	enum EState
	{
		eSTATE_APPEAR = 0,
		eSTATE_MAIN = 1,
		eSTATE_MAX = 2
	};

	int m_nWorldMax{};

	cTaskConsMenuMan* m_pTaskConsMenuMan{};

protected:
	int& nWorldMax();

	cTaskConsMenuMan*& pTaskConsMenuMan();

public:
	cAppG07MotosMenu();

	void Free() override;

private:
	void Init();

	IEnumerator* WaitPlayBGM();

	void Term();

public:
	void FadeInEndCallBack() override;

	void FadeInStartCallBack() override;

	void TutorialOutCallBack() override;

	void MainExec() override;

	void FadeOutEndCallBack() override;
};

}
