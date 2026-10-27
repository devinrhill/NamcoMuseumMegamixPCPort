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
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <ReleaseObjectManager.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

const Constants::gmSGolvisAttack* cTaskCharGolvis04::s_AttackTBL[4] = {[]() { Constants::gmSGolvisAttack* _o1 = new Constants::gmSGolvisAttack(); _o1->uiKind = 3; _o1->fRangeMin = 0.0f; _o1->fRangeMax = 6.0f; _o1->iTimeMin = 5; _o1->iTimeMax = 10; return _o1; }(), []() { Constants::gmSGolvisAttack* _o2 = new Constants::gmSGolvisAttack(); _o2->uiKind = 9; _o2->fRangeMin = 3.0f; _o2->fRangeMax = 10.0f; _o2->iTimeMin = 5; _o2->iTimeMax = 10; return _o2; }(), []() { Constants::gmSGolvisAttack* _o3 = new Constants::gmSGolvisAttack(); _o3->uiKind = 8; _o3->fRangeMin = 3.0f; _o3->fRangeMax = 10.0f; _o3->iTimeMin = 5; _o3->iTimeMax = 10; return _o3; }(), []() { Constants::gmSGolvisAttack* _o4 = new Constants::gmSGolvisAttack(); _o4->uiKind = 0; return _o4; }()};
const int cTaskCharGolvis04::scDemoFrameCnts[11] = {30, 354, 384, 420, 635, 710, 743, 783, 820, 950, 990};

cTaskCharGolvis04::cTaskCharGolvis04(cJetTask* pParent)
	: cTaskCharGolvis(pParent), m_hDemoPtk(new cKingdomPtkHnd())
{
	SetName("cTaskCharGolvis04");
	Init();
}

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
	CreateDummyDemoCamera();
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
	std::vector<SkinnedMeshRenderer*> componentsInChildren; // TODO(unity): componentsInChildren = obj.pScnMdl().GetComponentsInChildren<SkinnedMeshRenderer>()
	for (SkinnedMeshRenderer* obj2 : componentsInChildren)
	{
		Bounds localBounds = Bounds(); // TODO(unity): localBounds = obj2.localBounds
		localBounds.Expand(100.0f); // the converter dropped the argument (C#: Expand(100f))
		// TODO(unity): obj2.localBounds = localBounds;
	}
	ConnectChild(m_pDemoPacMan);
}

void cTaskCharGolvis04::Term()
{
	if (m_pDemoPacMan != nullptr)
	{
		m_pDemoPacMan->TermOnce();
	}
	ReleaseObjectManager::Regist(m_dummyDemoCamObj);
	ReleaseObjectManager::Regist(m_dummyDemoCamAimObj);
	cTaskCharGolvis::Term();
}

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

void cTaskCharGolvis04::Setting(bool bRelocate)
{
	cTaskCharGolvis::Setting(bRelocate);
	nPushOutLevel() = 2u;
	m_iState = cTaskCharGolvis04::gmEMonsterStateGolvisGear1::STATE_GOLVISGEAR1_MOVE;
	InitStateFirst();
}

void cTaskCharGolvis04::InitStateFirst()
{
	ChangeToAppear();
	m_bDisp = true;
}

void cTaskCharGolvis04::InitStateStart()
{
	m_iAIMode = 2;
	m_iState = cTaskCharGolvis04::gmEMonsterStateGolvisGear1::STATE_GOLVISGEAR1_MOVE;
}

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

