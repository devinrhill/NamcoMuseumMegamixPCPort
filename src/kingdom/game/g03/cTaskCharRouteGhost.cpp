#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"
#include "DummyNW4R.hpp"

#include <iterator>
#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharEnemy.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskGhostRoute.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g03
{

string cTaskCharRouteGhost::GetGhostModelKey()
{
	return ((m_nGhostRider == 1u) ? "BIN_GOS_CLYD_MDL" : ((m_nGhostRider == 2u) ? "BIN_GOS_PINK_MDL" : ((m_nGhostRider == 3u) ? "BIN_GOS_BLIN_MDL" : ((m_nGhostRider == 4u) ? "BIN_GOS_INKY_MDL" : ("BIN_GOS_CLYD_MDL")))));
}

string cTaskCharRouteGhost::GetGhostAnmKey()
{
	return ((m_nGhostRider == 1u) ? "BIN_GOS_CLYD_CHA" : ((m_nGhostRider == 2u) ? "BIN_GOS_PINK_CHA" : ((m_nGhostRider == 3u) ? "BIN_GOS_BLIN_CHA" : ((m_nGhostRider == 4u) ? "BIN_GOS_INKY_CHA" : ("BIN_GOS_CLYD_CHA")))));
}

string cTaskCharRouteGhost::GetGhostSRTAnmKey()
{
	return ((m_nGhostRider == 1u) ? "BIN_GOS_CLYD_TSA" : ((m_nGhostRider == 2u) ? "BIN_GOS_PINK_TSA" : ((m_nGhostRider == 3u) ? "BIN_GOS_BLIN_TSA" : ((m_nGhostRider == 4u) ? "BIN_GOS_INKY_TSA" : ("BIN_GOS_CLYD_TSA")))));
}

void cTaskCharRouteGhost::SetTargetSpeed(int percent)
{
	m_fRouteMaxSpeed = m_fMaxSpeed * ((float)percent / 100.0f);
}

uint& cTaskCharRouteGhost::nGhostRider()
{
	return m_nGhostRider;
}

int& cTaskCharRouteGhost::nTargetPointID()
{
	return m_nTargetPointID;
}

Vector4& cTaskCharRouteGhost::vMoveTarget()
{
	return m_vMoveTarget;
}

Vector4& cTaskCharRouteGhost::vFootPos()
{
	return m_vFootPos;
}

Vector4& cTaskCharRouteGhost::GetBomsShootPos()
{
	return vTrans();
}

uint& cTaskCharRouteGhost::nSndType()
{
	return m_nSndType;
}

cKingdomPtkHnd* cTaskCharRouteGhost::pPtkHndEffect(int num)
{
	if (num >= 0 && num < 2)
	{
		return m_PtkHndEffect[num];
	}
	return nullptr;
}

uint cTaskCharRouteGhost::GetSnaipeTime()
{
	return ((m_nBombStatus == 0) ? 0u : ((m_nBombStatus == 1) ? 1000u : ((m_nBombStatus == 2) ? 1500u : (0u))));
}

cTaskChar::eKIND cTaskCharRouteGhost::GetKind()
{
	return cTaskChar::eKIND::eKIND_PACMAN;
}

string cTaskCharRouteGhost::GetModelKey()
{
	return string();
}

string cTaskCharRouteGhost::GetAnmKey()
{
	return string();
}

string cTaskCharRouteGhost::GetNodeKey0()
{
	return string();
}

string cTaskCharRouteGhost::GetNodeKey1()
{
	return string();
}

uint cTaskCharRouteGhost::GetEffectType0()
{
	return 0u;
}

uint cTaskCharRouteGhost::GetEffectType1()
{
	return 0u;
}

bool cTaskCharRouteGhost::IsDead()
{
	if (nStat() <= 5)
	{
		return false;
	}
	return true;
}

void cTaskCharRouteGhost::SetDrawShadow(bool draw)
{
	ShadowCastingMode shadowCastingMode = {};
	// FIXME(cs2cpp): unresolved identifier ShadowCastingMode
	// TODO(unity): ShadowCastingMode shadowCastingMode = (draw ? ShadowCastingMode.On : ShadowCastingMode.Off);
	std::vector<Renderer*> componentsInChildren; // TODO(unity): componentsInChildren = pPBModel().pScnMdl().GetComponentsInChildren<Renderer>()
	for (int i = 0; i < (int)componentsInChildren.size(); i++)
	{
		// TODO(unity): componentsInChildren[i].shadowCastingMode = shadowCastingMode;
	}
	// TODO(unity): componentsInChildren = pPBModel(1u).pScnMdl().GetComponentsInChildren<Renderer>();
	for (int j = 0; j < (int)componentsInChildren.size(); j++)
	{
		// TODO(unity): componentsInChildren[j].shadowCastingMode = shadowCastingMode;
	}
}

cTaskCharRouteGhost::cTaskCharRouteGhost()
{
	SetName("cTaskCharRouteGhost");
	Init();
}

void cTaskCharRouteGhost::Init()
{
	for (int i = 0; i < (int)std::size(m_PtkHndEffect); i++)
	{
		m_PtkHndEffect[i] = new cKingdomPtkHnd();
	}
	nStat() = 1;
	nOptionItem() = 0;
	m_nGhostRider = 1u;
	m_nCount = 0;
	m_bMove = false;
	m_fSpeed = 0.0f;
	m_fRouteMaxSpeed = 0.0f;
	m_fTargetSpeed = 0.0f;
	m_nTargetPointID = 0;
	m_vMoveTarget = Vector4();
	m_fEatSpeed = 0.0f;
	m_nBombStatus = 0;
	m_nBombCounter = 0;
	m_fMaxSpeed = 0.0f;
	m_fAddSpeed = 0.0f;
	m_fTurnSpeed = 0.0f;
	m_fIzikeSpeed = 0.0f;
	m_nIzikeFlashTime = 0;
	m_nSndType = 0u;
	m_fFootDist = 0.0f;
	m_vFootPos = Vector4();
	::Constants::jetGeoVecZero(m_vEffectNode[0]);
	::Constants::jetGeoVecZero(m_vEffectNode[1]);
	m_bPackAttack = false;
	SetState(0u);
}

void cTaskCharRouteGhost::Term()
{
	m_PtkHndEffect[0]->Detach(true);
	m_PtkHndEffect[1]->Detach(true);
	cTaskCharGhost::Term();
}

bool cTaskCharRouteGhost::Execute(cJetTask* pParam)
{
	InGamePadManager::GetPad1No();
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	cTaskGhostRoute* cTaskGhostRoute2 = static_cast<cTaskGhostRoute*>(pParent());
	::Constants::jetGeoVecCopy(vTransOld(), vTrans());
	if (Constants::IS_PAC_PAUSE() || cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		return true;
	}
	if (cAppG03PacnRollGame2->IsGame())
	{
		if (!m_bMove)
		{
			if (GetKind() == cTaskChar::eKIND::eKIND_RIDE_PLANE)
			{
				if (cAppG03PacnRollGame2->pTaskGhostMan()->GetGondolaAnim() != 0.0f)
				{
					m_bMove = true;
				}
			}
			else if (GetKind() == cTaskChar::eKIND::eKIND_RIDE_HOVER)
			{
				vRotate()[1] = ::Constants::JET_DEG2RAD(180.0f);
				if (Mathf::Abs(cTaskCharPacman2->vTrans()[1] - vTrans()[1]) < 5.0f)
				{
					Vector4 array[2] = {};
					::Constants::jetGeoVecSet(array[0], cTaskCharPacman2->vTrans()[0] - vTrans()[0], 0.0f, cTaskCharPacman2->vTrans()[2] - vTrans()[2], 0.0f);
					::Constants::jetGeoVecSet(array[1], vTrans()[0] - vTrans()[0], 0.0f, vTrans()[2] - 1.0f - vTrans()[2], 0.0f);
					float x = array[0][0] * array[1][0] + array[0][2] * array[1][2];
					float y = array[0][0] * array[1][2] - array[0][2] * array[1][0];
					if (Mathf::Abs(57.29578f * Mathf::Atan2(y, x)) < 90.0f)
					{
						m_bMove = true;
					}
				}
			}
			else
			{
				m_bMove = true;
			}
		}
		if (nStat() < 7)
		{
			if (nStat() < 5 && ::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), pModel(nType())->vTrans()) < 30.0f)
			{
				Constants::PACN_APP()->pTaskCollision()->PushBackChar(this);
			}
			if (m_nBombStatus != 0)
			{
				if (m_nBombCounter > 0)
				{
					m_nBombCounter--;
				}
				if (!Constants::PACN_APP()->pTaskGhostMan()->IsIjike() && !IsDead() && ::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), pModel(nType())->vTrans()) < 20.0f && m_nBombCounter <= 0)
				{
					cAppG03PacnRollGame2->pTaskGhostMan()->ShotBomb(this);
					m_nBombCounter = 180;
				}
			}
			CheckStatus();
		}
		switch (nStat())
		{
		case 1:
			m_fTargetSpeed = m_fRouteMaxSpeed;
			Move();
			if (vMoveTarget()[0] == vTrans()[0] && vMoveTarget()[2] == vTrans()[2])
			{
				if (GetKind() == cTaskChar::eKIND::eKIND_RIDE_PLANE && m_nTargetPointID == 0)
				{
					m_bMove = false;
				}
				if (m_nTargetPointID++ >= cTaskGhostRoute2->nRouteNum())
				{
					m_nTargetPointID = 0;
				}
			}
			break;
		case 3:
			if (nKind() >= cTaskChar::eKIND::eKIND_RIDE_CAR)
			{
				if (nState() != 1)
				{
					SetState(1u);
					SetType(1u);
					SetAnmState(2u);
				}
				nStat() = 4;
				break;
			}
			if (nState() != 1)
			{
				SetState(1u);
				SetType(1u);
				SetAnmState(1u);
			}
			if (pModel(nType())->GetFrameMaxAnmChr(0u) < pModel(nType())->GetFrameAnmChr(0u))
			{
				SetAnmState(2u);
				nStat() = 4;
			}
			break;
		case 4:
			m_fTargetSpeed = m_fRouteMaxSpeed * m_fIzikeSpeed;
			Move();
			if (vMoveTarget()[0] == vTrans()[0] && vMoveTarget()[2] == vTrans()[2] && m_nTargetPointID++ >= cTaskGhostRoute2->nRouteNum())
			{
				m_nTargetPointID = 0;
			}
			if (cAppG03PacnRollGame2->IjikeGetCount() < m_nIzikeFlashTime)
			{
				if (cAppG03PacnRollGame2->IjikeGetCount() % 30 < 15)
				{
					SetType(1u);
				}
				else
				{
					SetType(0u);
				}
			}
			else if (nType() != 1)
			{
				SetType(1u);
			}
			break;
		case 5:
			{
				pPBModel(2u)->Invisible();
				m_PtkHndEffect[0]->Detach(true);
				m_PtkHndEffect[1]->Detach(true);
				m_nCount++;
				if ((float)m_nCount < 25.0f)
				{
					vScale()[0] = (25.0f - (float)m_nCount) / 25.0f;
					if (vScale()[0] < 0.01f)
					{
						vScale()[0] = 0.01f;
					}
					vScale()[1] = vScale()[0];
					vScale()[2] = vScale()[0];
					vScale()[3] = vScale()[0];
				}
				m_fEatSpeed += 0.02f;
				float f = Mathf::Atan2(cTaskCharPacman2->vTrans()[0] - vTrans()[0], cTaskCharPacman2->vTrans()[2] - vTrans()[2]);
				float num = Mathf::Sin(f);
				float num2 = Mathf::Cos(f);
				vTrans()[0] += num * m_fEatSpeed;
				vTrans()[2] += num2 * m_fEatSpeed;
				if (num < 0.0f)
				{
					if (cTaskCharPacman2->vTrans()[0] > vTrans()[0])
					{
						vTrans()[0] = cTaskCharPacman2->vTrans()[0];
					}
				}
				else if (cTaskCharPacman2->vTrans()[0] < vTrans()[0])
				{
					vTrans()[0] = cTaskCharPacman2->vTrans()[0];
				}
				if (num2 < 0.0f)
				{
					if (cTaskCharPacman2->vTrans()[2] > vTrans()[2])
					{
						vTrans()[2] = cTaskCharPacman2->vTrans()[2];
					}
				}
				else if (cTaskCharPacman2->vTrans()[2] < vTrans()[2])
				{
					vTrans()[2] = cTaskCharPacman2->vTrans()[2];
				}
				if (cTaskCharPacman2->vTrans()[0] == vTrans()[0] && cTaskCharPacman2->vTrans()[2] == vTrans()[2])
				{
					if (pPBModel()->pExpandMdl(0u) != nullptr)
					{
						if (pPBModel()->pExpandMdl(0u) != nullptr)
						{
							pPBModel()->pExpandMdl(0u)->Invisible();
						}
					}
					if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
					{
						if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
						{
							pPBModel(1u)->pExpandMdl(0u)->Invisible();
						}
					}
					nStat() = 6;
				}
				break;
			}
		case 6:
			Constants::PACN_APP()->NakajimaPlay(this);
			pPBModel()->Invisible();
			pPBModel(1u)->Invisible();
			pPBModel(2u)->Invisible();
			if (pPBModel()->pExpandMdl(0u) != nullptr)
			{
				if (pPBModel()->pExpandMdl(0u) != nullptr)
				{
					pPBModel()->pExpandMdl(0u)->Invisible();
				}
			}
			if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
			{
				if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
				{
					pPBModel(1u)->pExpandMdl(0u)->Invisible();
				}
			}
			m_PtkHndEffect[0]->Detach(true);
			m_PtkHndEffect[1]->Detach(true);
			nStat() = 9;
			break;
		case 7:
			m_nCount = 0;
			m_bPackAttack = false;
			m_fEatSpeed = 0.0f;
			m_fSpeed = 0.0f;
			m_fTargetSpeed = m_fMaxSpeed;
			m_nTargetPointID = 0;
			m_bSndEngineStop = false;
			m_nBombCounter = 0;
			m_vMoveTarget = Vector4();
			::Constants::jetGeoVecZero(vRotate());
			::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
			pPBModel(2u)->Visible();
			if (!cAppG03PacnRollGame2->pTaskGhostMan()->IsIjike())
			{
				pPBModel()->Visible();
				if (pPBModel()->pExpandMdl(0u) != nullptr)
				{
					if (pPBModel()->pExpandMdl(0u) != nullptr)
					{
						pPBModel()->pExpandMdl(0u)->Visible();
					}
				}
				SetType(0u);
				SetState(0u);
			}
			else
			{
				pPBModel(1u)->Visible();
				if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
				{
					if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
					{
						pPBModel(1u)->pExpandMdl(0u)->Visible();
					}
				}
				SetType(1u);
				SetState(1u);
			}
			pHitChar(0)->FlagOn(2u);
			::Constants::jetGeoVecCopy(vTrans(), vStartPos());
			vTrans()[1] += 20.0f;
			nStat() = 8;
			break;
		case 8:
			vTrans()[1] -= 0.5f;
			if (vStartPos()[1] >= vTrans()[1])
			{
				vTrans()[1] = vStartPos()[1];
				nStat() = 1;
			}
			if (!cAppG03PacnRollGame2->pTaskGhostMan()->IsIjike())
			{
				break;
			}
			if (cAppG03PacnRollGame2->IjikeGetCount() < m_nIzikeFlashTime)
			{
				if (cAppG03PacnRollGame2->IjikeGetCount() % 30 < 15)
				{
					SetType(1u);
				}
				else
				{
					SetType(0u);
				}
			}
			else if (nType() != 1)
			{
				SetType(1u);
			}
			break;
		case 9:
			if (bRevival())
			{
				nStat() = 7;
				bRevival() = false;
			}
			break;
		}
	}
	if (nKind() == cTaskChar::eKIND::eKIND_RIDE_HOVER)
	{
		float num3 = vRotate()[1];
		if (num3 > (float)std::numbers::pi)
		{
			float num4 = num3 - (float)std::numbers::pi;
			num3 *= -1.0f;
			num3 += num4 * 2.0f;
		}
		float num5 = Mathf::Abs(num3);
		vRotate()[0] = num5 / (float)std::numbers::pi * 0.508966f - 0.254483f;
	}
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vTrans());
	if (cTaskGhostRoute2->IsNeedBottomCheck() && GetKind() != cTaskChar::eKIND::eKIND_RIDE_PLANE && GetKind() != cTaskChar::eKIND::eKIND_RIDE_UFO && GetKind() != cTaskChar::eKIND::eKIND_RIDE_BOAT)
	{
		v[1] += 5.0f;
		float _discard2;
		Constants::sFace* _discard3;
		cTaskBgHit* _discard4;
		if (!cAppG03PacnRollGame2->pTaskCollision()->CheckBottom(v, _discard2, v, _discard3, _discard4))
		{
			v[1] -= 5.0f;
		}
	}
	::Constants::jetGeoVecCopy(pPBModel(2u)->vTrans(), v);
	::Constants::jetGeoVecCopy(vTrans(), v);
	if (nKind() >= cTaskChar::eKIND::eKIND_RIDE_CAR)
	{
		if (nKind() == cTaskChar::eKIND::eKIND_RIDE_HOVER || nKind() == cTaskChar::eKIND::eKIND_RIDE_PLANE || nKind() == cTaskChar::eKIND::eKIND_RIDE_BOAT)
		{
			Vector4 v2;
			::Constants::jetGeoVecCopy(v2, m_vFootPos);
			v2[1] += m_fFootDist;
			Matrix4x4 m;
			::Constants::jetGeoMtxIdentity(m);
			::Constants::jetGeoMtxRotY(m, m, vRotate()[1]);
			if (nKind() == cTaskChar::eKIND::eKIND_RIDE_HOVER)
			{
				::Constants::jetGeoMtxRotX(m, m, vRotate()[0]);
			}
			::Constants::jetGeoVecTrans(v2, m, v2);
			::Constants::jetGeoVecAdd(v, v2, v);
		}
		else
		{
			::Constants::jetGeoVecAdd(v, m_vFootPos, v);
			v[1] += m_fFootDist;
		}
	}
	else
	{
		v[1] += m_fFootDist;
	}
	::Constants::jetGeoVecCopy(pPBModel()->vTrans(), v);
	::Constants::jetGeoVecCopy(pPBModel(1u)->vTrans(), v);
	::Constants::jetGeoVecCopy(pPBModel()->vScale(), vScale());
	::Constants::jetGeoVecCopy(pPBModel(1u)->vScale(), vScale());
	::Constants::jetGeoVecCopy(pPBModel(2u)->vScale(), vScale());
	::Constants::jetGeoVecCopy(pPBModel()->vRotate(), vRotate());
	::Constants::jetGeoVecCopy(pPBModel(1u)->vRotate(), vRotate());
	::Constants::jetGeoVecCopy(pPBModel(2u)->vRotate(), vRotate());
	if (nStat() < 9)
	{
		Matrix4x4 m2;
		::Constants::jetGeoMtxIdentity(m2);
		::Constants::jetGeoMtxRotY(m2, m2, pPBModel(2u)->vRotate()[1]);
		if (nKind() == cTaskChar::eKIND::eKIND_RIDE_HOVER)
		{
			::Constants::jetGeoMtxRotX(m2, m2, pPBModel(2u)->vRotate()[0]);
		}
		Vector4 v3;
		if (GetEffectType0() != 38)
		{
			::Constants::jetGeoVecTrans(v3, m2, m_vEffectNode[0]);
			::Constants::jetGeoVecAdd(v3, v3, vTrans());
			if (!m_PtkHndEffect[0]->IsValid() || !m_PtkHndEffect[0]->IsActive())
			{
				Constants::PACN_APP()->SetEffect(GetEffectType0(), v3, Vector4(), pPtkHndEffect(0));
			}
			m_PtkHndEffect[0]->vSetTrans(v3);
			if (GetEffectType0() == 26)
			{
				Vector4 v4 = Vector4::zero;
				::Constants::jetGeoVecSet(v4, 0.0f - pPBModel(2u)->vRotate()[0], pPBModel(2u)->vRotate()[1] - (float)std::numbers::pi, 0.0f, 1.0f);
				m_PtkHndEffect[0]->vSetRotate(v4);
			}
		}
		if (GetEffectType1() != 38)
		{
			::Constants::jetGeoVecTrans(v3, m2, m_vEffectNode[1]);
			::Constants::jetGeoVecAdd(v3, v3, pPBModel(2u)->vTrans());
			if (!m_PtkHndEffect[1]->IsValid() || !m_PtkHndEffect[1]->IsActive())
			{
				Constants::PACN_APP()->SetEffect(GetEffectType1(), v3, Vector4(), pPtkHndEffect(1));
			}
			m_PtkHndEffect[1]->vSetTrans(v3);
			if (GetEffectType1() == 26)
			{
				Vector4 v5 = Vector4::zero;
				::Constants::jetGeoVecSet(v5, 0.0f - pPBModel(2u)->vRotate()[0], pPBModel(2u)->vRotate()[1] - (float)std::numbers::pi, 0.0f, 1.0f);
				m_PtkHndEffect[1]->vSetRotate(v5);
			}
		}
	}
	if (GetKind() == cTaskChar::eKIND::eKIND_RIDE_UFO)
	{
		pHitChar(0)->SetOfsPos(0.0f, 1.5f, 0.0f);
	}
	else if (GetKind() == cTaskChar::eKIND::eKIND_RIDE_BOAT)
	{
		pHitChar(0)->SetOfsPos(0.0f, 1.0f, 0.0f);
	}
	if (nStat() < 9 && !m_bSndEngineStop && Constants::PACN_APP()->pTaskConsRetry() != nullptr && !Constants::PACN_APP()->pTaskConsRetry()->IsVisible() && nullptr == m_SndHndEngine)
	{
		switch (m_nSndType)
		{
		case 1u:
			m_SndHndEngine = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE044_ENGINE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eNONE, SoundSeObject::ESeGroup::eNORMAL, pPBModel(2u)->pScnMdl(), Constants::scAudioMinDistance, Constants::scAudioMaxDistance);
			break;
		case 2u:
			m_SndHndEngine = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE045_ENGINE2, 5.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eNONE, SoundSeObject::ESeGroup::eNORMAL, pPBModel(2u)->pScnMdl(), Constants::scAudioMinDistance, Constants::scAudioMaxDistance);
			break;
		}
	}
	return cTaskCharEnemy::Execute(pParam);
}

