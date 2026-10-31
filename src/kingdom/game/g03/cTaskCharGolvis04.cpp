#include "kingdom/game/g03/cTaskCharGolvis04.hpp"
#include "kingdom/game/g03/cTaskConsoleMan.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharGear1.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/KingdomJobTable.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

const Constants::gmSGolvisAttack* cTaskCharGolvis04::s_AttackTBL[4] = {[]() { Constants::gmSGolvisAttack* _o1 = new Constants::gmSGolvisAttack(); _o1->uiKind = 3; _o1->fRangeMin = 0.0f; _o1->fRangeMax = 6.0f; _o1->iTimeMin = 5; _o1->iTimeMax = 10; return _o1; }(), []() { Constants::gmSGolvisAttack* _o2 = new Constants::gmSGolvisAttack(); _o2->uiKind = 9; _o2->fRangeMin = 3.0f; _o2->fRangeMax = 10.0f; _o2->iTimeMin = 5; _o2->iTimeMax = 10; return _o2; }(), []() { Constants::gmSGolvisAttack* _o3 = new Constants::gmSGolvisAttack(); _o3->uiKind = 8; _o3->fRangeMin = 3.0f; _o3->fRangeMax = 10.0f; _o3->iTimeMin = 5; _o3->iTimeMax = 10; return _o3; }(), []() { Constants::gmSGolvisAttack* _o4 = new Constants::gmSGolvisAttack(); _o4->uiKind = 0; return _o4; }()};

cTaskCharGolvis04::cTaskCharGolvis04(cJetTask* pParent)
	: cTaskCharGolvis(pParent), m_hDemoPtk(new cKingdomPtkHnd())
{
	SetName("cTaskCharGolvis04");
	Init();
}

// DOL 0x8011bc7c [audited]
void cTaskCharGolvis04::Init()
{
	::Constants::jetGeoVecSet(vTrans(), 0.0f, 0.0f, 0.0f, 1.0f);
	m_nPacNowGearNo = -1;
	ChangeDeathProc(0u);
	bUseDeathDemo() = true;
	m_nDemoTimer = 0;
	m_nMoveTimer = 0;
	m_nAttackPatern = -1;
	Setting(false);
	m_pGear = new cTaskCharGear1(this);
	m_pGear->nFlag() |= 2u;
	DemoPacManCreate();
}

void cTaskCharGolvis04::DemoPacManCreate()
{
	m_pDemoPacMan = new cTaskChar();
	cKingdomModel* obj = m_pDemoPacMan->CreateModel();
	obj->RegistModel("BIN_PAC_NORMAL_MDL");
	obj->IsDrawShadowDepth(true);
	obj->SetLightSetIdx(2);
	obj->SetFogIdx(2);
	obj->Invisible();
	// (the C# widened the Unity bounds here; the DOL registers with flags 0x36f, FUN_8001f72c)
	ConnectChild(m_pDemoPacMan);
}

// DOL 0x8011bdac [audited]
void cTaskCharGolvis04::Term()
{
	// DOL blr (0x8011bdac): the demo Pac-Man is a child task and terminates with the tree
}

void cTaskCharGolvis04::ReleaseTerm()
{
	cTaskCharGolvis04::Term();
	m_hDemoPtk->Detach(true);   // ~cKingdomPtkHnd of +0xa10
	delete m_pDemoCamera;       // the scene camera task dies with the tree in the DOL (connected to this task)
	m_pDemoCamera = nullptr;
	cTaskCharGolvis::ReleaseTerm();
}

// DOL 0x8011bdb0 [audited: thresholds 10 / 20, timer 40 vs decomp]
bool cTaskCharGolvis04::IsForceAttack()
{
	int gear = m_pGear->GetGear(Constants::PACMAN_OBJ()->vTrans());
	Vector4 v;
	::Constants::jetGeoVecSub(v, Constants::PACMAN_OBJ()->vTrans(), vTrans());
	if (gear >= 4)
	{
		if (m_nMoveTimer == 40 && ::Constants::jetGeoVecLength(v) < 10.0f)
		{
			return true;
		}
	}
	else
	{
		if (gear >= 2)
		{
			return false;
		}
		if (m_nMoveTimer == 40 && ::Constants::jetGeoVecLength(v) < 20.0f)
		{
			return true;
		}
	}
	return false;
}

// DOL 0x8011beb8 [audited]
bool cTaskCharGolvis04::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss())
	{
		pHitChar(1)->FlagOff(17u);
		MonsterFrameMove();
	}
	return cTaskCharGolvis::Execute(pParam);
}

