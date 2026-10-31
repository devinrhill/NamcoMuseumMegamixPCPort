#include "GamePad.hpp"
#include "kingdom/game/g03/cTaskBgGimmickArrow.hpp"
#include "kingdom/MslRand.hpp"
#include "DummyNW4R.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "ResNode.hpp"
#include "ResUserData.hpp"
#include "ResUserDataItem.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>
#include "platform/devtools.hpp"
#include <cmath>

namespace kingdom::game::g03
{

// DOL 0x8014e348 [audited: per AUDIT_G03_G07 row] (user data IntervalTime / LifeTime / |Speed|; life defaults to the interval; direction = (0,0,1) turned by the node's raw Y rotation (ResNode here is Unity-mirrored, the x flip undoes that); default position copy; sound radius 15)
cTaskBgGimmickArrow::cTaskBgGimmickArrow(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickArrow");
	Init();
	ResUserData* resUserData = node1->GetResUserData();
	if (resUserData->IsValid())
	{
		for (int i = 0; i < resUserData->GetItemNumEntries(); i++)
		{
			ResUserDataItem* item = resUserData->GetItem(i);
			if (item->IsValid())
			{
				if (CustomKingdomUtil::strstr(item->GetName(), "IntervalTime"))
				{
					m_nIntervalTime = (uint)item->GetS32();
				}
				else if (CustomKingdomUtil::strstr(item->GetName(), "LifeTime"))
				{
					m_nLifeTime = (uint)item->GetS32();
				}
				else if (CustomKingdomUtil::strstr(item->GetName(), "Speed"))
				{
					m_fSpeed = ::Constants::fabs(item->GetF32());
				}
			}
		}
	}
	if (m_nLifeTime == 0)
	{
		m_nLifeTime = m_nIntervalTime;
	}
	float r = ::Constants::JET_DEG2RAD(node1->GetRotate().y);
	::Constants::jetGeoVecZero(m_vDir);
	m_vDir[2] = 1.0f;
	::Constants::jetGeoVecRotY(m_vDir, m_vDir, r);
	m_vDir.x *= -1.0f;
	::Constants::jetGeoVecCopy(m_vDefaultTrans, this->Model->vTrans());
	m_fSndLength = 15.0f;
}

void cTaskBgGimmickArrow::Init()
{
	m_nIntervalTime = 0u;
	m_nLifeTime = 0u;
	m_nIntervalCnt = 0u;
	m_fSpeed = 0.0f;
	m_bBreak = false;
	m_nBreakTime = 0u;
	m_bHit = false;
}

void cTaskBgGimmickArrow::Term()
{
	cTaskBgGimmick::Term();
}

// DOL 0x8014ed44 [audited: per AUDIT_G03_G07 row]
bool cTaskBgGimmickArrow::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	float array[6] = {5.0f, -5.0f, 10.0f, -10.0f, 3.0f, -3.0f};
	if (!Constants::IS_PAC_PAUSE() && cAppG03PacnRollGame2->IsGame() && !cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		m_nIntervalCnt++;
		if (m_nIntervalCnt > m_nLifeTime)
		{
			this->Model->Invisible();
			IsCollision() = false;
		}
		if (m_nIntervalCnt > m_nIntervalTime)
		{
			::Constants::jetGeoVecCopy(this->Model->vTrans(), m_vDefaultTrans);
			m_nIntervalCnt = 0u;
			m_bBreak = false;
			this->Model->vRotate()[0] = 0.0f;
			this->Model->Visible();
			IsCollision() = true;
			m_bHit = false;
			if (::Constants::jetGeoVecDistance(this->Model->vTrans(), cAppG03PacnRollGame2->pTaskCharPacman()->vTrans()) <= m_fSndLength)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE058_SHOT, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
			}
		}
		Vector4 v;
		if (!m_bBreak)
		{
			::Constants::jetGeoVecScale(v, m_vDir, m_fSpeed);
			::Constants::jetGeoVecAdd(this->Model->vTrans(), this->Model->vTrans(), v);
			if (HitCheck())
			{
				cAppG03PacnRollGame* cAppG03PacnRollGame3 = Constants::PACN_APP();
				cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame3->pTaskCharPacman();
				if (cTaskCharPacman2->nType() == 2)
				{
					m_bBreak = true;
					m_nBreakTime = 16u;
					m_fBreakRot = ::Constants::JET_DEG2RAD(array[kingdom::MslRandRange(0, 6)]   /* DOL 0x8014ed44: rand() % 6 (PtkSim::Rand15) */);
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE059_YA, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					GamePad::Rumble(0, 500);   // DOL 0x8014efd4 [audited: class row done in the 43rd session; function not individually re-read]
				}
				Vector4 v2;
				::Constants::jetGeoVecSub(v2, cTaskCharPacman2->vTrans(), this->Model->vTrans());
				if (!m_bHit)
				{
					cAppG03PacnRollGame3->SetEffect(4u, cTaskCharPacman2->vTrans(), Vector4());
					cTaskCharPacman2->DamageProcess(1u, v2, cTaskCharPacman2->GetParam()->fBoundSpeed, 2);
					m_bHit = true;
				}
				new cKingdomPtkHnd(5u, true, true, true, 0.0f, cTaskCharPacman2->vTrans());
			}
		}
		else if (this->Model->IsVisible())
		{
			if (m_nBreakTime != 0)
			{
				m_nBreakTime--;
				::Constants::jetGeoVecScale(v, m_vDir, -0.025f);
				::Constants::jetGeoVecAdd(this->Model->vTrans(), this->Model->vTrans(), v);
				this->Model->vRotate()[0] += m_fBreakRot;
			}
			else
			{
				this->Model->Invisible();
			}
		}
	}
	if (this->Model->IsVisible())
	{
		PushBack();
	}
	return true;
}

// DOL 0x8014f180 [audited: per AUDIT_G03_G07 row]
void cTaskBgGimmickArrow::Draw(int nStatus, cJetTask* pParam)
{
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, this->Model->vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, this->Model->vRotate());
	Matrix4x4 _discard1;
	::Constants::jetGeoMtx44to34(_discard1, m);
	DummyNW4R::SetMtx(this->Model->pScnMdl(), m);
}

// inlined in Execute: squared distance <= radius^2 + 0.36
bool cTaskBgGimmickArrow::HitCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v;
	::Constants::jetGeoVecSub(v, cTaskCharPacman2->vTrans(), this->Model->vTrans());
	{ const float c[3] = {this->Model->vTrans()[0], this->Model->vTrans()[1], this->Model->vTrans()[2]}; devtools::G03HitSphere(c, std::sqrt(cTaskCharPacman2->fRadius() * cTaskCharPacman2->fRadius() + 0.36f), 0xffffffffu); } // devtools: --g03-show-gimmick-hit (not 1:1): where Pac-Man's centre is hit
	if (::Constants::jetGeoVecInnerProduct(v, v) <= cTaskCharPacman2->fRadius() * cTaskCharPacman2->fRadius() + 0.36f)
	{
		return true;
	}
	return false;
}

}