void cTaskCharRouteGhost::Draw(int nStatus, cJetTask* pParam)
{
	uint num = nType();
	nType() = 2u;
	Matrix4x4 m;
	Matrix4x4 m2;
	if (nKind() >= cTaskChar::eKIND::eKIND_RIDE_CAR)
	{
		::Constants::jetGeoMtxIdentity(m);
		::Constants::jetGeoMtxTrans(m, m, pCurrentModel()->vTrans());
		if (nKind() != cTaskChar::eKIND::eKIND_RIDE_UFO)
		{
			::Constants::jetGeoMtxRotZYX(m, m, pCurrentModel()->vRotate());
		}
		::Constants::jetGeoMtx44to34(m2, m);
		DummyNW4R::SetMtx(pCurrentModel()->pScnMdl(), m);
		DummyNW4R::SetScale(pCurrentModel()->pScnMdl(), pCurrentModel()->vScale()[0], pCurrentModel()->vScale()[1], pCurrentModel()->vScale()[2]);
		if (nStat() != 9 && !pPBModel(2u)->IsVisible())
		{
			pPBModel(2u)->Visible();
		}
	}
	else if (nStat() != 9 && pPBModel(2u)->IsVisible())
	{
		pPBModel(2u)->Invisible();
	}
	nType() = num;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, pCurrentModel()->vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, pCurrentModel()->vRotate());
	::Constants::jetGeoMtx44to34(m2, m);
	DummyNW4R::SetMtx(pCurrentModel()->pScnMdl(), m);
	DummyNW4R::SetScale(pCurrentModel()->pScnMdl(), pCurrentModel()->vScale()[0], pCurrentModel()->vScale()[1], pCurrentModel()->vScale()[2]);
	if (nStat() != 9)
	{
		if (nType() == 0)
		{
			if (!pPBModel()->IsVisible())
			{
				pPBModel()->Visible();
			}
			if (pPBModel(1u)->IsVisible())
			{
				pPBModel(1u)->Invisible();
			}
		}
		else
		{
			if (pPBModel()->IsVisible())
			{
				pPBModel()->Invisible();
			}
			if (!pPBModel(1u)->IsVisible())
			{
				pPBModel(1u)->Visible();
			}
		}
	}
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACN_APP()->IsNakajimaPause(this))
	{
		UpdateFrame();
	}
}

