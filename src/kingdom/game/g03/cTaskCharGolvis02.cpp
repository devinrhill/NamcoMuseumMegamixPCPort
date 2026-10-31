#include "kingdom/game/g03/cTaskCharGolvis02.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include "cJetTask.hpp"
#include "kingdom/game/g03/cGolvisAIArea.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

const Constants::gmSGolvisAttack* cTaskCharGolvis02::s_AttackTBL[4] = {[]() { Constants::gmSGolvisAttack* _o1 = new Constants::gmSGolvisAttack(); _o1->uiKind = 15; _o1->fRangeMin = 0.0f; _o1->fRangeMax = 999.0f; _o1->iTimeMin = 0; _o1->iTimeMax = 1; return _o1; }(), []() { Constants::gmSGolvisAttack* _o2 = new Constants::gmSGolvisAttack(); _o2->uiKind = 3; _o2->fRangeMin = 3.0f; _o2->fRangeMax = 6.0f; _o2->iTimeMin = 5; _o2->iTimeMax = 10; return _o2; }(), []() { Constants::gmSGolvisAttack* _o3 = new Constants::gmSGolvisAttack(); _o3->uiKind = 9; _o3->fRangeMin = 3.0f; _o3->fRangeMax = 6.0f; _o3->iTimeMin = 5; _o3->iTimeMax = 10; return _o3; }(), []() { Constants::gmSGolvisAttack* _o4 = new Constants::gmSGolvisAttack(); _o4->uiKind = 0; return _o4; }()};

cTaskCharGolvis02::cTaskCharGolvis02(cJetTask* pParent)
	: cTaskCharGolvis(pParent)
{
	SetName("cTaskCharGolvis02");
	Init();
}

// DOL 0x8011ae74 [audited]
void cTaskCharGolvis02::Init()
{
	m_pAI = new cGolvisAIArea(this, KingdomGlobal::PacnRollSys->nRound());
	::Constants::jetGeoVecSet(vTrans(), 0.0f, 2.0f, -16.0f, 1.0f);
	Setting(false);
}

// DOL 0x8011af10 [audited]
void cTaskCharGolvis02::Term()
{
	// DOL: delete the AI (vtable dtor, flag 1); the base Term runs from the dtor chain (cTaskCharGolvis::ReleaseTerm)
	if (m_pAI != nullptr)
	{
		delete m_pAI;
		m_pAI = nullptr;
	}
}

// DOL 0x8011af68 [audited: same constants as Golvis01 0x8011a10c]
bool cTaskCharGolvis02::IsNearAttack()
{
	float num = ::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(0) - vTrans(0));
	float num2 = ::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(1) - vTrans(1));
	float num3 = ::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(2) - vTrans(2));
	if ((num < 0.2f && num3 < 0.2f) || (num < 4.0f && num3 < 4.0f && num2 > 1.3f))
	{
		return true;
	}
	return false;
}

// DOL 0x8011b050 [audited]
bool cTaskCharGolvis02::IsForceAttack()
{
	return IsNearAttack();
}

// DOL 0x8011b060 [audited]
bool cTaskCharGolvis02::ChangeToAttack(int iAttackKind)
{
	::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(0) - vTrans(0));
	::Constants::fabs(Constants::PACMAN_OBJ()->vTrans(2) - vTrans(2));
	if (IsNearAttack())
	{
		return cTaskCharGolvis::ChangeToAttack(6);
	}
	return cTaskCharGolvis::ChangeToAttack(iAttackKind);
}

// DOL 0x8011b0d4 [audited]
bool cTaskCharGolvis02::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss())
	{
		pHitChar(1)->FlagOff(17u);
		FallProcess();
		MonsterFrameMove();
	}
	return cTaskCharGolvis::Execute(pParam);
}

// DOL 0x8011b190 [audited]
void cTaskCharGolvis02::Draw(int nStatus, cJetTask* pParam)
{
	cTaskCharGolvis::Draw(nStatus, pParam);
}

// DOL 0x8011b194 [audited]
void cTaskCharGolvis02::Setting(bool bRelocate)
{
	cTaskCharGolvis::Setting(bRelocate);
	vRotate()[1] = 0.0f;
	m_iState = cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_RANDOM;
	InitStateFirst();
}

// DOL 0x8011b1e4 [audited]
void cTaskCharGolvis02::InitStateFirst()
{
	ChangeToAppear();
	m_bDisp = true;
	vRotate()[1] += ::Constants::JET_DEG2RAD(90.0f);
}

// DOL 0x8011b234 [audited]
void cTaskCharGolvis02::InitStateStart()
{
	m_iAIMode = 2;
	m_iState = cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_PURSUIT;
}

// DOL 0x8011b248 [audited]
void cTaskCharGolvis02::MonsterFrameMove_PreProcess()
{
	if (vTrans()[1] < 0.0f)
	{
		vTrans()[1] = 0.0f;
	}
}

// DOL 0x8011b260 [audited: state switch, SetKind(2|3, 0, 180), speeds vs decomp]
void cTaskCharGolvis02::MonsterFrameMove_Free()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	if (m_iState != cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_JUMP)
	{
		if (nType() != 2)
		{
			if (m_iState == cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_ESCAPE)
			{
				m_iState = cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_PURSUIT;
			}
		}
		else if (m_iState == cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_PURSUIT)
		{
			m_iState = cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_ESCAPE;
		}
	}
	switch (m_iState)
	{
	case cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_RANDOM:
	case cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_PURSUIT:
		m_bEnableGravity = true;
		m_pAI->SetKind(2, 0);
		if (m_iAIMode == 2 && (TryChangeToAttack() || TryChangeToLaugh()))
		{
			return;
		}
		break;
	case cTaskCharGolvis02::gmEMonsterStateGolvisArea::STATE_GOLVISAREA_ESCAPE:
		m_bEnableGravity = true;
		m_pAI->SetKind(3, 0);
		break;
	}
	cTaskCharGolvis02::gmEMonsterStateGolvisArea iState = m_iState;
	if ((uint)iState > 2u || HitCheckPacman())
	{
		return;
	}
	float fTarget = m_fMoveSpeed;
	if (nType() == 2)
	{
		fTarget = m_fBijikeMoveSpeed;
	}
	::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, fTarget, 0.01f);
	m_pAI->Think(Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, m_pAI->vGoalPos());
	if (nType() == 2)
	{
		Vector4 v3;
		::Constants::jetGeoVecSub(v3, v2, vTrans());
		Vector4 outCross;
		Constants::sFace* outFace;
		cTaskBgHit* _discard6;
		if (Constants::PACN_APP()->pTaskCollision()->CheckVectorSimple(0u, vTrans(), v3, outCross, outFace, _discard6))
		{
			::Constants::jetGeoVecScale(v3, outFace->vNorm, 0.5f);
			::Constants::jetGeoVecAdd(v2, outCross, v3);
		}
	}
	if (::Constants::bizMathNearTransXZ(vTrans(), vTrans(), v2, m_fMoveSpeedNow))
	{
		float pOut;
		::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(9.0f));
		vRotate()[1] = pOut;
	}
}

// DOL 0x8011b528 [audited]
void cTaskCharGolvis02::MonsterFrameMove_Escape()
{
	MonsterFrameMove_Free();
}

// DOL 0x8011b538 [audited]
Constants::gmSGolvisAttack* cTaskCharGolvis02::GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum)
{
	return cTaskCharGolvis::GetAttackKind(nEatNum, nIjikeLevel, nNum);
}

}
