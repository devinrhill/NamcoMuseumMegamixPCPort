#include "kingdom/game/g03/cTaskCharAreaGhost.hpp"
#include "DummyNW4R.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharEnemy.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskGhostArea.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

float& cTaskCharAreaGhost::fFootDist()
{
	return m_fFootDist;
}

int& cTaskCharAreaGhost::nTargetPointID()
{
	return m_nTargetPointID;
}

Vector4& cTaskCharAreaGhost::vTarget()
{
	return m_vTarget;
}

Vector4& cTaskCharAreaGhost::vMoveTarget()
{
	return m_vMoveTarget;
}

int& cTaskCharAreaGhost::nMoveStatus()
{
	return m_nMoveStatus;
}

int& cTaskCharAreaGhost::nMoveCount()
{
	return m_nMoveCount;
}

cTaskChar::eKIND cTaskCharAreaGhost::GetKind()
{
	return cTaskChar::eKIND::eKIND_PACMAN;
}

string cTaskCharAreaGhost::GetModelKey()
{
	return string();
}

string cTaskCharAreaGhost::GetAnmKey()
{
	return string();
}

string cTaskCharAreaGhost::GetSRTAnmKey()
{
	return string();
}

uint cTaskCharAreaGhost::GetPushOutLevel()
{
	return 0u;
}

// DOL 0x8015dedc [audited: described by the 43rd-session pass] (vtable +0x5c: stat > 8)
bool cTaskCharAreaGhost::IsDead()
{
	if (nStat() <= 8)
	{
		return false;
	}
	return true;
}

cTaskCharAreaGhost::cTaskCharAreaGhost()
{
	SetName("cTaskCharAreaGhost");
	Init();
}

// DOL 0x8015b5cc [audited: class row done in the 43rd session; function not individually re-read]
void cTaskCharAreaGhost::Init()
{
	m_nCount = 0;
	m_fEatSpeed = 0.0f;
	m_bHydeItem = false;
	m_nMoveStatus = 0;
	m_nMoveCount = 0;
	m_fSpeed = 0.0f;
	m_fTargetSpeed = 0.0f;
	m_nTargetPointID = 0;
	m_vTarget = Vector4();
	m_vMoveTarget = Vector4();
	m_vPacManTrans = Vector4();
	m_bCanFlying = false;
	m_nFlyingProc = 0u;
	m_bFlying = false;
	m_bFlyingUp = false;
	m_vFlyingTarget = Vector4();
	m_bPackAttack = false;
	m_fFootDist = 0.0f;
	m_fMaxSpeed = 0.0f;
	m_fAddSpeed = 0.0f;
	m_fTurnSpeed = 0.0f;
	m_fIzikeSpeed = 0.0f;
	m_nIzikeFlashTime = 0;
	if (KingdomGlobal::PacnRollSys->nRound() == 1 || KingdomGlobal::PacnRollSys->nRound() == 9)
	{
		m_bCanFlying = true;
	}
	nStat() = 0;
	SetState(0u);
}

void cTaskCharAreaGhost::Term()
{
	cTaskCharGhost::Term();
}