void cTaskCharRouteGhost::SetType(const uint& type)
{
	CustomKingdomUtil::Assert(3 > type, "error");
	pCurrentModel()->Invisible();
	if (pCurrentModel()->pExpandMdl(0u) != nullptr)
	{
		if (pCurrentModel()->pExpandMdl(0u) != nullptr)
		{
			pCurrentModel()->pExpandMdl(0u)->Invisible();
		}
	}
	nType() = type;
	pCurrentModel()->Visible();
	if (pCurrentModel()->pExpandMdl(0u) != nullptr)
	{
		if (pCurrentModel()->pExpandMdl(0u) != nullptr)
		{
			pCurrentModel()->pExpandMdl(0u)->Visible();
		}
	}
	pPBModel(2u)->Visible();
}

void cTaskCharRouteGhost::SetAnmState(const uint& state)
{
	switch (state)
	{
	case 0u:
		SetAnm(GetGhostAnmKey());
		SetAnm(GetGhostSRTAnmKey());
		break;
	case 1u:
		SetAnm("BIN_GOS_IZIK01_CHA");
		SetAnm("BIN_GOS_IZIK_TSA");
		break;
	case 2u:
		SetAnm("BIN_GOS_IZIK00_CHA");
		SetAnm("BIN_GOS_IZIK_TSA");
		break;
	}
}

