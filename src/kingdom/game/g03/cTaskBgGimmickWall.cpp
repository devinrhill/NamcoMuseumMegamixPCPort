#include "kingdom/game/g03/cTaskBgGimmickWall.hpp"
#include <ESEKind.hpp>
#include <SoundManager.hpp>

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskBgGimmickWall::cTaskBgGimmickWall(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickWall");
	Init();
}

void cTaskBgGimmickWall::Init()
{
	NextMode(0u, 0u);
	::Constants::jetGeoVecUnit(m_vAreaSize);
	m_vAreaSize[0] = 7.0f;
	m_vAreaSize[1] = 6.0f;
	m_vAreaSize[2] = 6.0f;
	m_vAreaSize[3] = 1.0f;
	for (uint num = 0u; num < 2; num++)
	{
		::Constants::jetGeoVecUnit(m_vEffectTransL[num]);
	}
	m_nEff = 0u;
	m_fAddRot = 0.0f;
	m_nCnt = 0u;
	IsDrawShadowDepth(true);
	Constants::PACN_APP();
	ResMdl* resMdl = pFaceList()->pResMdl();
	for (uint num2 = 0u; (long)num2 < (long)resMdl->GetChildResNodeNumEntries(); num2++)
	{
		ResNode* firstChildChildResNode = resMdl->GetFirstChildChildResNode((int)num2);
		if (!firstChildChildResNode->IsValid() || !CustomKingdomUtil::strstr(firstChildChildResNode->GetName(), "effect"))
		{
			continue;
		}
		Vector3 vector = firstChildChildResNode->GetTranslate();
		::Constants::jetGeoVecSet(m_vEffectTransL[m_nEff], vector.x, vector.y, vector.z, 1.0f);
		if (m_nEff == 0)
		{
			::Constants::jetGeoVecCopy(m_vAreaSize, m_vEffectTransL[m_nEff]);
			m_vAreaSize[2] = m_vAreaSize[1];
			for (int i = 0; i < 4; i++)
			{
				m_vAreaSize[i] = ::Constants::fabs(m_vAreaSize[i]);
			}
		}
		if (++m_nEff >= 2)
		{
			break;
		}
	}
}

void cTaskBgGimmickWall::Term()
{
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickWall::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (!Constants::IS_PAC_PAUSE() && cAppG03PacnRollGame2->IsGame() && !cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		uint n = 0u;
		if (nModeState(n) == 0)
		{
			nOldState(n) = nModeState(n);
			switch (nModeStep(n))
			{
			case 0u:
			_switch1_0u:
				nModeStep(n) = 1u;
				goto _switch1_1u;
			case 1u:
			_switch1_1u:
				if (AreaCheck())
				{
					NextMode(n, 1u);
				}
				if (nOldState(n) == nModeState(n))
				{
					break;
				}
				nModeStep(n) = 2u;
				goto _switch1_2u;
			case 2u:
			_switch1_2u:
				nModeStep(n) = 0u;
				break;
			}
		}
		else if (1 == nModeState(n))
		{
			nOldState(n) = nModeState(n);
			switch (nModeStep(n))
			{
			case 0u:
			_switch2_0u:
				nModeStep(n) = 1u;
				OnState(128u);
				goto _switch2_1u;
			case 1u:
			_switch2_1u:
				m_fAddRot += 0.0007f;
				if (m_fAddRot > 0.03f)
				{
					m_fAddRot = 0.03f;
				}
				this->Model->vRotate()[0] += m_fAddRot;
				if (this->Model->vRotate()[0] > ::Constants::JET_DEG2RAD(90.0f))
				{
					this->Model->vRotate()[0] = ::Constants::JET_DEG2RAD(90.0f);
					NextMode(n, 2u);
				}
				if (nOldState(n) == nModeState(n))
				{
					break;
				}
				nModeStep(n) = 2u;
				goto _switch2_2u;
			case 2u:
			_switch2_2u:
				nModeStep(n) = 0u;
				// WII (80154548): the wall landing sound, missing from the port
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE046_WALL);
				break;
			}
		}
		else if (2 == nModeState(n))
		{
			nOldState(n) = nModeState(n);
			switch (nModeStep(n))
			{
			case 0u:
			_switch3_0u:
				{
					nModeStep(n) = 1u;
					for (uint num = 0u; num < m_nEff; num++)
					{
						Vector4 v;
						::Constants::jetGeoVecTrans(v, pMtx(), m_vEffectTransL[num]);
						cAppG03PacnRollGame2->SetEffect(34u, v, Vector4());
					}
					m_nCnt = 50u;
					IsCollision() = false;      // WII (80154650): collision goes off on entry (the port: one frame later)
					goto _switch3_1u;
				}
			case 1u:
			_switch3_1u:
				if (m_nCnt != 0)
				{
					if (m_nCnt % 3 == 0)
					{
						this->Model->Invisible();
					}
					else
					{
						this->Model->Visible();
					}
					m_nCnt--;
				}
				else
				{
					NextMode(n, 3u);
				}
				if (nOldState(n) == nModeState(n))
				{
					break;
				}
				nModeStep(n) = 2u;
				goto _switch3_2u;
			case 2u:
			_switch3_2u:
				nModeStep(n) = 0u;
				break;
			}
		}
		else if (3 == nModeState(n))
		{
			nOldState(n) = nModeState(n);
			switch (nModeStep(n))
			{
			case 0u:
			_switch4_0u:
				nModeStep(n) = 1u;
				this->Model->Invisible();
				IsCollision() = false;
				goto _switch4_1u;
			case 1u:
			_switch4_1u:
				if (nOldState(n) == nModeState(n))
				{
					break;
				}
				nModeStep(n) = 2u;
				goto _switch4_2u;
			case 2u:
			_switch4_2u:
				nModeStep(n) = 0u;
				break;
			}
		}
	}
	UpdateMatrix();
	return BaseExecute(pParam);     // WII (801547dc): skips cTaskBgGimmick::Execute and goes to cTaskBgHit::Execute
}

void cTaskBgGimmickWall::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

void cTaskBgGimmickWall::HitCallbackWall(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecTrans(v, pMtxRotate(), face->vNorm);
	if (v[1] > 0.0f)
	{
		if (!cTaskCharPacman2->IsState(8u))
		{
			cTaskCharPacman2->DamageProcess(1u, v, 0.001f);
		}
		else
		{
			cTaskCharPacman2->DamageProcess(0u, v, 0.001f);
		}
	}
}

bool cTaskBgGimmickWall::AreaCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector3 min = Vector3();
	min.x = 0.0f - m_vAreaSize[0];
	min.y = -1.0f;
	min.z = 0.0f;
	Vector3 max = Vector3();
	max.x = m_vAreaSize[0];
	max.y = m_vAreaSize[1] * 0.75f;
	max.z = m_vAreaSize[2];
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, this->Model->vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, this->Model->vRotate());
	Matrix4x4 m2;
	::Constants::jetGeoMtx44to34(m2, m);
	AABB target = AABB(min, max, false);
	target.Set(target, m2);
	Vector3 trans = Vector3();
	trans.x = cTaskCharPacman2->vTrans()[0];
	trans.y = cTaskCharPacman2->vTrans()[1];
	trans.z = cTaskCharPacman2->vTrans()[2];
	float _discard5;
	if (DummyNW4R::DistSqPoint3ToAABB(trans, target, _discard5) != 0.0f)
	{
		return false;
	}
	return true;
}

}
