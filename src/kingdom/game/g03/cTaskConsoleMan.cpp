#include "kingdom/game/g03/cTaskConsoleMan.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGate.hpp"
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
#include "kingdom/game/g03/cTaskConsPacLife.hpp"
#include "kingdom/game/g03/cTaskConsPCookie.hpp"
#include "kingdom/game/g03/cTaskConsStageName.hpp"
#include "kingdom/game/g03/cTaskConsStart.hpp"
#include "kingdom/game/g03/cTaskConsStock.hpp"
#include "kingdom/game/g03/cTaskConsTimer.hpp"
#include "kingdom/game/g03/cTaskGate.hpp"
#include "kingdom/game/g03/cTaskItem.hpp"
#include "kingdom/game/g03/cTaskPoint.hpp"
#include "KingdomGlobal.hpp"
#include "MotosUIManager.hpp"
#include "MotosUIRollCaution.hpp"
#include "MotosUIRollStageName.hpp"
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>

namespace kingdom::game::g03
{

cTaskConsoleMan::cTaskConsoleMan()
{
	Init();
}

cTaskConsoleMan::cTaskConsoleMan(cAppG03PacnRollGame* app)
{
	SetName("cTaskConsoleMan");
	Init();
	RegistApp(app);
}

void cTaskConsoleMan::Init()
{
	UnityUIStageName = MotosUIManager::Get<MotosUIRollStageName>(MotosUIManager::EUIIndex::RollStageName);
	UnityUICaution = MotosUIManager::Get<MotosUIRollCaution>(MotosUIManager::EUIIndex::RollCaution);
	m_pApp = nullptr;
	m_pPac = nullptr;
	m_pBoss = nullptr;
	m_pGate = nullptr;
	m_pCheckPoint = nullptr;
	m_bTitle = false;
	SetClearStep(0u);
	ConnectChild(m_pTaskConsStart = new cTaskConsStart());
	ConnectChild(m_pTaskConsStageName = new cTaskConsStageName());
	ConnectChild(m_pTaskConsCookieWarning = new cTaskConsCookieWarning());
	ConnectChild(m_pTaskConsHaveCookie = new cTaskConsHaveCookie());
	ConnectChild(m_pTaskConsCookie = new cTaskConsCookie());
	ConnectChild(m_pTaskConsGate = new cTaskConsGate());
	ConnectChild(m_pTaskConsStock = new cTaskConsStock());
	ConnectChild(m_pTaskConsGhost = new cTaskConsGhost());
	ConnectChild(m_pTaskConsCheckPoint = new cTaskConsCheckPoint());
	ConnectChild(m_pTaskConsClear = new cTaskConsClear());
	ConnectChild(m_pTaskConsTimer = new cTaskConsTimer());
	ConnectChild(m_pTaskConsPacLife = new cTaskConsPacLife());
	ConnectChild(m_pTaskConsEnemyLife = new cTaskConsEnemyLife());
	ConnectChild(m_pTaskConsCombo = new cTaskConsCombo());
	ConnectChild(m_pTaskConsPCookie = new cTaskConsPCookie());
	ConnectChild(m_pTaskConsBomCursor[0] = new cTaskConsBomCursor());
	ConnectChild(m_pTaskConsBomCursor[1] = new cTaskConsBomCursor());
	ConnectChild(m_pTaskConsBomCursor[2] = new cTaskConsBomCursor());
	ConnectChild(m_pTaskConsButtCursor = new cTaskConsButtCursor());
	ConnectChild(m_pTaskConsNotCameraCtrl = new cTaskConsNotCameraCtrl());
	m_nWaitCnt = 0u;
	m_bManagerVisible = true;
	m_pTaskConsStageName->UnityUI = UnityUIStageName;
	m_pTaskConsCookieWarning->UnityUI = UnityUICaution;
}

void cTaskConsoleMan::Term()
{
	if (m_pTaskConsStageName != nullptr)
	{
		m_pTaskConsStageName->Term();
	}
	if (m_pTaskConsCookieWarning != nullptr)
	{
		m_pTaskConsCookieWarning->Term();
	}
}

bool cTaskConsoleMan::RegistApp(cAppG03PacnRollGame* app)
{
	CustomKingdomUtil::Assert(app != nullptr, "nullpo");
	if (app == nullptr)
	{
		return false;
	}
	m_pApp = app;
	return true;
}

bool cTaskConsoleMan::RegistPac(cTaskCharPacman* pPac)
{
	if (pPac == nullptr)
	{
		return false;
	}
	m_pPac = pPac;
	return true;
}

bool cTaskConsoleMan::RegistBoss(cTaskCharGolvis* pBoss)
{
	if (pBoss == nullptr)
	{
		return false;
	}
	m_pBoss = pBoss;
	return true;
}

bool cTaskConsoleMan::RegistGate(cTaskGate* pGate)
{
	if (pGate == nullptr)
	{
		return false;
	}
	m_pGate = pGate;
	return true;
}

bool cTaskConsoleMan::RegistCheckPoint(cTaskPoint* pCheckPoint)
{
	if (pCheckPoint == nullptr)
	{
		return false;
	}
	m_pCheckPoint = pCheckPoint;
	return true;
}

bool cTaskConsoleMan::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	if (!IsConsole())
	{
		return true;
	}
	cTaskItem* cTaskItem2 = m_pApp->pTaskItem();
	bool flag = KingdomGlobal::PacnRollSys->IsBossStage();
	m_pTaskConsTimer->Update(m_pApp->nGameTime());
	if (m_pApp->pTaskCamera() != nullptr)
	{
		m_pTaskConsNotCameraCtrl->Update(m_pApp->pTaskCamera()->bCameraKeepOut());
	}
	if (m_pPac != nullptr)
	{
		bool flag2 = false;
		bool flag3 = false;
		bool flag4 = false;
		bool flag5 = false;
		if (cTaskItem2 != nullptr)
		{
			flag2 = m_pTaskConsCookie->IsUpdate(cTaskItem2->GetAteCookie());
		}
		flag3 = m_pTaskConsStock->IsUpdate(m_pPac->nLife());
		flag4 = m_pTaskConsGhost->IsUpdate(m_pPac->nEatEnemy());
		m_nWaitCnt++;
		if (flag)
		{
			flag2 = false;
			flag3 = false;
		}
		if (m_pApp->nModeState() == 2)
		{
			flag5 = (InGamePadManager::GetKeyDown(InGamePadManager::GetPad1No(), PadDefine::EPadKeycode::eKEY_CIRCLE) ? true : false);
			if (flag2 | flag3 | flag4 | flag5)
			{
				m_nWaitCnt = 0u;
				if (!flag)
				{
					if (cTaskItem2 != nullptr)
					{
						m_pTaskConsCookie->Update(cTaskItem2->GetCookieMax(), cTaskItem2->GetAteCookie());
					}
					m_pTaskConsGhost->Start(m_pPac->nEatEnemy());
				}
				m_pTaskConsStock->Start(m_pPac->nLife());
			}
		}
		if (m_nWaitCnt >= 180)
		{
			m_nWaitCnt = 180u;
			if (!flag)
			{
				if (cTaskItem2 != nullptr)
				{
					m_pTaskConsCookie->End();
				}
				m_pTaskConsGhost->End();
			}
			m_pTaskConsStock->End();
		}
		if (!flag && cTaskItem2 != nullptr)
		{
			m_pTaskConsHaveCookie->Update(cTaskItem2->nHaveCookie());
		}
		m_pTaskConsPacLife->Update(m_pPac->nMaxHP(), m_pPac->nHP());
		if (m_pPac->IsState(4u))
		{
			m_pTaskConsCombo->Update(m_pPac->nEatComboCnt());
		}
	}
	if (m_pBoss != nullptr)
	{
		m_pTaskConsEnemyLife->Update(m_pBoss->nLife(), (uint)m_pBoss->GetBijikeLevel());
		m_pTaskConsPCookie->Update(m_pBoss->GetBijikeLevel(), m_pBoss->nLife());
	}
	if (m_pGate != nullptr)
	{
		cTaskBgGate* cTaskBgGate2 = m_pGate->pCurrentGate();
		if (cTaskBgGate2 != nullptr)
		{
			if (cTaskBgGate2->IsDrain() && cTaskItem2 != nullptr)
			{
				if (cTaskItem2->nHaveCookie() == 0)
				{
					m_pTaskConsCookieWarning->StartWarning();
				}
				else
				{
					m_pTaskConsCookieWarning->StartGate();
				}
			}
			m_pTaskConsGate->Update(cTaskBgGate2->nNeedCookie());
		}
		else
		{
			m_pTaskConsCookieWarning->End();
			m_pTaskConsGate->End();
		}
	}
	if (m_pCheckPoint != nullptr && m_pCheckPoint->IsFontDraw())
	{
		m_pTaskConsCheckPoint->Start();
	}
	switch (nGetClearStep())
	{
	case 1u:
	_switch1_1u:
		CustomKingdomUtil::Assert(m_pTaskConsCombo != nullptr, "error");
		m_pTaskConsCombo->End();
		SetClearStep(2u);
		goto _switch1_2u;
	case 2u:
	_switch1_2u:
		if (m_pTaskConsCombo->IsInvisible())
		{
			SetClearStep(3u);
		}
		break;
	case 3u:
	_switch1_3u:
		cTaskItem2 = m_pApp->pTaskItem();
		if (cTaskItem2->GetCookieMax() == cTaskItem2->GetAteCookie())
		{
			m_pTaskConsClear->PStart();
		}
		else
		{
			m_pTaskConsClear->Start();
		}
		SetClearStep(4u);
		break;
	default:
	_switch1_default:
		CustomKingdomUtil::Assert("unknown");
		break;
	case 0u:
	_switch1_0u:
	case 4u:
	_switch1_4u:
		break;
	}
	return true;
}