void cTaskCharRouteGhost::SetAnm(string anime, uint idx)
{
	AnimationClip* animationClip = dynamic_cast<AnimationClip*>(ResourceDatabase::Get<Object>(anime));
	CustomKingdomUtil::Assert(nullptr != animationClip, "nullpo");
	if (!CheckRepetition(animationClip, idx))
	{
		pModel(nType())->RegistAnm(idx, animationClip);
	}
}

void cTaskCharRouteGhost::SetModel()
{
	nKind() = GetKind();
	cKingdomModel* obj = CreateModel();
	obj->RegistModel(GetGhostModelKey());
	obj->IsDrawShadowDepth(false);
	obj->SetLightSetIdx(2);
	obj->SetFogIdx(2);
	Vector4 v = Vector4();
	obj->GetLocalNodeTrans(v, "asi");
	m_fFootDist = 0.0f - v[1];
	cKingdomModel* obj2 = CreateModel(1u);
	obj2->RegistModel("BIN_GOS_IZIK_MDL");
	obj2->IsDrawShadowDepth(false);
	obj2->SetLightSetIdx(2);
	obj2->SetFogIdx(2);
	obj2->Invisible();
	cKingdomModel* obj3 = CreateModel(2u);
	obj3->RegistModel(GetModelKey());
	obj3->IsDrawShadowDepth(true);
	obj3->SetLightSetIdx(2);
	obj3->SetFogIdx(2);
	obj3->GetLocalNodeTrans(m_vFootPos, "man_root");
	AnimationClip* animationClip = dynamic_cast<AnimationClip*>(ResourceDatabase::Get<Object>(GetAnmKey()));
	CustomKingdomUtil::Assert(nullptr != animationClip, "nullpo");
	if (!CheckRepetition(animationClip, 0u))
	{
		pModel(2u)->RegistAnm(0u, animationClip);
	}
	SetType(0u);
	SetAnmState(0u);
}