void cTaskCharGolvis04::ChangeToDamage()
{
	vTrans()[1] = vFirstTrans()[1];
	cTaskCharGolvis::ChangeToDamage();
}

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
		Constants::PACN_APP()->pTaskFade()->Fade(EFadeKind::eFADE_TYPE_OUT_BLACK);
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
				// TODO(unity): cTaskCharPacman2.pModel(num).pScnMdl().SetActive(value: false);
			}
			SetAnmState(33u);
			m_nDemoTimer = 0;
			m_pDemoPacMan->pModel()->RegistAnm(0u, "BIN_DEMO_BOS_ANM_PAC");
			Constants::PACN_APP()->pTaskFade()->Fade(EFadeKind::eFADE_TYPE_IN_BLACK);
			vScale()[0] = 1.0f;
			vScale()[1] = vScale()[0];
			vScale()[2] = vScale()[0];
			Constants::PACN_APP()->pTaskCamera()->SetCameraType(4u);
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
		m_pDemoPacMan->UpdateFrame();
		Constants::PACN_APP()->pTaskBgGimmick()->Model->UpdateFrame();
		Constants::PACN_APP()->pTaskBgGimmick(1u)->Model->UpdateFrame();
		::Constants::jetGeoVecUnit(m_pDemoPacMan->vTrans());
		::Constants::jetGeoVecUnit(m_pDemoPacMan->vRotate());
		UpdateDummyDemoCamera();
		if (GetDemoCurrentFrameCnt() >= (float)scDemoFrameCnts[(int)m_demoStep])
		{
			switch (m_demoStep)
			{
			case cTaskCharGolvis04::EDemoStep::Entry:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE099_GOL_TOUJYOU);
				break;
			case cTaskCharGolvis04::EDemoStep::AttackBefore:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE084_BOSS_IJIKE_1);
				break;
			case cTaskCharGolvis04::EDemoStep::Attack:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE080_BOSS_TATAKI);
				break;
			case cTaskCharGolvis04::EDemoStep::Escape:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE101_GOL_IJIKE);
				break;
			case cTaskCharGolvis04::EDemoStep::Jump1:
			case cTaskCharGolvis04::EDemoStep::Jump2:
			case cTaskCharGolvis04::EDemoStep::Jump3:
			case cTaskCharGolvis04::EDemoStep::Jump4:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE031_JUMP);
				break;
			case cTaskCharGolvis04::EDemoStep::Dash:
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE022_DASH);
				break;
			case cTaskCharGolvis04::EDemoStep::FadeOut:
				Constants::PACN_APP()->pTaskFade()->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE);
				ChangeDeathProc(3u);
				break;
			}
			m_demoStep = (EDemoStep)((int)m_demoStep + 1);
		}
		break;
	case 3u:
		GetDemoCurrentFrameCnt();
		if (false)
		{
			UpdateDummyDemoCamera();
		}
		if (false)
		{
			m_pDemoPacMan->UpdateFrame();
			Constants::PACN_APP()->pTaskBgGimmick()->Model->UpdateFrame();
			Constants::PACN_APP()->pTaskBgGimmick(1u)->Model->UpdateFrame();
		}
		if (Constants::PACN_APP()->pTaskFade()->IsEnd())
		{
			m_hDemoPtk->Detach(true);
			KingdomGlobal::PacnRollSys->nRound() = 25u;
			cTaskCharPacman2->SetRetryData();
			kingdom::Constants::Kingdom_SetJobNo(221249536u);
		}
		break;
	}
	return true;
}

void cTaskCharGolvis04::IjikeStart()
{
	m_bIjikeMsgCome = true;
	if (nLife() < 3)
	{
		m_pGear->nFlag() ^= 1u;
	}
}

Constants::gmSGolvisAttack* cTaskCharGolvis04::GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum)
{
	return cTaskCharGolvis::GetAttackKind(nEatNum, nIjikeLevel, nNum);
}

void cTaskCharGolvis04::CreateDummyDemoCamera()
{
	AnimationClip* animationClip = nullptr; // TODO(unity): animationClip = ResourceCache.Get<AnimationClip>("g03_pacnroll/bg/world5/st5/demo/g03d_demo01_a01")
	// TODO(unity): m_dummyDemoCamObj = new GameObject();
	// TODO(unity): m_dummyDemoCamObj.name = "g03d_demo01_a01";
	// TODO(unity): m_dummyDemoCamAnim = m_dummyDemoCamObj.AddComponent<Animation>();
	// TODO(unity): m_dummyDemoCamAnim.AddClip(animationClip, animationClip.name);
	// TODO(unity): m_dummyDemoCamAnim.clip = animationClip;
	// TODO(unity): m_dummyDemoCamAnim.Sample(0f);
	// TODO(unity): m_dummyDemoCamAnim.Stop();
	AnimationClip* animationClip2 = nullptr; // TODO(unity): animationClip2 = ResourceCache.Get<AnimationClip>("g03_pacnroll/bg/world5/st5/demo/g03d_demo01_a01_aim")
	// TODO(unity): m_dummyDemoCamAimObj = new GameObject();
	// TODO(unity): m_dummyDemoCamAimObj.name = "g03d_demo01_a01_aim";
	// TODO(unity): m_dummyDemoCamAimAnim = m_dummyDemoCamAimObj.AddComponent<Animation>();
	// TODO(unity): m_dummyDemoCamAimAnim.AddClip(animationClip2, animationClip2.name);
	// TODO(unity): m_dummyDemoCamAimAnim.clip = animationClip2;
	// TODO(unity): m_dummyDemoCamAimAnim.Sample(0f);
	// TODO(unity): m_dummyDemoCamAimAnim.Stop();
}

void cTaskCharGolvis04::UpdateDummyDemoCamera()
{
	// TODO(unity): m_dummyDemoCamAnim.Progress(Time.deltaTime);
	// TODO(unity): m_dummyDemoCamAimAnim.Progress(Time.deltaTime);
	// TODO(unity): Constants.PACN_APP().pTaskCamera().vPos() = m_dummyDemoCamObj.transform.position.GetUnityPos();
	// TODO(unity): Constants.PACN_APP().pTaskCamera().vTarget() = m_dummyDemoCamAimObj.transform.position;
}

float cTaskCharGolvis04::GetDemoCurrentFrameCnt()
{
	if (false /* TODO(unity): m_dummyDemoCamAnim == null || m_dummyDemoCamAnim.clip == null */)
	{
		return -1.0f;
	}
	// TODO(unity): return m_dummyDemoCamAnim[m_dummyDemoCamAnim.clip.name].time * 60f - 10f;
	return 0.0f;
}

}
