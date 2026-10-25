#include "kingdom/game/g03/cTaskCharGolvis05.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cTaskCharGear2.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

const float cTaskCharGolvis05::s_fHeight[3] = {-2.0f, 0.0f, 2.0f};
const Constants::gmSGolvisAttack* cTaskCharGolvis05::s_AttackTBL[2] = {[]() { Constants::gmSGolvisAttack* _o1 = new Constants::gmSGolvisAttack(); _o1->uiKind = 6; _o1->fRangeMin = 0.0f; _o1->fRangeMax = 6.0f; _o1->iTimeMin = 5; _o1->iTimeMax = 10; return _o1; }(), []() { Constants::gmSGolvisAttack* _o2 = new Constants::gmSGolvisAttack(); _o2->uiKind = 0; return _o2; }()};
const int cTaskCharGolvis05::m_aaGerasPatern1[4][4] = {{0, 1, 0, 1}, {1, 2, 1, 2}, {2, 0, 2, 0}, {1, 1, 1, 1}};
const int cTaskCharGolvis05::m_aaGerasPatern2[3][4] = {{2, 0, 2, 0}, {0, 2, 0, 2}, {1, 1, 1, 1}};

bool cTaskCharGolvis05::s_CheckLimit(const Vector4& vPos)
{
	Vector4 array[2] = {};
	::Constants::jetGeoVecCopy(array[0], Constants::PACN_APP()->pTaskBgGimmick()->Model->vTrans());
	::Constants::jetGeoVecCopy(array[1], Constants::PACN_APP()->pTaskBgGimmick()->Model->vTrans());
	if (vPos[0] > array[0][0] + 10.0f)
	{
		return false;
	}
	if (vPos[0] < array[0][0] - 10.0f)
	{
		return false;
	}
	if (vPos[2] > ::Constants::JET_MAX(array[0][2] + 10.0f, array[1][2] + 10.0f))
	{
		return false;
	}
	if (vPos[2] < ::Constants::JET_MIN(array[0][2] - 10.0f, array[1][2] - 10.0f))
	{
		return false;
	}
	if (vPos[2] > ::Constants::JET_MAX(array[0][2], array[1][2]) || vPos[2] < ::Constants::JET_MIN(array[0][2], array[1][2]))
	{
		bool flag = false;
		for (int i = 0; i < 2; i++)
		{
			if (100.0f > ::Constants::bizGeoVecDistance2XZ(array[i], vPos))
			{
				flag = true;
			}
			if (flag)
			{
				break;
			}
		}
		if (!flag)
		{
			return false;
		}
	}
	return true;
}

cTaskCharGolvis05::cTaskCharGolvis05(cJetTask* pParent)
	: cTaskCharGolvis(pParent)
{
	SetName("cTaskCharGolvis05");
	Init();
}

bool cTaskCharGolvis05::IsIzike()
{
	return GetBijikeLevel() == 3;
}

void cTaskCharGolvis05::Init()
{
	m_nMoveWait = 0;
	::Constants::jetGeoVecSet(vTrans(), 0.0f, 0.0f, 0.0f, 1.0f);
	m_nGearTimer = 0;
	m_nEscapeMoveTimer = 0;
	m_nEscapeDir = 0;
	Setting(false);
	m_pGear = new cTaskCharGear2(this);
}

void cTaskCharGolvis05::Term()
{
	cTaskCharGolvis::Term();
}

bool cTaskCharGolvis05::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss())
	{
		pHitChar(1)->FlagOff(17u);
		MonsterFrameMove();
	}
	return cTaskCharGolvis::Execute(pParam);
}

void cTaskCharGolvis05::Draw(int nStatus, cJetTask* pParam)
{
	cTaskCharGolvis::Draw(nStatus, pParam);
}

void cTaskCharGolvis05::Setting(bool bRelocate)
{
	cTaskCharGolvis::Setting(bRelocate);
	nPushOutLevel() = 2u;
	m_iState = cTaskCharGolvis05::gmEMonsterStateGolvisGear2::STATE_GOLVISGEAR2_MOVE;
	InitStateFirst();
}

void cTaskCharGolvis05::InitStateFirst()
{
	ChangeToAppear();
	m_bDisp = true;
}

void cTaskCharGolvis05::InitStateStart()
{
	m_iAIMode = 2;
	m_iState = cTaskCharGolvis05::gmEMonsterStateGolvisGear2::STATE_GOLVISGEAR2_MOVE;
}

void cTaskCharGolvis05::MonsterFrameMove_PreProcess()
{
	GearsExecuteFrameMove();
}

Vector4 cTaskCharGolvis05::CalcButtCursorTrans(Vector4 v)
{
	if (!cTaskCharGolvis05::s_CheckLimit(v))
	{
		float value = v[1];
		Vector4 v2 = Vector4::zero;
		Vector4 v3 = Vector4::zero;
		::Constants::jetGeoVecUnit(v2);
		::Constants::jetGeoVecUnit(v3);
		v2[0] = v[0];
		v2[2] = v[2];
		v3[0] = Constants::PACN_APP()->pTaskBgGimmick()->Model->vTrans()[0];
		v3[2] = Constants::PACN_APP()->pTaskBgGimmick()->Model->vTrans()[2];
		::Constants::jetGeoVecSub(v, v2, v3);
		Vector4 v4;
		::Constants::jetGeoVecNormalize(v4, v);
		v4[0] *= 10.0f;
		v4[1] *= 10.0f;
		v4[2] *= 10.0f;
		::Constants::jetGeoVecAdd(v, v4, Constants::PACN_APP()->pTaskBgGimmick()->Model->vTrans());
		v[1] = value;
	}
	return v;
}