void cTaskCharRouteGhost::SetParam(int kind)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	m_fMaxSpeed = cAppG03PacnRollGame2->pGhostParam()->GetGhostSpeed(kind);
	m_fAddSpeed = cAppG03PacnRollGame2->pGhostParam()->GetGhostAddSpeed(kind);
	m_fTurnSpeed = cAppG03PacnRollGame2->pGhostParam()->GetGhostTurnSpeed(kind);
	fRadius() = cAppG03PacnRollGame2->pGhostParam()->GetGhostHitSize(kind);
	m_fIzikeSpeed = cAppG03PacnRollGame2->pGhostCommonParam()->GetGhostCommonIzikeSpeed();
	m_nIzikeFlashTime = KingdomGlobal::KingSys->nMSecToFrame(cAppG03PacnRollGame2->pGhostCommonParam()->GetGhostCommonIzikeFlash());
	m_fTargetSpeed = m_fMaxSpeed;
	cHitChar* cHitChar2 = new cHitChar(this, 1);
	cHitChar2->SetRange(fRadius());
	pvecHitChar().push_back(cHitChar2);
}

void cTaskCharRouteGhost::SetEffect()
{
	if (!GetNodeKey0().empty())
	{
		pPBModel(2u)->GetLocalNodeTrans(m_vEffectNode[0], GetNodeKey0());
	}
	if (!GetNodeKey1().empty())
	{
		pPBModel(2u)->GetLocalNodeTrans(m_vEffectNode[1], GetNodeKey1());
	}
}