// DOL 0x8015b6cc [audited: described by the 43rd-session pass] (Blinky / Pinky / Inky / Clyde AI by GetKind 3 / 2 / 4 / 1)
bool cTaskCharAreaGhost::Execute(cJetTask* pParam)
{
	InGamePadManager::GetPad1No();
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	cTaskGhostArea* cTaskGhostArea2 = static_cast<cTaskGhostArea*>(pParent());
	if (Constants::IS_PAC_PAUSE() || cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		return true;
	}
	if (nStat() != 0 && ::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), pModel(nType())->vTrans()) > 50.0f)
	{
		return true;
	}
	if (cAppG03PacnRollGame2->IsGame())
	{
		::Constants::jetGeoVecCopy(vTransOld(), vTrans());
		if (!cTaskCharPacman2->IsState(8u))
		{
			fRadius();
			::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), pModel(nType())->vTrans());
		}
		if (nStat() < 8)
		{
			if (cTaskGhostArea2->GhostNum() == 1 && !cTaskGhostArea2->IsAreaFusion())
			{
				if (::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), pModel(nType())->vTrans()) < 10.0f)
				{
					Constants::PACN_APP()->pTaskCollision()->PushBackChar(this);
				}
			}
			else if (::Constants::jetGeoVecDistance(cTaskCharPacman2->vTrans(), pModel(nType())->vTrans()) < 50.0f)
			{
				Constants::PACN_APP()->pTaskCollision()->PushBackChar(this);
			}
		}
		CheckStatus();
		switch (nStat())
		{
		case 0:
			m_nMoveStatus = 0;
			nStat() = 1;
			break;
		case 1:
			m_nMoveStatus = 1;
			nStat() = 2;
			break;
		case 2:
			m_nMoveStatus = 0;
			m_fTargetSpeed = m_fMaxSpeed;
			Move();
			if (vTarget()[0] == vTrans()[0] && vTarget()[2] == vTrans()[2])
			{
				nStat() = 1;
			}
			else if (vMoveTarget()[0] == vTrans()[0] && vMoveTarget()[2] == vTrans()[2])
			{
				nStat() = 3;
			}
			break;
		case 3:
			m_nMoveStatus = 2;
			nStat() = 2;
			break;
		case 4:
			if (m_nCount > 30)
			{
				switch (GetKind())
				{
				case cTaskChar::eKIND::eKIND_CLYDE:
					m_nMoveStatus = 3;
					break;
				case cTaskChar::eKIND::eKIND_PINKY:
					if (m_nMoveCount < 300)
					{
						m_nMoveCount++;
						m_nMoveStatus = 3;
					}
					else if (m_nMoveCount == 300)
					{
						m_nMoveCount++;
						m_nMoveStatus = 1;
						::Constants::jetGeoVecCopy(m_vPacManTrans, cTaskCharPacman2->vTrans());
					}
					else if (m_nMoveCount == 301)
					{
						m_nMoveStatus = 0;
						m_fTargetSpeed = m_fMaxSpeed;
						if (vTarget()[0] == vTrans()[0] && vTarget()[2] == vTrans()[2])
						{
							m_nMoveStatus = 1;
						}
						else if (vMoveTarget()[0] == vTrans()[0] && vMoveTarget()[2] == vTrans()[2])
						{
							m_nMoveStatus = 2;
						}
						if (m_vPacManTrans[0] != cTaskCharPacman2->vTrans()[0] || m_vPacManTrans[2] != cTaskCharPacman2->vTrans()[2])
						{
							m_nMoveCount = 0;
						}
						::Constants::jetGeoVecCopy(m_vPacManTrans, cTaskCharPacman2->vTrans());
					}
					break;
				case cTaskChar::eKIND::eKIND_BLINKY:
					if (m_nMoveCount == 0)
					{
						m_nMoveStatus = 1;
						m_nMoveCount = 1;
					}
					else if (m_nMoveCount == 1)
					{
						m_nMoveStatus = 0;
						m_fTargetSpeed = m_fMaxSpeed;
						if (vTarget()[0] == vTrans()[0] && vTarget()[2] == vTrans()[2])
						{
							m_nMoveStatus = 1;
						}
						else if (vMoveTarget()[0] == vTrans()[0] && vMoveTarget()[2] == vTrans()[2])
						{
							m_nMoveStatus = 2;
						}
						Vector4 v = Vector4::zero;
						::Constants::jetGeoVecSet(v, cTaskCharPacman2->vTrans()[0] - vTrans()[0], 0.0f, cTaskCharPacman2->vTrans()[2] - vTrans()[2], 0.0f);
						if (::Constants::jetGeoVecLength(v) < 2.0f)
						{
							Vector4 array[2] = {};
							::Constants::jetGeoVecCopy(array[0], v);
							::Constants::jetGeoVecSet(array[1], vMoveTarget()[0] - vTrans()[0], 0.0f, vMoveTarget()[2] - vTrans()[2], 0.0f);
							float x = array[0][0] * array[1][0] + array[0][2] * array[1][2];
							float y = array[0][0] * array[1][2] - array[0][2] * array[1][0];
							if (Mathf::Abs(57.29578f * Mathf::Atan2(y, x)) > 90.0f)
							{
								m_nMoveStatus = 3;
								m_nMoveCount = 2;
							}
						}
					}
					else if (m_nMoveCount == 2)
					{
						Vector4 v2 = Vector4::zero;
						::Constants::jetGeoVecSet(v2, cTaskCharPacman2->vTrans()[0] - vTrans()[0], 0.0f, cTaskCharPacman2->vTrans()[2] - vTrans()[2], 0.0f);
						if (::Constants::jetGeoVecLength(v2) < 5.0f)
						{
							m_nMoveStatus = 3;
						}
						else
						{
							m_nMoveCount = 0;
						}
					}
					break;
				case cTaskChar::eKIND::eKIND_INKY:
					if (m_nMoveCount < 300)
					{
						m_nMoveStatus = 3;
					}
					else if (m_nMoveCount < 360)
					{
						m_nMoveStatus = 4;
					}
					else
					{
						m_nMoveCount = 0;
					}
					m_nMoveCount++;
					break;
				}
				Move();
			}
			else
			{
				m_nMoveCount = 0;
				m_nCount++;
			}
			if (!cTaskGhostArea2->IsAreaInPacMan())
			{
				nStat() = 1;
			}
			break;
		case 6:
			m_nMoveStatus = 1;
			if (nState() != 1)
			{
				SetState(1u);
				SetType(1u);
				SetAnmState(1u);
				pHitChar(0)->FlagOff(2u);
			}
			// "max <= frame": the port's one-time clips stop at max (the DOL's "max < frame" never fired: ghosts froze in the ijike start, state 6)
			if (pModel(nType())->GetFrameMaxAnmChr(0u) <= pModel(nType())->GetFrameAnmChr(0u))
			{
				SetAnmState(2u);
				nStat() = 7;
			}
			break;
		case 7:
			if (!cTaskGhostArea2->IsAreaInPacMan())
			{
				m_nMoveStatus = 0;
				if (vTarget()[0] == vTrans()[0] && vTarget()[2] == vTrans()[2])
				{
					m_nMoveStatus = 1;
				}
				else if (vMoveTarget()[0] == vTrans()[0] && vMoveTarget()[2] == vTrans()[2])
				{
					m_nMoveStatus = 2;
				}
			}
			else
			{
				m_nMoveStatus = 4;
			}
			m_fTargetSpeed = m_fMaxSpeed * m_fIzikeSpeed;
			Move();
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
		case 8:
			{
				m_nMoveStatus = 0;
				m_nCount++;
				if ((float)m_nCount > 25.0f)
				{
					::Constants::jetGeoVecZero(vScale());
				}
				else
				{
					vScale()[0] = (25.0f - (float)m_nCount) / 25.0f;
					vScale()[1] = vScale()[0];
					vScale()[2] = vScale()[0];
					vScale()[3] = 1.0f;
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
					::Constants::jetGeoVecZero(vScale());
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
					nStat() = 9;
				}
				break;
			}
		case 9:
			m_nMoveStatus = 0;
			Constants::PACN_APP()->NakajimaPlay(this);
			FastDead();
			break;
		case 10:
			Constants::PACN_APP()->SetEffect(29u, vStartPos(), Vector4());
			m_nCount = 0;
			m_bPackAttack = false;
			m_fEatSpeed = 0.0f;
			m_fSpeed = 0.0f;
			m_fTargetSpeed = m_fMaxSpeed;
			m_nTargetPointID = 0;
			m_bSndEngineStop = false;
			m_vTarget = Vector4();
			m_vMoveTarget = Vector4();
			::Constants::jetGeoVecZero(vRotate());
			::Constants::jetGeoVecSet(vScale(), 1.0f, 1.0f, 1.0f, 1.0f);
			if (!cAppG03PacnRollGame2->pTaskGhostMan()->IsIjike())
			{
				pPBModel()->Visible();
				if (!m_bHydeItem && pPBModel()->pExpandMdl(0u) != nullptr)
				{
					if (pPBModel()->pExpandMdl(0u) != nullptr)
					{
						pPBModel()->pExpandMdl(0u)->Visible();
					}
				}
				SetType(0u);
			}
			else
			{
				pPBModel(1u)->Visible();
				if (!m_bHydeItem && pPBModel(1u)->pExpandMdl(0u) != nullptr)
				{
					if (pPBModel(1u)->pExpandMdl(0u) != nullptr)
					{
						pPBModel(1u)->pExpandMdl(0u)->Visible();
					}
				}
				SetType(1u);
			}
			SetState(0u);
			pHitChar(0)->FlagOn(2u);
			::Constants::jetGeoVecCopy(vTrans(), vStartPos());
			nStat() = 1;
			break;
		case 11:
			if (bRevival())
			{
				nStat() = 10;
				bRevival() = false;
			}
			break;
		}
	}
	if (cTaskGhostArea2->IsNeedBottomCheck() && nStat() < 8 && !m_bFlying)
	{
		vTrans()[1] += 5.0f;
		float _discard3;
		Constants::sFace* _discard4;
		cTaskBgHit* _discard5;
		if (!cAppG03PacnRollGame2->pTaskCollision()->CheckBottom(vTrans(), _discard3, vTrans(), _discard4, _discard5))
		{
			vTrans()[1] -= 5.0f;
		}
	}
	Vector4 v3;
	::Constants::jetGeoVecCopy(v3, vTrans());
	v3[1] += m_fFootDist;
	::Constants::jetGeoVecCopy(pCurrentModel()->vTrans(), v3);
	::Constants::jetGeoVecCopy(pCurrentModel()->vRotate(), vRotate());
	::Constants::jetGeoVecCopy(pCurrentModel()->vScale(), vScale());
	pHitChar(0)->SetOfsPos(0.0f, 0.3f, 0.0f);
	return cTaskCharEnemy::Execute(pParam);
}

