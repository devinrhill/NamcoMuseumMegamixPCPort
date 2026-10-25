#pragma once

#include "common.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"

namespace kingdom::game
{

class cAppGameBaseMenu : public cAppKingdom
{
private:
	enum EState
	{
		eSTATE_FADE_IN = 0,
		eSTATE_MAIN = 1,
		eSTATE_TUTORIAL = 2,
		eSTATE_FADE_OUT = 3,
		eSTATE_END = 4
	};

	cTaskFade* m_pTaskFade{};

	int m_nState{};

	int m_nStep{};

	uint m_nNextJob{};

	int m_nStageNo{};

	int m_nWorldNo{};

	bool m_bAllStop{};

	bool m_bTutorial{};

	kingdom::game::menu::cTaskConsMenuTutorialBase* m_pTaskTutorial{};

protected:
	cTaskFade*& pTaskFade();

	int& nStageNo();

	int& nWorldNo();

	bool& bTutorial();

	kingdom::game::menu::cTaskConsMenuTutorialBase*& pTaskTutorial();

public:
	cAppGameBaseMenu();

	void Free() override;

	void nSetNextJob(uint nNo);

	virtual void MainInCallBack();

	virtual void MainExec() = 0;

	virtual void MainOutCallBack();

	virtual void TutorialOutCallBack();

	virtual void PlaySndSe(const string& pSndName, int ctrl = 0);

	virtual void TutorialStart();

	virtual void TutorialEnd();

private:
	void Init();

	void Term();

public:
	virtual void TutorialInCallBack();

	virtual void TutorialExec();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	virtual void FadeInStartCallBack();

	virtual void FadeInEndCallBack();

	virtual void FadeOutStartCallBack();

	virtual void FadeOutEndCallBack();

	bool IsSndAllStop();

	void SetSndAllStop(bool sw);

	void HomeButtonCheck();
};

}
