#include "kingdom/game/g03/cTaskBgGimmickGondola.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cGondolaEffect.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <Extentions.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g03
{

cTaskBgGimmickGondola::cTaskBgGimmickGondola(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickGondola");
	Init();
}

void cTaskBgGimmickGondola::Init()
{
	m_bBoot = false;
	m_nSleepTime = 0u;
	m_fAnmFrameOld = 0.0f;
	m_nEff = 0u;
	NextMode(0u, 0u);
	for (int i = 0; i < (int)std::size(m_PtkHnd); i++)
	{
		m_PtkHnd[i] = new cGondolaEffect();
	}
	Constants::PACN_APP();
	ResMdl* resMdl = pFaceList()->pResMdl();
	for (uint num = 0u; (long)num < (long)resMdl->GetResNodeNumEntries(); num++)
	{
		ResNode* firstChildResNode = resMdl->GetFirstChildResNode((int)num);
		if (!firstChildResNode->IsValid())
		{
			continue;
		}
		if (CustomKingdomUtil::strstr(firstChildResNode->GetName(), "effect"))
		{
			Vector3 vector = firstChildResNode->GetRotate();
			::Constants::jetGeoVecSet(m_PtkHnd[m_nEff]->vRotateL(), vector.x, vector.y, vector.z, 1.0f);
			vector = firstChildResNode->GetTranslate();
			::Constants::jetGeoVecSet(m_PtkHnd[m_nEff]->vTransL(), vector.x, vector.y, vector.z, 1.0f);
			m_PtkHnd[m_nEff]->nType() = 1u;
			m_nEff++;
			if (m_nEff == 3)
			{
				break;
			}
		}
		else if (CustomKingdomUtil::strstr(firstChildResNode->GetName(), "ef_wind"))
		{
			Vector3 vector = firstChildResNode->GetRotate();
			::Constants::jetGeoVecSet(m_PtkHnd[m_nEff]->vRotateL(), vector.x, vector.y, vector.z, 1.0f);
			vector = firstChildResNode->GetTranslate();
			::Constants::jetGeoVecSet(m_PtkHnd[m_nEff]->vTransL(), vector.x, vector.y, vector.z, 1.0f);
			m_PtkHnd[m_nEff]->nType() = 2u;
			m_nEff++;
			if (m_nEff == 3)
			{
				break;
			}
		}
	}
	std::vector<SkinnedMeshRenderer*> componentsInChildren; // TODO(unity): componentsInChildren = base.Model.pScnMdl().GetComponentsInChildren<SkinnedMeshRenderer>()
	for (SkinnedMeshRenderer* obj : componentsInChildren)
	{
		Bounds localBounds = Bounds(); // TODO(unity): localBounds = obj.localBounds
		localBounds.Expand(1000.0f); // the converter dropped the argument (C#: Expand(1000f))
		// TODO(unity): obj.localBounds = localBounds;
	}
}

void cTaskBgGimmickGondola::Term()
{
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE041_KUUTYUU);
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickGondola::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (!Constants::IS_PAC_PAUSE() && cAppG03PacnRollGame2->IsGame() && !cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		float frameAnmChr = this->Model->GetFrameAnmChr(0u);
		float frameMaxAnmChr = this->Model->GetFrameMaxAnmChr(0u);
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
				if (m_nSleepTime != 0)
				{
					m_nSleepTime--;
				}
				if (m_nSleepTime == 0 && m_bBoot)
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
				{
					nModeStep(n) = 1u;
					for (uint num = 0u; num < m_nEff; num++)
					{
						uint kind = 33u;
						if (m_PtkHnd[num]->nType() == 2)
						{
							kind = 15u;
						}
						cAppG03PacnRollGame2->SetEffect(kind, m_PtkHnd[num]->vTransL(), Vector4(), m_PtkHnd[num]);
					}
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE041_KUUTYUU, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					goto _switch2_1u;
				}
			case 1u:
			_switch2_1u:
				if (m_bBoot && (frameAnmChr >= frameMaxAnmChr || frameAnmChr < m_fAnmFrameOld))
				{
					this->Model->SetFrameAnmChr(0u, 0.0f);
					m_bBoot = false;
					m_nSleepTime = 120u;
					for (uint num2 = 0u; num2 < m_nEff; num2++)
					{
						if (m_PtkHnd[num2]->IsValid() && m_PtkHnd[num2]->IsActive())
						{
							m_PtkHnd[num2]->Detach(true);
						}
					}
					NextMode(n, 0u);
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
				SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE041_KUUTYUU);
				break;
			}
		}
		m_fAnmFrameOld = frameAnmChr;
		if (m_bBoot)
		{
			this->Model->UpdateFrame();
		}
		cAppG03PacnRollGame2->pTaskGhostMan()->SetGondolaAnim(this->Model->GetFrameAnmChr(0u));
	}
	UpdateMatrix();
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecSet(v, pMtx()[0][3], pMtx()[1][3], pMtx()[2][3], 1.0f);
	return BaseExecute(pParam);
}

void cTaskBgGimmickGondola::Draw(int nStatus, cJetTask* pParam)
{
	Matrix4x4 m = Extentions::ConvertCoordinateSystem(pMtx());
	for (uint num = 0u; num < m_nEff; num++)
	{
		if (m_PtkHnd[num]->IsValid() && m_PtkHnd[num]->IsActive())
		{
			if (m_PtkHnd[num]->nType() == 2)
			{
				Vector4 v = Vector4::zero;
				::Constants::jetGeoVecSet(v, 0.0f, ::Constants::JET_DEG2RAD(180.0f), 0.0f, 1.0f);
				m_PtkHnd[num]->vSetRotate(v);
				m_PtkHnd[num]->UpdateMatrix(m);
			}
			else
			{
				Vector4 v2;
				::Constants::jetGeoVecTrans(v2, pMtx(), m_PtkHnd[num]->vTransL());
				m_PtkHnd[num]->vSetTrans(v2);
				m_PtkHnd[num]->vSetRotate(m_PtkHnd[num]->vRotateL());
			}
		}
	}
	cTaskBgGimmick::Draw(nStatus, pParam);
}

void cTaskBgGimmickGondola::HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
	if (m_nSleepTime == 0)
	{
		m_bBoot = true;
	}
}

}