void cTaskConsoleMan::TitleStart(bool title_play)
{
	if (IsConsole() && !KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		if (title_play)
		{
			m_pTaskConsStageName->Start();
			m_bTitle = true;
		}
		if (KingdomGlobal::PacnRollSys->IsBossStage())
		{
			ConsoleStartBoss();
		}
		else
		{
			ConsoleStart();
		}
	}
}

void cTaskConsoleMan::ConsoleStart()
{
	if (!IsConsole())
	{
		return;
	}
	cTaskItem* cTaskItem2 = m_pApp->pTaskItem();
	if (m_pPac != nullptr)
	{
		m_pTaskConsHaveCookie->Start(cTaskItem2->nHaveCookie());
		m_pTaskConsCookie->Start(cTaskItem2->GetCookieMax(), cTaskItem2->GetAteCookie());
		m_pTaskConsGhost->Start(m_pPac->nEatEnemy());
		m_pTaskConsStock->Start(m_pPac->nLife());
		m_pTaskConsTimer->Start(m_pApp->nGameTime());
		m_pTaskConsPacLife->Start(m_pPac->nMaxHP());
	}
	if (m_pGate != nullptr)
	{
		cTaskBgGate* cTaskBgGate2 = m_pGate->pCurrentGate();
		if (cTaskBgGate2 != nullptr)
		{
			m_pTaskConsGate->Start(cTaskBgGate2->nNeedCookie());
		}
	}
}

