#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

uint& cTaskGimmickBase::nFlag()
{
	return m_nFlag;
}

uint& cTaskGimmickBase::nTaskKind()
{
	return m_nTaskKind;
}

int& cTaskGimmickBase::nStep()
{
	return m_nStep;
}

uint& cTaskGimmickBase::nMode()
{
	return m_nMode;
}

int& cTaskGimmickBase::nLCounter()
{
	return m_nLCounter;
}

int& cTaskGimmickBase::nComa()
{
	return m_nComa;
}

uint& cTaskGimmickBase::nComaNum()
{
	return m_nComaNum;
}

int& cTaskGimmickBase::nModel()
{
	return m_nModel;
}

uint& cTaskGimmickBase::nMapPos(uint n)
{
	return m_nMapPos[n];
}

Vector4& cTaskGimmickBase::vMTrans()
{
	return m_vMTrans;
}

Vector4& cTaskGimmickBase::vRot()
{
	return m_vRot;
}

int& cTaskGimmickBase::nGmkWidth()
{
	return m_nGmkWidth;
}

int& cTaskGimmickBase::nGmkDepth()
{
	return m_nGmkDepth;
}

uint& cTaskGimmickBase::nWork(int n)
{
	return m_nWork[n];
}

uint& cTaskGimmickBase::nTimer(int n)
{
	return m_nTimer[n];
}

uint& cTaskGimmickBase::nHitChar()
{
	return m_nHitChar;
}

uint& cTaskGimmickBase::nHitCounter()
{
	return m_nHitCounter;
}

bool cTaskGimmickBase::IsFall(Vector4 pMTrans)
{
	return false;
}

bool cTaskGimmickBase::IsAction(cTaskChar* pChar)
{
	return false;
}

bool cTaskGimmickBase::Create(uint no)
{
	return false;
}

cTaskGimmickBase::cTaskGimmickBase()
{
	Init();
}

// DOL 0x8012c1dc [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskGimmickBase::Init()
{
	m_nFlag = 10u;
	m_nTaskKind = 0u;
	m_nStep = 0;
	m_nMode = 0u;
	m_nLCounter = 0;
	m_nMapPos[0] = 0u;
	m_nMapPos[1] = 0u;
	::Constants::jetGeoVecUnit(m_vWTrans);
	::Constants::jetGeoVecUnit(m_vMTrans);
	::Constants::jetGeoVecUnit(m_vRot);
	m_nAnimeKick = 0;
	m_nComaPattern = 0;
	m_nGmkWidth = 1;
	m_nGmkDepth = 1;
	m_nComa = 0;
	m_nComaNum = 0u;
	m_nModel = 0;
	m_nHitCounter = 0u;
	m_nHitChar = 0u;
	for (int i = 0; i < 4; i++)
	{
		m_nTimer[i] = 0u;
	}
}

void cTaskGimmickBase::Term()
{
}

void cTaskGimmickBase::UpdateFrame()
{
	if (!Constants::MOTOS_IS_PAUSE())
	{
		cTaskModel::UpdateFrame();
	}
}

Vector4& cTaskGimmickBase::vWTrans()
{
	CustomKingdomUtil::Assert(pModel() != nullptr, "nullpo");
	return pModel()->vTrans();
}

void cTaskGimmickBase::PresetEnv()
{
}

// DOL 0x8012c310 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskGimmickBase::IsStepOnCntW(Vector4 pWTrans)
{
	float num = KingdomGlobal::g_pMotosGame->pMap()->fPanelSize() / 2.0f;
	if (vWTrans()[0] - num <= pWTrans[0] && vWTrans()[0] + num > pWTrans[0] && vWTrans()[2] - num <= pWTrans[2] && vWTrans()[2] + num > pWTrans[2])
	{
		return true;
	}
	return false;
}

// DOL 0x8012c3fc [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskGimmickBase::IsStepOnCnt(cTaskChar* pChar)
{
	Vector4 vWorld = Vector4();
	KingdomGlobal::g_pMotosGame->pMap()->Map2World(vWorld, pChar->vMTrans());
	return IsStepOnCntW(vWorld);
}

// DOL 0x8012c480 [audited: per AUDIT_G03_G07 row]
void cTaskGimmickBase::Draw(int nStatus, cJetTask* pParam)
{
	uint num = 0u;
	while (true)
	{
		cTaskModel::cPBModel* cPBModel = pPBModel(num);
		if (cPBModel != nullptr)
		{
			Matrix4x4 m;
			::Constants::jetGeoMtxTrans(m, KingdomGlobal::g_pMotosGame->pSlopeMatrix(), cPBModel->vTrans());
			::Constants::jetGeoMtxRotZYX(m, m, cPBModel->vRotate());
			Matrix4x4 _discard1;
			::Constants::jetGeoMtx44to34(_discard1, m);
			CustomKingdomUtil::Assert(cPBModel != nullptr, "nullpo");
			DummyNW4R::SetMtx(cPBModel->pScnMdl(), m);
			DummyNW4R::SetScale(cPBModel->pScnMdl(), cPBModel->vScale()[0], cPBModel->vScale()[1], cPBModel->vScale()[2]);
			cPBModel->UpdateMatrix();   // DOL 0x8012c5c0 [audited: described by the 43rd-session pass]: the push-back (+0xd8) into this frame's scene list (0x8009c3f4)
			num++;
			continue;
		}
		break;
	}
}

// DOL 0x8012c5f0 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskGimmickBase::IsStepOnTrig(cTaskChar* pChar)
{
	Vector4 vWorld = Vector4();
	Vector4 vWorld2 = Vector4();
	KingdomGlobal::g_pMotosGame->pMap()->Map2World(vWorld, pChar->vMTrans());
	KingdomGlobal::g_pMotosGame->pMap()->Map2World(vWorld2, pChar->vMTransOLD());
	if (IsStepOnCntW(vWorld) && (!IsStepOnCntW(vWorld2) || pChar->bTestFlagOLD(384u)))
	{
		return true;
	}
	return false;
}

Vector4& cTaskGimmickBase::vRotate()
{
	CustomKingdomUtil::Assert(pModel() != nullptr, "nullpo");
	return pModel()->vRotate();
}

}
