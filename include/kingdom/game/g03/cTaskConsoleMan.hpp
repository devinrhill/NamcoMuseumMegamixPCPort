#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "MotosUIRollCaution.hpp"
#include "MotosUIRollStageName.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskConsBomCursor.hpp"
#include "kingdom/game/g03/cTaskConsButtCursor.hpp"
#include "kingdom/game/g03/cTaskConsCheckPoint.hpp"
#include "kingdom/game/g03/cTaskConsClear.hpp"
#include "kingdom/game/g03/cTaskConsCombo.hpp"
#include "kingdom/game/g03/cTaskConsCookie.hpp"
#include "kingdom/game/g03/cTaskConsCookieWarning.hpp"
#include "kingdom/game/g03/cTaskConsEnemyLife.hpp"
#include "kingdom/game/g03/cTaskConsGate.hpp"
#include "kingdom/game/g03/cTaskConsGhost.hpp"
#include "kingdom/game/g03/cTaskConsHaveCookie.hpp"
#include "kingdom/game/g03/cTaskConsNotCameraCtrl.hpp"
#include "kingdom/game/g03/cTaskConsPCookie.hpp"
#include "kingdom/game/g03/cTaskConsPacLife.hpp"
#include "kingdom/game/g03/cTaskConsStageName.hpp"
#include "kingdom/game/g03/cTaskConsStart.hpp"
#include "kingdom/game/g03/cTaskConsStock.hpp"
#include "kingdom/game/g03/cTaskConsTimer.hpp"
#include "kingdom/game/g03/cTaskGate.hpp"
#include "kingdom/game/g03/cTaskPoint.hpp"

namespace kingdom::game::g03
{

class cTaskConsoleMan : public cTaskKingdom
{
private:
	enum EBomCursor
	{
		BOM_CURSOR_MAX = 3
	};

	enum EClearStep
	{
		eCLEAR_STEP_NOCLEAR = 0,
		eCLEAR_STEP_INIT = 1,
		eCLEAR_STEP_COMBOWAIT = 2,
		eCLEAR_STEP_DISP = 3,
		eCLEAR_STEP_END = 4,
		eCLEAR_STEP_MAX = 5
	};

	cAppG03PacnRollGame* m_pApp{};

	cTaskCharPacman* m_pPac{};

	cTaskCharGolvis* m_pBoss{};

	cTaskGate* m_pGate{};

	cTaskPoint* m_pCheckPoint{};

	cTaskConsStart* m_pTaskConsStart{};

	cTaskConsStageName* m_pTaskConsStageName{};

	cTaskConsClear* m_pTaskConsClear{};

	cTaskConsTimer* m_pTaskConsTimer{};

	cTaskConsNotCameraCtrl* m_pTaskConsNotCameraCtrl{};

	cTaskConsCookieWarning* m_pTaskConsCookieWarning{};

	cTaskConsGate* m_pTaskConsGate{};

	cTaskConsCheckPoint* m_pTaskConsCheckPoint{};

	cTaskConsHaveCookie* m_pTaskConsHaveCookie{};

	cTaskConsCookie* m_pTaskConsCookie{};

	cTaskConsStock* m_pTaskConsStock{};

	cTaskConsGhost* m_pTaskConsGhost{};

	cTaskConsPacLife* m_pTaskConsPacLife{};

	cTaskConsCombo* m_pTaskConsCombo{};

	cTaskConsEnemyLife* m_pTaskConsEnemyLife{};

	cTaskConsPCookie* m_pTaskConsPCookie{};

	cTaskConsBomCursor* m_pTaskConsBomCursor[3]{};

	cTaskConsButtCursor* m_pTaskConsButtCursor{};

	uint m_nWaitCnt{};

	bool m_bTitle{};

	uint m_nClearStep{};

	bool m_bManagerVisible{};

public:
	MotosUIRollStageName* UnityUIStageName{};

	MotosUIRollCaution* UnityUICaution{};

	cTaskConsoleMan();

	cTaskConsoleMan(cAppG03PacnRollGame* app);

private:
	void Init();

public:
	void Term();

	bool RegistApp(cAppG03PacnRollGame* app);

	bool RegistPac(cTaskCharPacman* pPac);

	bool RegistBoss(cTaskCharGolvis* pBoss);

	bool RegistGate(cTaskGate* pGate);

	bool RegistCheckPoint(cTaskPoint* pCheckPoint);

	bool Execute(cJetTask* pParam) override;

	void TitleStart(bool title_play = true);

private:
	void ConsoleStart();

	void ConsoleStartBoss();

public:
	void StartStart();

	void ClearStart();

	bool IsTitle();

	bool IsClear();

	bool IsConsole();

	void BomCursorStart(uint idx, Vector4 trans);

	void BomCursorRed(uint idx);

	void BomCursorUpdate(uint idx, Vector4 trans);

	void BomCursorEnd(uint idx);

	void ButtCursorStart(Vector4 trans);

	void ButtCursorRed();

	void ButtCursorUpdate(Vector4 trans);

	void ButtCursorEnd();

private:
	void SetClearStep(uint step);

	uint nGetClearStep();

public:
	void Invisible();

	void Visible();

	bool IsVisible();

protected:
	void ReleaseTerm() override { cTaskConsoleMan::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