void cTaskCharGolvis04::Draw(int nStatus, cJetTask* pParam)
{
	cTaskCharGolvis::Draw(nStatus, pParam);
}

// DOL 0x8011bf64 [audited: gear tests and kinds 8/9/14 vs disassembly]
bool cTaskCharGolvis04::ChangeToAttack(int iAttackKind)
{
	if (m_pGear->GetGear(vTrans()) < 2)
	{
		return false;
	}
	int gear = m_pGear->GetGear(Constants::PACMAN_OBJ()->vTrans());
	if (gear >= 4)
	{
		if (iAttackKind == 8 || iAttackKind == 9 || iAttackKind == 14)
		{
			return cTaskCharGolvis::ChangeToAttack(iAttackKind);
		}
		return false;
	}
	if (gear >= 2)
	{
		if (iAttackKind == 8 || iAttackKind == 9)
		{
			return false;
		}
		return cTaskCharGolvis::ChangeToAttack(iAttackKind);
	}
	if (iAttackKind == 8 || iAttackKind == 14)
	{
		m_nPacNowGearNo = -1;
		return cTaskCharGolvis::ChangeToAttack(iAttackKind);
	}
	return false;
}

// DOL 0x8011c060 [audited]
void cTaskCharGolvis04::Setting(bool bRelocate)
{
	cTaskCharGolvis::Setting(bRelocate);
	nPushOutLevel() = 2u;
	m_iState = cTaskCharGolvis04::gmEMonsterStateGolvisGear1::STATE_GOLVISGEAR1_MOVE;
	InitStateFirst();
}

// DOL 0x8011c0b0 [audited]
void cTaskCharGolvis04::InitStateFirst()
{
	ChangeToAppear();
	m_bDisp = true;
}

// DOL 0x8011c0f0 [audited]
void cTaskCharGolvis04::InitStateStart()
{
	m_iAIMode = 2;
	m_iState = cTaskCharGolvis04::gmEMonsterStateGolvisGear1::STATE_GOLVISGEAR1_MOVE;
}

// DOL 0x8011c104 [audited]
void cTaskCharGolvis04::MonsterFrameMove_PreProcess()
{
	int subState = GetSubState();
	if ((subState == 10 || subState == 11 || subState == 12) && GetCount(0u) != 0)
	{
		m_pGear->nFlag() |= 2u;
	}
	if (GetAIMode() == 13)
	{
		uint num = 2u;
		m_pGear->nFlag() &= ~num;
	}
}

// DOL 0x8011c150 [audited: 43rd-session pass; not re-read now]
void cTaskCharGolvis04::MonsterFrameMove_Free()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	if (nType() != 2)
	{
		uint num = 2u;
		m_pGear->nFlag() &= ~num;
		m_pGear->SetLevel(3 - nLife());
	}
	if (m_iState != cTaskCharGolvis04::gmEMonsterStateGolvisGear1::STATE_GOLVISGEAR1_MOVE)
	{
		return;
	}
	int gear = m_pGear->GetGear(vTrans());
	if (m_nMoveTimer != 0)
	{
		m_nMoveTimer--;
	}
	Vector4 v2;
	if ((gear == 2 || gear == 3) && m_nMoveTimer == 0)
	{
		::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, m_fMoveSpeed, 0.01f);
		::Constants::bizMathNearTransXZ(v2, vTrans(), v, m_fMoveSpeedNow);
		::Constants::jetGeoVecSub(v2, v2, vTrans());
		::Constants::jetGeoVecAdd(vTrans(), vTrans(), v2);
	}
	else
	{
		if (!CustomKingdomUtil::IF(m_nMoveTimer))
		{
			m_nMoveTimer = 40;
		}
		::Constants::jetGeoVecSub(v2, Constants::PACN_APP()->pTaskBgGimmick(2u)->Model->vTrans(), vTrans());
		float num2 = ::Constants::jetGeoVecLength(v2);
		::Constants::jetGeoVecSub(v2, Constants::PACN_APP()->pTaskBgGimmick(3u)->Model->vTrans(), vTrans());
		float num3 = ::Constants::jetGeoVecLength(v2);
		uint num4 = 2u;
		if (num3 < num2)
		{
			num4 = 3u;
		}
		::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, m_fMoveSpeed, 0.01f);
		::Constants::bizMathNearTransXZ(v2, vTrans(), Constants::PACN_APP()->pTaskBgGimmick(num4)->Model->vTrans(), m_fMoveSpeedNow);
		::Constants::jetGeoVecSub(v2, v2, vTrans());
		::Constants::jetGeoVecAdd(vTrans(), vTrans(), v2);
	}
	if (m_iAIMode == 2)
	{
		TryChangeToAttack();
		TryChangeToLaugh();
	}
}

