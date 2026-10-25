#include "kingdom/game/g03/cTaskCharGhostBomb.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/cTaskConsoleMan.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

bool cTaskCharGhostBomb::IsWait()
{
	if (m_nProc != 0)
	{
		return false;
	}
	return true;
}

cTaskCharGhostBomb::cTaskCharGhostBomb(uint cursor)
{
	SetName("cTaskCharGhostBomb");
	Init();
	cKingdomModel* cKingdomModel2 = CreateModel();
	if (cKingdomModel2 != nullptr)
	{
		cKingdomModel2->RegistModel("BIN_BOMB_MDL");
		cKingdomModel2->IsDrawShadowDepth(true);
		cKingdomModel2->SetLightSetIdx(1);
		cKingdomModel2->SetFogIdx(0);
		cKingdomModel2->Invisible();
		::Constants::jetGeoVecSet(vTrans(), 0.0f, 0.0f, 0.0f, 0.0f);
		::Constants::jetGeoVecSet(vRotate(), 0.0f, 0.0f, 0.0f, 0.0f);
		::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 0.0f);
	}
	m_nProc = 0u;
	m_nCursorId = cursor;
	SetParam(60u, 5.0f, 10.0f);
	cHitChar* cHitChar2 = new cHitChar(this, 1);
	cHitChar2->SetRange(Constants::PACN_APP()->pGhostCommonParam()->GetGhostBombAttackArea());
	pvecHitChar().push_back(cHitChar2);
}

void cTaskCharGhostBomb::Term()
{
	m_PtkHndBurst->Detach(true);
	cTaskChar::Term();
}

void cTaskCharGhostBomb::Init()
{
	m_PtkHndBurst = new cKingdomPtkHnd();
	m_nProc = 0u;
	m_nSnaipeTime = 0u;
	m_nCursorId = 0u;
	m_nTimer = 0u;
	m_nFlyTime = 0u;
	m_fRadiusMin = 0.0f;
	m_fRadiusMax = 0.0f;
	m_fRadius = 0.0f;
	m_vTarget = Vector4();
	m_vCenter = Vector4();
	m_vStart = Vector4();
	m_vCursor = Vector4();
	m_pGhostTask = nullptr;
}

bool cTaskCharGhostBomb::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (Constants::IS_PAC_PAUSE() || cAppG03PacnRollGame2->IsNakajimaPause(this) || m_nProc == 0)
	{
		return true;
	}
	bool flag = false;
	if (m_pGhostTask != nullptr && m_pGhostTask->IsDead())
	{
		flag = true;
	}
	if (cAppG03PacnRollGame2->pTaskGhostMan()->IsIjike() | flag)
	{
		m_PtkHndBurst->Detach(true);
		Constants::PACN_APP()->pTaskConsole()->BomCursorEnd(m_nCursorId);
		pHitChar(0)->FlagOff(2u);
		pCurrentModel()->Invisible();
		cAppG03PacnRollGame2->pTaskConsole()->BomCursorEnd(m_nCursorId);
		m_nProc = 0u;
	}
	switch (m_nProc)
	{
	case 1u:
		{
			float f = Random::Range(0, 1048576);
			::Constants::jetGeoVecCopy(m_vCursor, cTaskCharPacman2->vTrans());
			m_vCursor[0] += Mathf::Sin(f) * 50.0f;
			m_vCursor[2] += Mathf::Cos(f) * 50.0f;
			cAppG03PacnRollGame2->pTaskConsole()->BomCursorStart(m_nCursorId, m_vCursor);
			m_SndHnd = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE064_AIM_HYOUJI);
			pCurrentModel()->RegistAnm(0u, "BIN_BOMB_ANM_BOSS_FLYING");
			m_nTimer = 0u;
			m_nProc = 2u;
			break;
		}
	case 2u:
		if (m_nSnaipeTime > m_nTimer++)
		{
			float num = ::Constants::jetGeoVecDistance(m_vCursor, cTaskCharPacman2->vTrans());
			float num2 = num / 5.0f;
			if (num * num <= num2 * num2)
			{
				::Constants::jetGeoVecCopy(m_vCursor, cTaskCharPacman2->vTrans());
			}
			else
			{
				Vector4 v;
				::Constants::jetGeoVecSub(v, cTaskCharPacman2->vTrans(), m_vCursor);
				::Constants::jetGeoVecNormalize(v, v);
				::Constants::jetGeoVecScale(v, v, num2);
				::Constants::jetGeoVecAdd(m_vCursor, m_vCursor, v);
			}
			cAppG03PacnRollGame2->pTaskConsole()->BomCursorUpdate(m_nCursorId, m_vCursor);
		}
		else
		{
			Shoot();
			m_SndHnd = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE065_AIM_KOTEI);
			cAppG03PacnRollGame2->pTaskConsole()->BomCursorRed(m_nCursorId);
			m_nTimer = 0u;
			m_nProc = 3u;
		}
		break;
	case 3u:
		{
			Vector4 rotate;
			if (m_nTimer++ < m_nFlyTime)
			{
				float num3 = (float)m_nTimer / (float)m_nFlyTime;
				m_vCenter[1] = m_vTarget[1] * num3 + m_vStart[1] * (1.0f - num3);
				vTrans()[0] = m_vTarget[0] * num3 + m_vStart[0] * (1.0f - num3);
				vTrans()[2] = m_vTarget[2] * num3 + m_vStart[2] * (1.0f - num3);
				vTrans()[1] = Mathf::Sin(::Constants::JET_DEG2RAD(180.0f * num3)) * m_fRadius;
				vTrans()[1] += m_vCenter[1];
			}
			else
			{
				pCurrentModel()->Invisible();
				uint kind = 20u;
				Vector4& trans = vTrans();
				rotate = Vector4();
				cAppG03PacnRollGame2->SetEffect(kind, trans, rotate, m_PtkHndBurst);
				cAppG03PacnRollGame2->pTaskConsole()->BomCursorEnd(m_nCursorId);
				m_SndHnd = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE072_BOSS_BOM);
				m_nProc = 4u;
				m_burstWaitTime = 0.0f;
			}
			float outDist = 0.0f;
			Constants::sFace* _discard2;
			cTaskBgHit* _discard3;
			if (cAppG03PacnRollGame2->pTaskCollision()->CheckBottom(vTrans(), outDist, rotate, _discard2, _discard3) && outDist < 0.2f)
			{
				pCurrentModel()->Invisible();
				cAppG03PacnRollGame2->SetEffect(20u, vTrans(), Vector4(), m_PtkHndBurst);
				cAppG03PacnRollGame2->pTaskConsole()->BomCursorEnd(m_nCursorId);
				m_SndHnd = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE072_BOSS_BOM);
				m_nProc = 4u;
				m_burstWaitTime = 0.0f;
			}
			break;
		}
	case 4u:
		if (m_PtkHndBurst->IsValid() && m_PtkHndBurst->IsActive() && m_burstWaitTime < 0.7f)
		{
			if (::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), pModel(nType())->vTrans()) < 10.0f)
			{
				Constants::PACN_APP()->pTaskCollision()->PushBackChar(this);
				pHitChar(0)->FlagOn(2u);
			}
			m_burstWaitTime += Time::deltaTime;
		}
		else
		{
			pHitChar(0)->FlagOff(2u);
			m_pGhostTask = nullptr;
			m_nProc = 0u;
		}
		break;
	}
	::Constants::jetGeoVecCopy(pCurrentModel()->vTrans(), vTrans());
	::Constants::jetGeoVecCopy(pCurrentModel()->vRotate(), vRotate());
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, vRotate());
	DummyNW4R::SetMtx(pCurrentModel()->pScnMdl(), m);
	return true;
}

