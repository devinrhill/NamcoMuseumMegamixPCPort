#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
#include "kingdom/game/g03/cTaskBgGimmickFall.hpp"
#include "kingdom/MslRand.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "ResNode.hpp"
#include "ResUserData.hpp"
#include "ResUserDataItem.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

bool& cTaskBgGimmickFall::IsBoot()
{
	return m_bBoot;
}

bool& cTaskBgGimmickFall::IsReturn()
{
	return m_bReturn;
}

// DOL 0x8014f4c8 [audited: per AUDIT_G03_G07 row] (user data ShakeTime +0x52c, FallTime +0x530, ShakeFallSpeed -|v| +0x534, FallAccel -|v| +0x538)
cTaskBgGimmickFall::cTaskBgGimmickFall(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickFall");
	Init();
	ResUserData* resUserData = node1->GetResUserData();
	if (!resUserData->IsValid())
	{
		return;
	}
	for (int i = 0; i < resUserData->GetItemNumEntries(); i++)
	{
		ResUserDataItem* item = resUserData->GetItem(i);
		if (item->IsValid())
		{
			if (CustomKingdomUtil::strstr(item->GetName(), "ShakeTime"))
			{
				m_nShakeTime = (uint)item->GetS32();
			}
			else if (CustomKingdomUtil::strstr(item->GetName(), "FallTime"))
			{
				m_nFallTime = (uint)item->GetS32();
			}
			else if (CustomKingdomUtil::strstr(item->GetName(), "ShakeFallSpeed"))
			{
				m_fShakeFall = 0.0f - ::Constants::fabs(item->GetF32());
			}
			else if (CustomKingdomUtil::strstr(item->GetName(), "FallAccel"))
			{
				m_fFallAccel = 0.0f - ::Constants::fabs(item->GetF32());
			}
		}
	}
}

void cTaskBgGimmickFall::Init()
{
	m_bBoot = false;
	m_bReturn = true;
	m_nTimeCnt = 0u;
	m_fMoveY = 0.0f;
	m_nShakeTime = 0u;
	m_nFallTime = 0u;
	m_fShakeFall = 0.0f;
	m_fFallAccel = 0.0f;
	::Constants::jetGeoVecCopy(m_vDefaultTrans, this->Model->vTrans());
	OnState(32u);
}

void cTaskBgGimmickFall::Term()
{
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE056_YUKA_YURE);
	cTaskBgGimmick::Term();
}

// DOL 0x8014fd1c [audited: per AUDIT_G03_G07 row] (modes: 0 wait for the step-on, 1 shake (x / z jitter (rand() % 200 - 100) / 1000), 2 fall (accel, max -2.0), 3 return (20 frames, blinking on frame % 4 == 1), 4 gone)
bool cTaskBgGimmickFall::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE() || Constants::PACN_APP()->IsNakajimaPause(this))
	{
		return true;
	}
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
			if (m_bBoot)
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
			m_nTimeCnt = m_nShakeTime;
			m_fMoveY = m_fShakeFall;
			if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE056_YUKA_YURE));
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			if (m_nTimeCnt != 0)
			{
				m_nTimeCnt--;
				this->Model->vTrans()[1] += m_fMoveY;
				this->Model->vTrans()[0] += (float)(kingdom::MslRandRange(0, 200) - 100) / 1000.0f;
				this->Model->vTrans()[2] += (float)(kingdom::MslRandRange(0, 200) - 100) / 1000.0f;
			}
			else
			{
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
			SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE056_YUKA_YURE);
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
			nModeStep(n) = 1u;
			m_nTimeCnt = m_nFallTime;
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			if (m_nTimeCnt != 0)
			{
				m_nTimeCnt--;
				m_fMoveY += m_fFallAccel;
				if (m_fMoveY < -2.0f)
				{
					m_fMoveY = -2.0f;
				}
				this->Model->vTrans()[1] += m_fMoveY;
			}
			else if (m_bReturn)
			{
				NextMode(n, 3u);
			}
			else
			{
				NextMode(n, 4u);
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
			IsCollision() = false;
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
			IsCollision() = true;
			m_nTimeCnt = 20u;
			::Constants::jetGeoVecCopy(this->Model->vTrans(), m_vDefaultTrans);
			goto _switch4_1u;
		case 1u:
		_switch4_1u:
			if (m_nTimeCnt != 0)
			{
				if (m_nTimeCnt % 4 != 1)
				{
					this->Model->Invisible();
				}
				else
				{
					this->Model->Visible();
				}
				m_nTimeCnt--;
			}
			else
			{
				NextMode(n, 0u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch4_2u;
		case 2u:
		_switch4_2u:
			nModeStep(n) = 0u;
			this->Model->Visible();
			m_bBoot = false;
			break;
		}
	}
	else if (4 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch5_0u:
			nModeStep(n) = 1u;
			this->Model->Invisible();
			IsCollision() = false;
			goto _switch5_1u;
		case 1u:
		_switch5_1u:
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch5_2u;
		case 2u:
		_switch5_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	return cTaskBgGimmick::Execute(pParam);
}

void cTaskBgGimmickFall::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

void cTaskBgGimmickFall::HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
	m_bBoot = true;
}

}