// DOL 0x8015c874 [audited: described by the 43rd-session pass] (the Visible / Invisible toggles are the C#'s; the DOL only submits the current model)
void cTaskCharAreaGhost::Draw(int nStatus, cJetTask* pParam)
{
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, pCurrentModel()->vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, pCurrentModel()->vRotate());
	Matrix4x4 _discard6;
	::Constants::jetGeoMtx44to34(_discard6, m);
	DummyNW4R::SetMtx(pCurrentModel()->pScnMdl(), m);
	DummyNW4R::SetScale(pCurrentModel()->pScnMdl(), pCurrentModel()->vScale()[0], pCurrentModel()->vScale()[1], pCurrentModel()->vScale()[2]);
	pCurrentModel()->UpdateMatrix();   // DOL 0x8015ca20 [audited: described by the 43rd-session pass]: the current model's push-back (+0xd8) into this frame's scene list (0x8009c3f4)
	if (nStat() != 11)
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

// inline in the DOL (Execute 0x8015b6cc, Attacked, CheckStatus ...): Invisible the current model + its expand model (+0x60), type +0x1d0,
// Visible the new one + its expand model unless the item is hidden (+0x28c)
void cTaskCharAreaGhost::SetType(const uint& type)
{
	CustomKingdomUtil::Assert(2 > type, "error");
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
	if (!m_bHydeItem && pCurrentModel()->pExpandMdl(0u) != nullptr)
	{
		if (pCurrentModel()->pExpandMdl(0u) != nullptr)
		{
			pCurrentModel()->pExpandMdl(0u)->Visible();
		}
	}
}