// DOL 0x8011c3a0 [audited: 43rd-session pass; not re-read now]
void cTaskCharGolvis04::MonsterFrameMove_Escape()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	Vector4 _discard5;
	::Constants::jetGeoVecCopy(_discard5, vTrans());
	int gear = m_pGear->GetGear(vTrans());
	if (m_nMoveTimer != 0)
	{
		m_nMoveTimer--;
	}
	if ((gear == 2 || gear == 3) && m_nMoveTimer == 0)
	{
		m_pGear->nFlag() |= 2u;
		v[1] = vTrans()[1];
		::Constants::jetGeoVecSub(v, vTrans(), v);
		::Constants::jetGeoVecAdd(v, vTrans(), v);
		::Constants::jetGeoVecSub(v, v, vTrans());
		if (vTrans()[1] > vFirstTrans()[1])
		{
			vTrans()[1]--;
		}
		else
		{
			vTrans()[1] = vFirstTrans()[1];
		}
		::Constants::bizMathNearTransXZ(vTrans(), vTrans(), v, m_fBijikeMoveSpeed);
		return;
	}
	if (!CustomKingdomUtil::IF(m_nMoveTimer))
	{
		m_nMoveTimer = 20;
	}
	Vector4 v3;
	::Constants::jetGeoVecSub(v3, Constants::PACN_APP()->pTaskBgGimmick(2u)->Model->vTrans(), vTrans());
	float num = ::Constants::jetGeoVecLength(v3);
	::Constants::jetGeoVecSub(v3, Constants::PACN_APP()->pTaskBgGimmick(3u)->Model->vTrans(), vTrans());
	float num2 = ::Constants::jetGeoVecLength(v3);
	uint num3 = 2u;
	if (num2 < num)
	{
		num3 = 3u;
	}
	::Constants::bizMathNearTransXZ(v3, vTrans(), Constants::PACN_APP()->pTaskBgGimmick(num3)->Model->vTrans(), m_fBijikeMoveSpeed);
	::Constants::jetGeoVecSub(v3, v3, vTrans());
	::Constants::jetGeoVecAdd(vTrans(), vTrans(), v3);
}

// DOL 0x8011c5e4 [audited: death demo, 43rd-session pass; not re-read now]
void cTaskCharGolvis04::ChangeToDamage()
{
	vTrans()[1] = vFirstTrans()[1];
	cTaskCharGolvis::ChangeToDamage();
}

// the demo's per-frame animation calls (DOL 0x8011c62c [audited: per AUDIT_G03_G07 row], procs 2 and 3)
void cTaskCharGolvis04::DemoUpdateFrame()
{
	m_pDemoPacMan->UpdateFrame();                                  // FUN_8009a478
	if (m_pDemoCamera != nullptr)
	{
		m_pDemoCamera->UpdateFrame();                              // FUN_8014a43c(app +0x2b0)
	}
	Constants::PACN_APP()->pTaskBgGimmick()->Model->UpdateFrame();     // gimmick[0] +0x50 vtable +0x14
	Constants::PACN_APP()->pTaskBgGimmick(1u)->Model->UpdateFrame();   // gimmick[1]
}

