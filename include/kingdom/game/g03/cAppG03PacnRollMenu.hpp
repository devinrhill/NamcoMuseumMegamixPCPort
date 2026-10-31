#pragma once

#include "common.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"
#include "kingdom/game/g03/cTaskConsMenuMan.hpp"

namespace kingdom::game::g03
{

class cAppG03PacnRollMenu : public cAppGameBaseMenu
{
private:
	enum EState
	{
		eSTATE_APPEAR = 0,
		eSTATE_MAIN = 1,
		eSTATE_MAX = 2
	};

	static inline const string MENU_LZS = "g03_pacnroll/g03menu.lzs";

	int m_nWorldMax{};

	cTaskConsMenuMan* m_pTaskConsMenuMan{};

	kingdom::cTaskCursor* m_pCursor{}; // +0x2a4

protected:
	int& nWorldMax();

	cTaskConsMenuMan*& pTaskConsMenuMan();

public:
	cAppG03PacnRollMenu();

	void Free() override;

private:
	void Init();

	IEnumerator* WaitPlayBGM();

	void Term();

public:
	void FadeInEndCallBack() override;


	void TutorialOutCallBack() override;

	void MainExec() override;

};

}