void cTaskCharAreaGhost::SetAnmState(const uint& state)
{
	switch (state)
	{
	case 0u:
		SetAnm(GetAnmKey());
		SetAnm(GetSRTAnmKey());
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

void cTaskCharAreaGhost::SetAnm(string anime, uint idx)
{
	AnimationClip* animationClip = dynamic_cast<AnimationClip*>(ResourceDatabase::Get<Object>(anime));
	CustomKingdomUtil::Assert(nullptr != animationClip, "nullpo");
	if (!CheckRepetition(animationClip, idx))
	{
		pModel(nType())->RegistAnm(idx, animationClip);
	}
}

// DOL 0x8015caa0 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskCharAreaGhost::Move()
{
	(void)(static_cast<cTaskGhostArea*>(pParent()));
	if (m_bFlying)
	{
		::Constants::jetGeoVecCopy(vMoveTarget(), m_vFlyingTarget);
	}
	float num = Mathf::Atan2(vMoveTarget()[0] - vTrans()[0], vMoveTarget()[2] - vTrans()[2]);
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecSet(v, vMoveTarget()[0] - vTrans()[0], 0.0f, vMoveTarget()[2] - vTrans()[2], 0.0f);
	float num2 = ::Constants::jetGeoVecLength(v);
	Vector4 v2;
	::Constants::jetGeoVecZero(v2);
	v2[0] = Mathf::Sin(num);
	v2[2] = Mathf::Cos(num);
	::Constants::jetGeoVecNormalize(v2, v2);
	if (m_bFlying)
	{
		::Constants::jetGeoVecSet(v, m_vFlyingTarget[0] - vTrans()[0], 0.0f, m_vFlyingTarget[2] - vTrans()[2], 0.0f);
		num2 = ::Constants::jetGeoVecLength(v);
	}
	if (m_bCanFlying && !m_bFlying)
	{
		float num3 = fRadius() * 1.5f;
		::Constants::jetGeoVecCopy(m_vFlyingTarget, vTrans());
		if (num2 > num3)
		{
			m_vFlyingTarget[0] += v2[0] * num3;
			m_vFlyingTarget[1] += 5.0f;
			m_vFlyingTarget[2] += v2[2] * num3;
		}
		else
		{
			m_vFlyingTarget[0] = vMoveTarget()[0];
			m_vFlyingTarget[1] += 5.0f;
			m_vFlyingTarget[2] = vMoveTarget()[2];
		}
		float _discard8;
		Constants::sFace* _discard9;
		cTaskBgHit* _discard10;
		if (Constants::PACN_APP()->pTaskCollision()->CheckBottom(m_vFlyingTarget, _discard8, m_vFlyingTarget, _discard9, _discard10))
		{
			float num4 = 10.0f;
			if (KingdomGlobal::PacnRollSys->nRound() == 9)
			{
				num4 = 0.9f;
			}
			else if (KingdomGlobal::PacnRollSys->nRound() == 1)
			{
				num4 = 1.5f;
			}
			if (Mathf::Abs(m_vFlyingTarget[1] - vTrans()[1]) > num4)
			{
				if (vTrans()[1] < m_vFlyingTarget[1])
				{
					m_vFlyingTarget[1] += 0.1f;
					m_bFlyingUp = true;
				}
				else
				{
					m_bFlyingUp = false;
				}
				m_nFlyingProc = 0u;
				m_bFlying = true;
			}
		}
	}
	if (vRotate()[1] != num && (vMoveTarget()[0] != vTrans()[0] || vMoveTarget()[2] != vTrans()[2]))
	{
		float num5 = Mathf::Sin(vRotate()[1]);
		float num6 = Mathf::Cos(vRotate()[1]);
		float num7 = v2[0] * num6 - v2[2] * num5;
		if (num7 > 0.0f)
		{
			vRotate()[1] += m_fTurnSpeed;
		}
		else
		{
			vRotate()[1] -= m_fTurnSpeed;
		}
		num5 = Mathf::Sin(vRotate()[1]);
		num6 = Mathf::Cos(vRotate()[1]);
		if ((num7 > 0.0f && v2[0] * num6 - v2[2] * num5 < 0.0f) || (num7 < 0.0f && v2[0] * num6 - v2[2] * num5 > 0.0f))
		{
			vRotate()[1] = num;
		}
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
	if (!m_bFlying)
	{
		if (num2 > m_fSpeed)
		{
			vTrans()[0] += v2[0] * m_fSpeed;
			vTrans()[2] += v2[2] * m_fSpeed;
		}
		else
		{
			vTrans()[0] = vMoveTarget()[0];
			vTrans()[2] = vMoveTarget()[2];
		}
		return;
	}
	if (m_bFlyingUp)
	{
		switch (m_nFlyingProc)
		{
		case 0u:
			vTrans()[1] += 0.05f;
			if (vTrans()[1] >= m_vFlyingTarget[1])
			{
				vTrans()[1] = m_vFlyingTarget[1];
				m_nFlyingProc++;
			}
			break;
		case 1u:
			if (num2 > m_fSpeed)
			{
				vTrans()[0] += v2[0] * 0.05f;
				vTrans()[2] += v2[2] * 0.05f;
			}
			else
			{
				vTrans()[0] = m_vFlyingTarget[0];
				vTrans()[2] = m_vFlyingTarget[2];
				m_nFlyingProc++;
			}
			break;
		case 2u:
			m_bFlying = false;
			break;
		}
		return;
	}
	switch (m_nFlyingProc)
	{
	case 0u:
		if (num2 > m_fSpeed)
		{
			vTrans()[0] += v2[0] * 0.05f;
			vTrans()[2] += v2[2] * 0.05f;
		}
		else
		{
			vTrans()[0] = m_vFlyingTarget[0];
			vTrans()[2] = m_vFlyingTarget[2];
			m_nFlyingProc++;
		}
		break;
	case 1u:
		vTrans()[1] -= 0.05f;
		if (vTrans()[1] <= m_vFlyingTarget[1])
		{
			vTrans()[1] = m_vFlyingTarget[1];
			m_nFlyingProc++;
		}
		break;
	case 2u:
		m_bFlying = false;
		break;
	}
}

// DOL 0x8015d080 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskCharAreaGhost::SetModel()
{
	nKind() = GetKind();
	cKingdomModel* obj = CreateModel();
	obj->RegistModel(GetModelKey());
	obj->IsDrawShadowDepth(true);
	obj->SetLightSetIdx(2);
	obj->SetFogIdx(2);
	cKingdomModel* obj2 = CreateModel(1u);
	obj2->RegistModel("BIN_GOS_IZIK_MDL");
	obj2->IsDrawShadowDepth(true);
	obj2->SetLightSetIdx(2);
	obj2->SetFogIdx(2);
	obj2->Invisible();
	SetType(0u);
	SetAnmState(0u);
	Vector4 v = Vector4();
	obj2->GetLocalNodeTrans(v, "asi");
	m_fFootDist = 0.0f - v[1];
}

// DOL 0x8015d2f8 [audited: described by the 43rd-session pass] (CSV +0x350 rows by kind, cols 1..4; common CSV +0x354; hit range 0.82)
void cTaskCharAreaGhost::SetParam()
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	m_fMaxSpeed = cAppG03PacnRollGame2->pGhostParam()->GetGhostSpeed((int)GetKind());
	m_fAddSpeed = cAppG03PacnRollGame2->pGhostParam()->GetGhostAddSpeed((int)GetKind());
	m_fTurnSpeed = cAppG03PacnRollGame2->pGhostParam()->GetGhostTurnSpeed((int)GetKind());
	fRadius() = cAppG03PacnRollGame2->pGhostParam()->GetGhostHitSize((int)GetKind());
	m_fIzikeSpeed = cAppG03PacnRollGame2->pGhostCommonParam()->GetGhostCommonIzikeSpeed();
	m_nIzikeFlashTime = KingdomGlobal::KingSys->nMSecToFrame(cAppG03PacnRollGame2->pGhostCommonParam()->GetGhostCommonIzikeFlash());
	m_fTargetSpeed = m_fMaxSpeed;
	cHitChar* cHitChar2 = new cHitChar(this, 1);
	cHitChar2->SetRange(0.82f);
	pvecHitChar().push_back(cHitChar2);
	nPushOutLevel() = GetPushOutLevel();
}

void cTaskCharAreaGhost::SetOptionItem(int type)
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
	if (IsDead() || m_bHydeItem)
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

// DOL 0x8015def8 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskCharAreaGhost::SetHydeItem(bool on)
{
	if (on)
	{
		m_bHydeItem = true;
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
	else
	{
		m_bHydeItem = false;
		if (!IsDead())
		{
			if (pCurrentModel()->pExpandMdl(0u) != nullptr)
			{
				pCurrentModel()->pExpandMdl(0u)->Visible();
			}
		}
	}
}

void cTaskCharAreaGhost::FastDead()
{
	pPBModel()->Invisible();
	pPBModel(1u)->Invisible();
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
	pHitChar(0)->FlagOff(2u);
	nStat() = 11;
}

// DOL 0x8015e070 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskCharAreaGhost::CheckStatus()
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskGhostArea* cTaskGhostArea2 = static_cast<cTaskGhostArea*>(pParent());
	if (nStat() >= 8)
	{
		return;
	}
	if (!m_bPackAttack)
	{
		if (cAppG03PacnRollGame2->pTaskGhostMan()->IsIjike())
		{
			if (nStat() != 7 && nStat() != 6)
			{
				nStat() = 6;
			}
		}
		else if (cTaskGhostArea2->IsAreaInPacMan())
		{
			if (nStat() != 4)
			{
				Vector4 v;
				::Constants::jetGeoVecCopy(v, pPBModel()->vTrans());
				v[1] += 2.0f;
				cAppG03PacnRollGame2->SetEffect(6u, v, Vector4());
				m_nCount = 0;
			}
			nStat() = 4;
		}
		else if (nStat() == 7 || nStat() == 4 || nStat() == 6)
		{
			nStat() = 1;
		}
	}
	if (nStat() < 6 && nState() != 0)
	{
		SetState(0u);
		SetType(0u);
		SetAnmState(0u);
		pHitChar(0)->FlagOn(2u);
	}
}

void cTaskCharAreaGhost::ChangeArea()
{
	m_vTarget = Vector4();
	m_vMoveTarget = Vector4();
	m_nTargetPointID = 0;
	m_nMoveStatus = 1;
	m_nMoveCount = 0;
}

// DOL 0x8015e394 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskCharAreaGhost::Attacked(cHitChar* pRev, cHitChar* pAtk)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACN_APP()->IsNakajimaPause(this) && Constants::PACN_APP()->pTaskGhostMan()->IsIjike())
	{
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE017_GHOST);
		Constants::PACN_APP()->NakajimaPause(this);
		SetType(1u);
		nStat() = 8;
		m_bPackAttack = true;
		m_nCount = 0;
		// (43rd: the C#'s AddDeadEnemyCnt here fed a Steam statistic; the DOL has none)
	}
}