void cTaskCharGhostBomb::Draw(int nStatus, cJetTask* pParam)
{
	if (m_nProc != 0)
	{
		cTaskModel::Draw(nStatus, pParam);
		if (!Constants::IS_PAC_PAUSE() && !Constants::PACN_APP()->IsNakajimaPause(this))
		{
			UpdateFrame();
		}
	}
}

void cTaskCharGhostBomb::SetStart(cTaskCharRouteGhost* pParent)
{
	if (m_nProc == 0 && pParent != nullptr)
	{
		m_pGhostTask = pParent;
		m_nProc = 1u;
		m_nSnaipeTime = (uint)KingdomGlobal::KingSys->nMSecToFrame((int)m_pGhostTask->GetSnaipeTime());
	}
}

void cTaskCharGhostBomb::SetParam(uint nFlyTime, float fRadius, float max)
{
	m_nFlyTime = nFlyTime;
	m_fRadiusMin = fRadius;
	m_fRadiusMax = max;
}

void cTaskCharGhostBomb::Shoot()
{
	if (m_pGhostTask != nullptr)
	{
		Constants::PACN_APP()->pTaskCharPacman();
		::Constants::jetGeoVecCopy(m_vStart, m_pGhostTask->GetBomsShootPos());
		::Constants::jetGeoVecCopy(m_vTarget, m_vCursor);
		::Constants::jetGeoVecCopy(vTrans(), m_vStart);
		m_vCenter[0] = (m_vStart[0] + m_vTarget[0]) / 2.0f;
		m_vCenter[1] = m_vStart[1];
		m_vCenter[2] = (m_vStart[2] + m_vTarget[2]) / 2.0f;
		m_vCenter[3] = 1.0f;
		Vector4 v;
		::Constants::jetGeoVecSub(v, m_vStart, m_vTarget);
		m_fRadius = (v[0] * v[0] + v[2] * v[2]) * 0.5f;
		if (m_fRadius < m_fRadiusMin)
		{
			m_fRadius = m_fRadiusMin;
		}
		if (m_fRadius > m_fRadiusMax)
		{
			m_fRadius = m_fRadiusMax;
		}
		pCurrentModel()->Visible();
		m_nTimer = 0u;
	}
}

}