void cTaskConsoleMan::ConsoleStartBoss()
{
	if (IsConsole())
	{
		if (m_pPac != nullptr)
		{
			m_pTaskConsStock->Start(m_pPac->nLife());
			m_pTaskConsPacLife->Start(m_pPac->nMaxHP());
		}
		if (m_pBoss != nullptr)
		{
			m_pTaskConsEnemyLife->Start(m_pBoss->nLife());
		}
	}
}

void cTaskConsoleMan::StartStart()
{
	if (IsConsole() && !KingdomGlobal::PacnRollSys->IsTopMenuMode() && m_bTitle)
	{
		m_bTitle = false;
		m_pTaskConsStart->Start();
	}
}

void cTaskConsoleMan::ClearStart()
{
	if (IsConsole() && !KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		SetClearStep(1u);
	}
}

bool cTaskConsoleMan::IsTitle()
{
	if (!IsConsole())
	{
		return true;
	}
	return m_pTaskConsStageName->IsPlayEnd();
}

bool cTaskConsoleMan::IsClear()
{
	if (!IsConsole())
	{
		return true;
	}
	return m_pTaskConsClear->IsPlayEnd();
}

bool cTaskConsoleMan::IsConsole()
{
	if (m_pApp == nullptr)
	{
		return false;
	}
	if (m_pApp->pTaskItem() == nullptr)
	{
		return false;
	}
	return true;
}

void cTaskConsoleMan::BomCursorStart(uint idx, Vector4 trans)
{
	CustomKingdomUtil::Assert(idx < 3, "error");
	m_pTaskConsBomCursor[idx]->Start(trans);
}

void cTaskConsoleMan::BomCursorRed(uint idx)
{
	CustomKingdomUtil::Assert(idx < 3, "error");
	m_pTaskConsBomCursor[idx]->Red();
}

void cTaskConsoleMan::BomCursorUpdate(uint idx, Vector4 trans)
{
	CustomKingdomUtil::Assert(idx < 3, "error");
	m_pTaskConsBomCursor[idx]->Update(trans);
}

void cTaskConsoleMan::BomCursorEnd(uint idx)
{
	CustomKingdomUtil::Assert(idx < 3, "error");
	m_pTaskConsBomCursor[idx]->End();
}

void cTaskConsoleMan::ButtCursorStart(Vector4 trans)
{
	m_pTaskConsButtCursor->Start(trans);
}

void cTaskConsoleMan::ButtCursorRed()
{
	m_pTaskConsButtCursor->Red();
}

void cTaskConsoleMan::ButtCursorUpdate(Vector4 trans)
{
	m_pTaskConsButtCursor->Update(trans);
}

void cTaskConsoleMan::ButtCursorEnd()
{
	m_pTaskConsButtCursor->End();
}

void cTaskConsoleMan::SetClearStep(uint step)
{
	CustomKingdomUtil::Assert(step < 5, "over");
	m_nClearStep = step;
}

uint cTaskConsoleMan::nGetClearStep()
{
	return m_nClearStep;
}

void cTaskConsoleMan::Invisible()
{
	m_bManagerVisible = false;
}

void cTaskConsoleMan::Visible()
{
	m_bManagerVisible = true;
}

bool cTaskConsoleMan::IsVisible()
{
	return m_bManagerVisible;
}

}