void cTaskCharGolvis05::MonsterFrameMove_Free()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	if (m_iState != cTaskCharGolvis05::gmEMonsterStateGolvisGear2::STATE_GOLVISGEAR2_MOVE)
	{
		return;
	}
	if (m_nMoveWait == 0)
	{
		::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, m_fMoveSpeed, 0.01f);
		Vector4 vOut;
		::Constants::bizMathNearTransXZ(vOut, vTrans(), v, m_fMoveSpeedNow);
		::Constants::jetGeoVecSub(vOut, vOut, vTrans());
		if (!m_pGear->LimitLargeArea(vTrans(), vTrans(), vOut))
		{
			m_nMoveWait = 10;
		}
	}
	else
	{
		m_nMoveWait--;
	}
	if (m_iAIMode == 2)
	{
		TryChangeToAttack();
		TryChangeToLaugh();
	}
}

void cTaskCharGolvis05::GearsExecuteFrameMove()
{
	switch (nLife())
	{
	case 3u:
		GearsExecuteLife3();
		break;
	case 1u:
		GearsExecuteLife2();
		break;
	case 2u:
		GearsExecuteLife1();
		break;
	}
}

void cTaskCharGolvis05::MonsterFrameMove_Escape()
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	Vector4 vOut;
	if (m_nEscapeMoveTimer == 0)
	{
		::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, m_fBijikeMoveSpeed, 0.01f);
		::Constants::bizMathNearTransXZ(vOut, vTrans(), v, m_fMoveSpeedNow);
		::Constants::jetGeoVecSub(vOut, vTrans(), vOut);
		if (!m_pGear->LimitLargeArea(vTrans(), vTrans(), vOut))
		{
			m_nEscapeMoveTimer = 30;
			m_nEscapeDir = 0;
		}
	}
	if (m_nEscapeMoveTimer != 0)
	{
		::Constants::bizMathNearValue(m_fMoveSpeedNow, m_fMoveSpeedNow, m_fBijikeMoveSpeed, 0.01f);
		::Constants::bizMathNearTransXZ(vOut, vTrans(), v, m_fMoveSpeedNow);
		::Constants::jetGeoVecSub(vOut, vTrans(), vOut);
		bool flag = false;
		for (int i = 0; i < 90; i++)
		{
			int num = i * 3;
			Vector4 v2;
			switch (m_nEscapeDir)
			{
			case 1:
				::Constants::jetGeoVecRotY(v2, vOut, ::Constants::JET_DEG2RAD(45 + num));
				break;
			case 2:
				::Constants::jetGeoVecRotY(v2, vOut, ::Constants::JET_DEG2RAD(-45 - num));
				break;
			default:
				{
					::Constants::jetGeoVecRotY(v2, vOut, ::Constants::JET_DEG2RAD(45 + num));
					Vector4 v3;
					::Constants::jetGeoVecRotY(v3, vOut, ::Constants::JET_DEG2RAD(-45 - num));
					if (m_pGear->LimitLargeArea(vTrans(), vTrans(), v2))
					{
						m_nEscapeDir = 1;
						flag = true;
					}
					else if (m_pGear->LimitLargeArea(vTrans(), vTrans(), v3))
					{
						m_nEscapeDir = 2;
						flag = true;
					}
					break;
				}
			}
			if (flag || m_pGear->LimitLargeArea(vTrans(), vTrans(), v2))
			{
				break;
			}
		}
		m_nEscapeMoveTimer--;
	}
	if (vTrans()[1] > vFirstTrans()[1])
	{
		vTrans()[1]--;
	}
	else
	{
		vTrans()[1] = vFirstTrans()[1];
	}
}

void cTaskCharGolvis05::ChangeToDamage()
{
	vTrans()[1] = vFirstTrans()[1];
	m_nEscapeMoveTimer = 0;
	m_nEscapeDir = 0;
	cTaskCharGolvis::ChangeToDamage();
}

void cTaskCharGolvis05::GearsExecuteLife3()
{
	int bijikeLevel = GetBijikeLevel();
	CustomKingdomUtil::Assert(bijikeLevel < 4, "error");
	for (uint num = 0u; num < 4; num++)
	{
		m_pGear->ChangeHeight((int)(num + 1), m_aaGerasPatern1[bijikeLevel][num]);
	}
}

void cTaskCharGolvis05::GearsExecuteLife2()
{
	int num = ((m_nGearTimer >= 120) ? 1 : 0);
	if (IsIzike())
	{
		num = 2;
	}
	CustomKingdomUtil::Assert(num < 3, "error");
	for (uint num2 = 0u; num2 < 4; num2++)
	{
		m_pGear->ChangeHeight((int)(num2 + 1), m_aaGerasPatern2[num][num2]);
	}
	m_nGearTimer++;
	m_nGearTimer %= 240;
}

void cTaskCharGolvis05::GearsExecuteLife1()
{
	GearsExecuteLife2();
}

void cTaskCharGolvis05::IjikeStart()
{
	m_bIjikeMsgCome = true;
}

Constants::gmSGolvisAttack* cTaskCharGolvis05::GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum)
{
	return cTaskCharGolvis::GetAttackKind(nEatNum, nIjikeLevel, nNum);
}

bool cTaskCharGolvis05::ChangeToAttack(int iAttackKind)
{
	if (!cTaskCharGolvis05::s_CheckLimit(Constants::PACMAN_OBJ()->vTrans()))
	{
		if (iAttackKind == 10 || iAttackKind == 11 || iAttackKind == 12)
		{
			return false;
		}
		return cTaskCharGolvis::ChangeToAttack(iAttackKind);
	}
	return cTaskCharGolvis::ChangeToAttack(iAttackKind);
}

}