void cTaskCharRouteGhost::SetOptionItem(int type)
{
	nOptionItem() = type;
	Vector4 v;
	if (KingdomGlobal::PacnRollSys->nRound() == 10)
	{
		::Constants::jetGeoVecCopy(v, cTaskCharGhost::CHOCO_POS[0]);
	}
	else if (KingdomGlobal::PacnRollSys->nRound() == 12)
	{
		::Constants::jetGeoVecCopy(v, cTaskCharGhost::CHOCO_POS[1]);
	}
	else
	{
		::Constants::jetGeoVecZero(v);
	}
	switch (type)
	{
	case 1:
		pPBModel()->RegistExpand(0u, "item_root", "BIN_ITEM_COOKIE_POW_MDL");
		if (pPBModel()->pExpandMdl(0u) != nullptr)
		{
			pPBModel()->pExpandMdl(0u)->RegistAnm(0u, "BIN_ITEM_COOKIE_POW_TPA");
		}
		pPBModel(1u)->RegistExpand(0u, "item_root", "BIN_ITEM_COOKIE_POW_MDL");
		if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
		{
			pPBModel(1u)->pExpandMdl(0u)->RegistAnm(0u, "BIN_ITEM_COOKIE_POW_TPA");
		}
		Constants::PACN_APP()->pTaskItem()->SetEnemyItem(1u, this, v);
		break;
	case 2:
		pPBModel()->RegistExpand(0u, "item_root", "BIN_ITEM_CHOCO_PACN_MDL");
		pPBModel(1u)->RegistExpand(0u, "item_root", "BIN_ITEM_CHOCO_PACN_MDL");
		Constants::PACN_APP()->pTaskItem()->SetEnemyItem(4u, this, v);
		break;
	case 3:
		pPBModel()->RegistExpand(0u, "item_root", "BIN_ITEM_CHOCO_KNIGHT_MDL");
		pPBModel(1u)->RegistExpand(0u, "item_root", "BIN_ITEM_CHOCO_KNIGHT_MDL");
		Constants::PACN_APP()->pTaskItem()->SetEnemyItem(2u, this, v);
		break;
	case 4:
		pPBModel()->RegistExpand(0u, "item_root", "BIN_ITEM_CHOCO_WING_MDL");
		pPBModel(1u)->RegistExpand(0u, "item_root", "BIN_ITEM_CHOCO_WING_MDL");
		Constants::PACN_APP()->pTaskItem()->SetEnemyItem(3u, this, v);
		break;
	}
	if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
	{
		pPBModel(1u)->pExpandMdl(0u)->Invisible();
	}
	if (IsDead())
	{
		if (pPBModel()->pExpandMdl(0u) != nullptr)
		{
			if (pPBModel()->pExpandMdl(0u) != nullptr)
			{
				pPBModel()->pExpandMdl(0u)->Invisible();
			}
		}
		if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
		{
			if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
			{
				pPBModel(1u)->pExpandMdl(0u)->Invisible();
			}
		}
	}
}

