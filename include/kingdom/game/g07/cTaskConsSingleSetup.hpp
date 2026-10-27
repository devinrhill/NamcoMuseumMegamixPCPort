#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "kingdom/game/g07/MO_BG_COLOR.hpp"
#include "MotosUIMotosBossText.hpp"
#include "MotosUIMotosStartText.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/game/g07/cTaskConsPacIconSingleSetting.hpp"

namespace kingdom::game::g07
{

class cTaskConsSingleSetup : public cTaskConsoleBase
{
private:
	enum EMaxConsole
	{
		MAX_CONSOLE_SETUP_NUMBER = 3
	};

	enum EAnm
	{
		eANM_SEL_BAR = 0,
		eANM_POWER = 1,
		eANM_JUMP = 2,
		eANM_CHARGE = 3,
		eANM_MESSAGE = 4,
		eANM_HINT1 = 5,
		eANM_HINT2 = 6,
		eANM_MAX = 7,
		eANM_NORMAL_STAGE_MAX = eANM_HINT1
	};

	static const float aHeight[3];

	static const int nTikiAnm[4];

	static const int nAnmSelectItem[4];

	static const int nAnmNoSelectItem[4];

	static const uint nAnmBossHintWait[10];

	static const int nAnmBossHintOut[10];

	static const uint AnimNo[6];

	cTaskConsPacIconSingleSetting* m_pPacIcon{};

	cTaskConsItemNumber* m_pItemNumber[3]{};

	Ref<int>* m_aExecNumber[3]{};

	MO_BG_COLOR m_sBgColor{};

public:
	MotosUIMotosStartText* UnityUIStart{};

	MotosUIMotosBossText* UnityUIBoss{};

	cTaskConsSingleSetup();

	static cTaskAnita* Construct();

private:
	void Init();

public:
	void Term();

	int IsBossStageNo();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nState, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskConsSingleSetup::Term(); cTaskConsoleBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