// DOL 0x8015e524 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskCharAreaGhost::HitPushOut(Vector4 vVec, cTaskChar* pChar)
{
	if (m_bFlying || (nKind() < cTaskChar::eKIND::eKIND_BOSS && nKind() < pChar->nKind()))
	{
		return;
	}
	cTaskGhostArea* cTaskGhostArea2 = static_cast<cTaskGhostArea*>(pParent());
	Vector4 v;
	::Constants::jetGeoVecCopy(v, vTrans());
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, pChar->vTrans());
	Vector4 v3;
	::Constants::jetGeoVecAdd(v3, v, vVec);
	float num = (fRadius() - ::Constants::jetGeoVecDistance(v2, v)) / 2.0f;
	float num2 = Mathf::Atan2(v3[0] - v[0], v3[2] - v[2]);
	float num3 = Mathf::Sin(num2 + 0.8f) * num;
	float num4 = Mathf::Cos(num2 + 0.8f) * num;
	bool flag = false;
	v[0] += num3;
	v[2] += num4;
	float outDist;
	Constants::sFace* outFace;
	cTaskBgHit* outObj;
	if (cTaskGhostArea2->IsIntoAreaPos(v))
	{
		if (cTaskGhostArea2->IsNeedBottomCheck())
		{
			Vector4 v4;
			::Constants::jetGeoVecCopy(v4, v);
			v4[1] += 5.0f;
			if (Constants::PACN_APP()->pTaskCollision()->CheckBottom(v4, outDist, v4, outFace, outObj) && Mathf::Abs(v4[1] - v[1]) < 0.2f)
			{
				::Constants::jetGeoVecCopy(vTrans(), v);
				flag = true;
			}
		}
		else
		{
			::Constants::jetGeoVecCopy(vTrans(), v);
			flag = true;
		}
	}
	if (flag || nKind() >= cTaskChar::eKIND::eKIND_BOSS)
	{
		return;
	}
	::Constants::jetGeoVecCopy(v, vTrans());
	::Constants::jetGeoVecAdd(v3, v2, vVec);
	float num5 = Mathf::Atan2(v2[0] - v3[0], v2[2] - v3[2]);
	num = pChar->fRadius() - ::Constants::jetGeoVecDistance(v2, v);
	num3 = Mathf::Sin(num5 + 0.8f) * num;
	num4 = Mathf::Cos(num5 + 0.8f) * num;
	v2[0] += num3;
	v2[2] += num4;
	if (!cTaskGhostArea2->IsIntoAreaPos(v2))
	{
		return;
	}
	if (cTaskGhostArea2->IsNeedBottomCheck())
	{
		Vector4 v5;
		::Constants::jetGeoVecCopy(v5, v2);
		v5[1] += 5.0f;
		if (Constants::PACN_APP()->pTaskCollision()->CheckBottom(v5, outDist, v5, outFace, outObj) && Mathf::Abs(v5[1] - v2[1]) < 0.2f)
		{
			::Constants::jetGeoVecCopy(pChar->vTrans(), v2);
		}
	}
	else
	{
		::Constants::jetGeoVecCopy(pChar->vTrans(), v2);
	}
}

}