void cTaskCharRouteGhost::SetBombType(int type)
{
	m_nBombStatus = type;
}

void cTaskCharRouteGhost::FastDead()
{
	pPBModel()->Invisible();
	pPBModel(1u)->Invisible();
	pPBModel(2u)->Invisible();
	if (pPBModel()->pExpandMdl(0u) != nullptr)
	{
		if (pPBModel()->pExpandMdl(0u) != nullptr)
		{
			pPBModel()->pExpandMdl(0u)->Invisible();
		}
	}
	if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
	{
		if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
		{
			pPBModel(1u)->pExpandMdl(0u)->Invisible();
		}
	}
	m_PtkHndEffect[0]->Detach(true);
	m_PtkHndEffect[1]->Detach(true);
	nStat() = 9;
}

void cTaskCharRouteGhost::CheckStatus()
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	(void)(static_cast<cTaskGhostRoute*>(pParent()));
	if (nStat() >= 5)
	{
		return;
	}
	if (GetKind() < cTaskChar::eKIND::eKIND_RIDE_CAR)
	{
		if (!m_bPackAttack)
		{
			if (cAppG03PacnRollGame2->pTaskGhostMan()->IsIjike())
			{
				if (nStat() != 4 && nStat() != 3)
				{
					nStat() = 3;
				}
				pHitChar(0)->FlagOff(2u);
			}
			else if (nStat() == 4 || nStat() == 3)
			{
				nStat() = 1;
				pHitChar(0)->FlagOn(2u);
			}
		}
	}
	else if (!m_bPackAttack)
	{
		if (cAppG03PacnRollGame2->pTaskGhostMan()->IsIjike())
		{
			if (nStat() != 4 && nStat() != 3)
			{
				nStat() = 3;
			}
			if (cTaskCharPacman2->IsState(1u))
			{
				pHitChar(0)->FlagOff(2u);
			}
			else
			{
				pHitChar(0)->FlagOn(2u);
			}
		}
		else if (nStat() == 4 || nStat() == 3)
		{
			nStat() = 1;
			pHitChar(0)->FlagOn(2u);
		}
	}
	if (nStat() < 3 && nState() != 0)
	{
		SetState(0u);
		SetType(0u);
		SetAnmState(0u);
	}
}