// 43rd session: rewritten from the DOL. The C# drove the demo with Unity camera animation clips whose clock was stubbed (always frame 0),
// so the demo never left its first step; the DOL counts frames at +0x9f4 (m_nDemoTimer) and plays the camera as a SCN0 scene camera.
// DOL 0x8011c62c
bool cTaskCharGolvis04::MonsterFrameMove_Death()
{
	if (!IsBossDeath())
	{
		return false;
	}
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	switch (GetDeathProc())
	{
	case 0u:
		Constants::PACN_APP()->pTaskFade()->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK);   // (0x10002, 500)
		Constants::PACN_APP()->pTaskItem()->SetBossCookie(0u);
		ChangeDeathProc(1u);
		break;
	case 1u:
		if (Constants::PACN_APP()->pTaskFade()->IsEnd())
		{
			Constants::PACN_APP()->pTaskConsole()->Invisible();
			nType() = 0u;
			Constants::PACN_APP()->pTaskBgGimmick()->Model->RegistAnm(0u, "BIN_DEMO_ANM_DOOR_L");
			Constants::PACN_APP()->pTaskBgGimmick(1u)->Model->RegistAnm(0u, "BIN_DEMO_ANM_DOOR_R");
			::Constants::jetGeoVecUnit(vTrans());
			::Constants::jetGeoVecUnit(vRotate());
			m_pDemoPacMan->pModel()->Visible();
			::Constants::jetGeoVecUnit(m_pDemoPacMan->vTrans());
			::Constants::jetGeoVecUnit(m_pDemoPacMan->vRotate());
			m_pDemoPacMan->vTrans()[1] = 0.5f;
			Vector4 v = Vector4::zero;
			Vector4 v2 = Vector4::zero;
			::Constants::jetGeoVecUnit(v);
			::Constants::jetGeoVecUnit(v2);
			Constants::PACN_APP()->SetEffect(37u, v, v2, m_hDemoPtk);
			for (uint num = 0u; num < 3; num++)
			{
				cTaskCharPacman2->pModel(num)->Invisible();
			}
			SetAnmState(33u);
			m_nDemoTimer = 0;
			m_pDemoPacMan->pModel()->RegistAnm(0u, "BIN_DEMO_BOS_ANM_PAC");
			Constants::PACN_APP()->pTaskFade()->Fade(EFadeKind::eFADE_TYPE_IN_BLACK);   // (2, 500)
			vScale()[0] = 1.0f;
			vScale()[1] = vScale()[0];
			vScale()[2] = vScale()[0];
			// new cTaskSceneCamera(ScnRoot) -> app +0x2b0, FUN_80149f74(cam, 0, BIN_DEMO_BOS_CAMERA), FUN_8014a388(cam, 0), ConnectChild
			delete m_pDemoCamera;
			m_pDemoCamera = new cDemoSceneCamera();
			m_pDemoCamera->Bind("BIN_DEMO_BOS_CAMERA");
			m_pDemoCamera->SetAnm();
			cDemoSceneCamera::Active() = m_pDemoCamera;
			Constants::PACN_APP()->pTaskCharPacman()->DisposeEffect();
			Constants::PACN_APP()->StopIjikeSnd();
			ChangeDeathProc(2u);
		}
		else
		{
			vScale()[0] = (float)(25 - m_iCount[0]) / 25.0f;
			vScale()[1] = vScale()[0];
			vScale()[2] = vScale()[0];
		}
		break;
	case 2u:
		::Constants::jetGeoVecUnit(vRotate());
		DemoUpdateFrame();
		::Constants::jetGeoVecUnit(m_pDemoPacMan->vTrans());
		::Constants::jetGeoVecUnit(m_pDemoPacMan->vRotate());
		switch (m_nDemoTimer)   // SEs without a sound actor
		{
		case 0x1e:
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE099_GOL_TOUJYOU);
			break;
		case 0x162:
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE084_BOSS_IJIKE_1);
			break;
		case 0x180:
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE080_BOSS_TATAKI);
			break;
		case 0x1a4:
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE101_GOL_IJIKE);
			break;
		case 0x27b:
		case 0x2c6:
		case 0x2e7:
		case 0x30f:
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE031_JUMP);
			break;
		case 0x334:
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE022_DASH);
			break;
		}
		if (m_nDemoTimer++ == 0x3b6)
		{
			Constants::PACN_APP()->pTaskFade()->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE);   // (0x10001, 500)
			ChangeDeathProc(3u);
		}
		break;
	case 3u:
		if (++m_nDemoTimer < 0x3de)
		{
			DemoUpdateFrame();
		}
		if (Constants::PACN_APP()->pTaskFade()->IsEnd())
		{
			m_hDemoPtk->Detach(true);
			KingdomGlobal::PacnRollSys->nRound() = 25u;
			cTaskCharPacman2->SetRetryData();
			kingdom::Constants::Kingdom_SetJobNo(kingdom::job::PACNROLL_GAME);   // Go(0xd300000, 0, 0): the game again at round 25
		}
		break;
	}
	return true;
}

// DOL 0x8011ce18 [audited: 43rd-session pass]
void cTaskCharGolvis04::IjikeStart()
{
	m_bIjikeMsgCome = true;
	if (nLife() < 3)
	{
		m_pGear->nFlag() ^= 1u;
	}
}

// DOL 0x8011ce44 [audited: 43rd-session pass]
Constants::gmSGolvisAttack* cTaskCharGolvis04::GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum)
{
	return cTaskCharGolvis::GetAttackKind(nEatNum, nIjikeLevel, nNum);
}

}