void cTaskCharRouteGhost::Move()
{
	(void)(static_cast<cTaskGhostRoute*>(pParent()));
	if (!m_bMove)
	{
		return;
	}
	if (m_fSpeed != m_fTargetSpeed)
	{
		if (m_fSpeed > m_fTargetSpeed)
		{
			m_fSpeed -= m_fAddSpeed;
			if (m_fSpeed < m_fTargetSpeed)
			{
				m_fSpeed = m_fTargetSpeed;
			}
		}
		else
		{
			m_fSpeed += m_fAddSpeed;
			if (m_fSpeed > m_fTargetSpeed)
			{
				m_fSpeed = m_fTargetSpeed;
			}
		}
	}
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	float num = Mathf::Atan2(vMoveTarget()[0] - vTrans()[0], vMoveTarget()[2] - vTrans()[2]);
	v[0] = Mathf::Sin(num);
	v[2] = Mathf::Cos(num);
	::Constants::jetGeoVecNormalize(v, v);
	if (vRotate()[1] != num)
	{
		float num2 = Mathf::Sin(vRotate()[1]);
		float num3 = Mathf::Cos(vRotate()[1]);
		float num4 = v[0] * num3 - v[2] * num2;
		if (num4 > 0.0f)
		{
			vRotate()[1] += m_fTurnSpeed;
		}
		else
		{
			vRotate()[1] -= m_fTurnSpeed;
		}
		num2 = Mathf::Sin(vRotate()[1]);
		num3 = Mathf::Cos(vRotate()[1]);
		if ((num4 > 0.0f && v[0] * num3 - v[2] * num2 < 0.0f) || (num4 < 0.0f && v[0] * num3 - v[2] * num2 > 0.0f))
		{
			vRotate()[1] = num;
		}
	}
	vTrans()[0] += v[0] * m_fSpeed;
	vTrans()[2] += v[2] * m_fSpeed;
	if (v[0] < 0.0f)
	{
		if (vMoveTarget()[0] > vTrans()[0])
		{
			vTrans()[0] = vMoveTarget()[0];
		}
	}
	else if (vMoveTarget()[0] < vTrans()[0])
	{
		vTrans()[0] = vMoveTarget()[0];
	}
	if (v[2] < 0.0f)
	{
		if (vMoveTarget()[2] > vTrans()[2])
		{
			vTrans()[2] = vMoveTarget()[2];
		}
	}
	else if (vMoveTarget()[2] < vTrans()[2])
	{
		vTrans()[2] = vMoveTarget()[2];
	}
}

void cTaskCharRouteGhost::Attacked(cHitChar* pRev, cHitChar* pAtk)
{
	if (Constants::IS_PAC_PAUSE() || Constants::PACN_APP()->IsNakajimaPause(this))
	{
		return;
	}
	if (GetKind() < cTaskChar::eKIND::eKIND_RIDE_CAR)
	{
		if (Constants::PACN_APP()->pTaskGhostMan()->IsIjike())
		{
			SetType(1u);
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE017_GHOST);
			Constants::PACN_APP()->NakajimaPause(this);
			m_bPackAttack = true;
			nStat() = 5;
			m_nCount = 0;
			KingdomGlobal::PacnRollSys->AddDeadEnemyCnt();
		}
	}
	else if (Constants::PACN_APP()->pTaskGhostMan()->IsIjike() && Constants::PACN_APP()->pTaskCharPacman()->IsState(1u))
	{
		SetType(1u);
		Constants::PACN_APP()->NakajimaPause(this);
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE017_GHOST);
		m_bPackAttack = true;
		nStat() = 5;
		m_nCount = 0;
		if (nullptr != m_SndHndEngine)
		{
			SoundManager::get_SE()->Stop(m_SndHndEngine);
		}
		m_bSndEngineStop = true;
		KingdomGlobal::PacnRollSys->AddDeadEnemyCnt();
	}
}

void cTaskCharRouteGhost::HitPushOut(Vector4 vVec, cTaskChar* pChar)
{
	if (pChar->nKind() != cTaskChar::eKIND::eKIND_BOSS && pChar->nKind() != cTaskChar::eKIND::eKIND_BOSS_IJIKE && pChar->nKind() != nKind())
	{
		return;
	}
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vTrans());
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, pChar->vTrans());
	Vector4 v3;
	::Constants::jetGeoVecAdd(v3, v, vVec);
	float num = fRadius() + pChar->fRadius() - ::Constants::jetGeoVecDistance(v2, v);
	float num2 = Mathf::Atan2(v3[0] - v[0], v3[2] - v[2]);
	float num3 = Mathf::Sin(num2 + 0.8f) * num;
	float num4 = Mathf::Cos(num2 + 0.8f) * num;
	if (!(num < 0.0f))
	{
		v[0] += num3;
		v[2] += num4;
		::Constants::jetGeoVecCopy(vTrans(), v);
		cTaskGhostRoute* cTaskGhostRoute2 = static_cast<cTaskGhostRoute*>(pParent());
		Vector4 v4 = Vector4::zero;
		::Constants::jetGeoVecSet(v4, pChar->vTrans()[0] - vMoveTarget()[0], 0.0f, pChar->vTrans()[2] - vMoveTarget()[2], 0.0f);
		if (::Constants::jetGeoVecLength(v4) < fRadius() + pChar->fRadius() && m_nTargetPointID++ >= cTaskGhostRoute2->nRouteNum())
		{
			m_nTargetPointID = 0;
		}
	}
}

}
