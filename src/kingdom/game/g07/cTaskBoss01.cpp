#include "kingdom/game/g07/cTaskBoss01.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/g07/cParamBoss01.hpp"
#include "kingdom/game/g07/cParamDamage.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskMotosFade.hpp"
#include "kingdom/game/g07/cTaskPanelMan.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <Extentions.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g07
{

float cTaskBoss01::APPEAR_ADD_NEAR_FOG = 0.0f;
float cTaskBoss01::APPEAR_ADD_FAR_FOG = 0.0f;
Vector4 cTaskBoss01::s_vPos[6] = {};
int cTaskBoss01::g_HitDamage[6] = {};
const Constants::MO_BOSS_MODE_INFO* cTaskBoss01::s_ModeTbl[20] = {[]() { Constants::MO_BOSS_MODE_INFO* _o1 = new Constants::MO_BOSS_MODE_INFO(); _o1->m_nMode = 3; _o1->m_nMove = 1; _o1->m_nWait = 30; return _o1; }(), []() { Constants::MO_BOSS_MODE_INFO* _o2 = new Constants::MO_BOSS_MODE_INFO(); _o2->m_nMode = 4; _o2->m_nMove = -1; _o2->m_nWait = 5; return _o2; }(), []() { Constants::MO_BOSS_MODE_INFO* _o3 = new Constants::MO_BOSS_MODE_INFO(); _o3->m_nMode = 5; _o3->m_nMove = 2; _o3->m_nWait = 30; return _o3; }(), []() { Constants::MO_BOSS_MODE_INFO* _o4 = new Constants::MO_BOSS_MODE_INFO(); _o4->m_nMode = 6; _o4->m_nMove = -1; _o4->m_nWait = 30; _o4->m_nWork[0] = 1; _o4->m_nWork[1] = 0; _o4->m_nWork[2] = 0; _o4->m_nWork[3] = 0; return _o4; }(), []() { Constants::MO_BOSS_MODE_INFO* _o5 = new Constants::MO_BOSS_MODE_INFO(); _o5->m_nMode = 8; _o5->m_nMove = -1; _o5->m_nWait = 60; return _o5; }(), []() { Constants::MO_BOSS_MODE_INFO* _o6 = new Constants::MO_BOSS_MODE_INFO(); _o6->m_nMode = 6; _o6->m_nMove = -1; _o6->m_nWait = 30; _o6->m_nWork[0] = 1; _o6->m_nWork[1] = 0; _o6->m_nWork[2] = 0; _o6->m_nWork[3] = 0; return _o6; }(), []() { Constants::MO_BOSS_MODE_INFO* _o7 = new Constants::MO_BOSS_MODE_INFO(); _o7->m_nMode = 3; _o7->m_nMove = 3; _o7->m_nWait = 30; return _o7; }(), []() { Constants::MO_BOSS_MODE_INFO* _o8 = new Constants::MO_BOSS_MODE_INFO(); _o8->m_nMode = 6; _o8->m_nMove = -1; _o8->m_nWait = 30; return _o8; }(), []() { Constants::MO_BOSS_MODE_INFO* _o9 = new Constants::MO_BOSS_MODE_INFO(); _o9->m_nMode = 8; _o9->m_nMove = -1; _o9->m_nWait = 60; return _o9; }(), []() { Constants::MO_BOSS_MODE_INFO* _o10 = new Constants::MO_BOSS_MODE_INFO(); _o10->m_nMode = 6; _o10->m_nMove = -1; _o10->m_nWait = 30; return _o10; }(), []() { Constants::MO_BOSS_MODE_INFO* _o11 = new Constants::MO_BOSS_MODE_INFO(); _o11->m_nMode = 4; _o11->m_nMove = -1; _o11->m_nWait = 5; return _o11; }(), []() { Constants::MO_BOSS_MODE_INFO* _o12 = new Constants::MO_BOSS_MODE_INFO(); _o12->m_nMode = 5; _o12->m_nMove = 4; _o12->m_nWait = 30; return _o12; }(), []() { Constants::MO_BOSS_MODE_INFO* _o13 = new Constants::MO_BOSS_MODE_INFO(); _o13->m_nMode = 6; _o13->m_nMove = -1; _o13->m_nWait = 30; _o13->m_nWork[0] = 1; _o13->m_nWork[1] = 0; _o13->m_nWork[2] = 0; _o13->m_nWork[3] = 0; return _o13; }(), []() { Constants::MO_BOSS_MODE_INFO* _o14 = new Constants::MO_BOSS_MODE_INFO(); _o14->m_nMode = 8; _o14->m_nMove = -1; _o14->m_nWait = 60; return _o14; }(), []() { Constants::MO_BOSS_MODE_INFO* _o15 = new Constants::MO_BOSS_MODE_INFO(); _o15->m_nMode = 6; _o15->m_nMove = -1; _o15->m_nWait = 30; _o15->m_nWork[0] = 1; _o15->m_nWork[1] = 0; _o15->m_nWork[2] = 0; _o15->m_nWork[3] = 0; return _o15; }(), []() { Constants::MO_BOSS_MODE_INFO* _o16 = new Constants::MO_BOSS_MODE_INFO(); _o16->m_nMode = 3; _o16->m_nMove = 5; _o16->m_nWait = 30; return _o16; }(), []() { Constants::MO_BOSS_MODE_INFO* _o17 = new Constants::MO_BOSS_MODE_INFO(); _o17->m_nMode = 6; _o17->m_nMove = -1; _o17->m_nWait = 30; return _o17; }(), []() { Constants::MO_BOSS_MODE_INFO* _o18 = new Constants::MO_BOSS_MODE_INFO(); _o18->m_nMode = 8; _o18->m_nMove = -1; _o18->m_nWait = 60; return _o18; }(), []() { Constants::MO_BOSS_MODE_INFO* _o19 = new Constants::MO_BOSS_MODE_INFO(); _o19->m_nMode = 6; _o19->m_nMove = -1; _o19->m_nWait = 30; return _o19; }(), []() { Constants::MO_BOSS_MODE_INFO* _o20 = new Constants::MO_BOSS_MODE_INFO(); _o20->m_nMode = 7; _o20->m_nMove = 0; _o20->m_nWait = 40; return _o20; }()};

int cTaskBoss01::DAMAGE_BASE_HP(cTaskCharBoss* boss)
{
	return (int)((float)boss->nMaxHp() * 0.6f);
}

float cTaskBoss01::DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss)
{
	return 55.0f / (float)cTaskBoss01::DAMAGE_BASE_HP(boss);
}

float cTaskBoss01::DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss)
{
	return 35.0f / (float)cTaskBoss01::DAMAGE_BASE_HP(boss);
}

void cTaskBoss01::SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time)
{
	boss->nTimer() = time;
	if (boss->nTimer() != 0)
	{
		boss->SetMode(9);
		boss->ReturnMode((ushort)mode);
	}
	else
	{
		boss->SetMode(mode);
	}
}

cTaskBoss01::cTaskBoss01()
{
	Init();
}

float& cTaskBoss01::fTargetRot()
{
	return m_fTargetRot;
}

float& cTaskBoss01::fRushAccel()
{
	return m_fRushAccel;
}

float& cTaskBoss01::fRushSpeed()
{
	return m_fRushSpeed;
}

float& cTaskBoss01::fRotSpeed()
{
	return m_fRotSpeed;
}

float& cTaskBoss01::fNowAccel()
{
	return m_fNowAccel;
}

float& cTaskBoss01::fNowMaxSpeed()
{
	return m_fNowMaxSpeed;
}

Matrix4x4& cTaskBoss01::mRoll()
{
	return m_mRoll;
}

float& cTaskBoss01::fCenterPosY()
{
	return m_fCenterPosY;
}

void cTaskBoss01::Init()
{
	m_fTargetRot = 0.0f;
	m_fRushAccel = 0.0f;
	m_fRushSpeed = 0.0f;
	m_fRotSpeed = 0.0f;
	m_fNowAccel = 0.0f;
	m_fNowMaxSpeed = 0.0f;
	m_nHitFlash = 0;
	m_nAttacked = false;
	nAction() = 0;
	nMovePoint() = 0;
	fChargeDamageRate() = 0.0f;
	m_fCenterPosY = 0.0f;
	m_nCameraMode = 0u;
	SetMode(1);
	Vector4 v;
	Constants::MOTOS_VEC4_SET(v, -5.5f, 0.0f, -0.5f);
	ExHitAlloc(1);
	ExHitRegistRect(0, v, 11.0f, 1.0f);
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	Constants::VEC3_SET(s_vPos[0], cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f + 0.5f);
	Constants::VEC3_SET(s_vPos[1], s_vPos[0][0] - (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f - 6.0f), 0.0f, s_vPos[0][2]);
	Constants::VEC3_SET(s_vPos[2], s_vPos[0][0] - (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f - 6.0f), 0.0f, s_vPos[0][2] + (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f - 3.0f));
	Constants::VEC3_SET(s_vPos[3], s_vPos[0][0] + (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f - 6.0f), 0.0f, s_vPos[0][2] + (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f - 3.0f));
	Constants::VEC3_SET(s_vPos[4], s_vPos[0][0] + (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f - 6.0f), 0.0f, s_vPos[0][2] - (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f - 3.0f));
	Constants::VEC3_SET(s_vPos[5], s_vPos[0][0] - (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f - 6.0f), 0.0f, s_vPos[0][2] - (cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f - 3.0f));
	cParamBoss01* cParamBoss6 = new cParamBoss01();
	cParamBoss6->RegistData("CSV_BOSS1_PARAM");
	m_fRushAccel = cParamBoss6->GetValueF32(1, 4);
	m_fRushSpeed = cParamBoss6->GetValueF32(1, 5);
	m_fRotMaxSpeed = cParamBoss6->GetValueF32(1, 6);
	m_fRotAccel = cParamBoss6->GetValueF32(1, 7);
	nHp() = cParamBoss6->GetValueS32(1, 1);
	nBombMax() = cParamBoss6->GetValueS32(1, 2);
	nBombTime() = cParamBoss6->GetValueS32(1, 3);
	nScore() = cParamBoss6->GetValueS32(1, 8);
	nHp() *= 100;
	nMaxHp() = nHp();
	cParamDamage* cParamDamage2 = new cParamDamage("CSV_HIT_DAMAGE");
	for (int i = 0; i < 6; i++)
	{
		int num = (int)(cParamDamage2->GetValueF32(i + 1, 1) * 100.0f);
		g_HitDamage[i] = num;
	}
	fChargeDamageRate() = cParamDamage2->GetValueF32(7, 1);
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay() && KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP > 0)
	{
		nHp() = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP;
	}
	nHp();
	cTaskBoss01::DAMAGE_BASE_HP(this);
	OnFlag(1u);
	CreateBomb();
	Constants::tspVuMtxUnit(m_mRoll);
	nCoreFogColor() = 4278190335u;
	OnFlag(4194304u);
}

void cTaskBoss01::Term()
{
	cTaskCharBoss::Term();
}

bool cTaskBoss01::ActionBoth()
{
	if (GetMode() != 10 && nHp() < cTaskBoss01::DAMAGE_BASE_HP(this))
	{
		fCoreFogNear() = 30.0f - cTaskBoss01::DAMAGE_ADD_NEAR_FOG(this) * (float)(cTaskBoss01::DAMAGE_BASE_HP(this) - nHp());
		fCoreFogFar() = 60.0f - cTaskBoss01::DAMAGE_ADD_FAR_FOG(this) * (float)(cTaskBoss01::DAMAGE_BASE_HP(this) - nHp());
	}
	return true;
}

bool cTaskBoss01::ActionLocal()
{
	if (CustomKingdomUtil::IF(m_nHitFlash))
	{
		m_nHitFlash--;
		int value = m_nHitFlash % 8 / 4;
		if (!CustomKingdomUtil::IF(m_nHitFlash))
		{
			pModel()->SetFogIdx(2);
			pModel(1u)->SetFogIdx(2);
			pModel(3u)->SetFogIdx(2);
			SetMortion(1u, "ANM_CHAR_BOSS_1_A00");
			pModel(1u)->RegistAnm(0u, "ANM_CHAR_BOSS_1_A00");
			SetFlash(0, 0.0f);
			SetFlash(1, 0.0f);
			SetFlash(3, 0.0f);
		}
		else if (CustomKingdomUtil::IF(value))
		{
			pModel()->SetFogIdx(3);
			pModel(1u)->SetFogIdx(3);
			pModel(3u)->SetFogIdx(3);
			SetFlash(0, 1.0f);
			SetFlash(1, 1.0f);
			SetFlash(3, 1.0f);
		}
		else
		{
			pModel()->SetFogIdx(2);
			pModel(1u)->SetFogIdx(2);
			pModel(3u)->SetFogIdx(2);
			SetFlash(0, 0.0f);
			SetFlash(1, 0.0f);
			SetFlash(3, 0.0f);
		}
	}
	int num = ExecModeFunc();
	m_nAttacked = false;
	return num != 0;
}

bool cTaskBoss01::ActionOther()
{
	return true;
}

void cTaskBoss01::AdjustMaxSpeed()
{
	float num = ::Constants::jetGeoVecLength(vSpeed());
	if (num > fNowMaxSpeed())
	{
		num = fNowMaxSpeed() / num;
		Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
	}
}

void cTaskBoss01::DrawStartDirection()
{
}

void cTaskBoss01::DrawInGame()
{
}

void cTaskBoss01::DrawEndDirection()
{
}

bool cTaskBoss01::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (GetMode() != 0)
	{
		if (1 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				OffFlag(1u);
				OnFlag(32u);
				::Constants::jetGeoVecCopy(vMTrans(), s_vPos[0]);
				fFogNear() = -30.0f;
				fFogFar() = 40.0f;
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				ModeAppear();
				return true;
			}
		}
		else if (2 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				OffFlag(32u);
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				nAction() = 0;
				cTaskBoss01::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, s_ModeTbl[nAction()]->m_nWait);
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nMovePoint() = s_ModeTbl[nAction()]->m_nMove;
				::Constants::jetGeoVecUnit(vSpeed());
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel / 4.0f))
				{
					int nWait = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 19;
					cTaskBoss01::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait);
				}
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nTimer() = Constants::FPS_TIME(1.0f);
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				vMTrans()[1] = 0.0f;
				return true;
			}
			if (1 == nMes)
			{
				vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
				if (--nTimer() == 0)
				{
					int nWait2 = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 19;
					cTaskBoss01::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait2);
				}
				return true;
			}
		}
		else if (5 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nMovePoint() = s_ModeTbl[nAction()]->m_nMove;
				::Constants::jetGeoVecUnit(vSpeed());
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				vMTrans()[1] = 0.0f;
				return true;
			}
			if (1 == nMes)
			{
				vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.005);
				m_fNowAccel = fRushAccel();
				m_fNowMaxSpeed = fRushSpeed();
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel))
				{
					int nWait3 = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 19;
					cTaskBoss01::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait3);
				}
				return true;
			}
		}
		else if (6 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				if (CustomKingdomUtil::IF(s_ModeTbl[nAction()]->m_nWork[0]))
				{
					m_fTargetRot = vRotate()[1] + Constants::TSP_DEG2RAD(90.0f);
				}
				else
				{
					m_fTargetRot = vRotate()[1] - Constants::TSP_DEG2RAD(90.0f);
				}
				::Constants::jetGeoVecUnit(vSpeed());
				m_fRotSpeed = 0.0f;
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				if (!m_nAttacked)
				{
					m_fRotSpeed += m_fRotAccel;
					if (m_fRotSpeed > m_fRotMaxSpeed)
					{
						m_fRotSpeed = m_fRotMaxSpeed;
					}
					if (CustomKingdomUtil::IF(s_ModeTbl[nAction()]->m_nWork[0]))
					{
						vRotate()[1] += m_fRotSpeed;
					}
					else
					{
						vRotate()[1] -= m_fRotSpeed;
					}
				}
				else
				{
					m_fRotSpeed = 0.0f;
				}
				if (Constants::MOTOS_P_ABS(vRotate()[1] - m_fTargetRot) < m_fRotSpeed)
				{
					vRotate()[1] = m_fTargetRot;
					int nWait4 = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 19;
					cTaskBoss01::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait4);
				}
				return true;
			}
		}
		else if (7 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				m_fRotSpeed = 0.0f;
				nMovePoint() = s_ModeTbl[nAction()]->m_nMove;
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				if (++Step(1) % 40 == 0)
				{
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				}
				if (!m_nAttacked)
				{
					m_fRotSpeed += m_fRotAccel / 2.0f;
					if (m_fRotSpeed > m_fRotMaxSpeed)
					{
						m_fRotSpeed = m_fRotMaxSpeed;
					}
					vRotate()[1] -= m_fRotSpeed;
					while (vRotate()[1] > (float)std::numbers::pi * 2.0f)
					{
						vRotate()[1] -= (float)std::numbers::pi * 2.0f;
					}
					while (vRotate()[1] < 0.0f)
					{
						vRotate()[1] += (float)std::numbers::pi * 2.0f;
					}
				}
				else
				{
					m_fRotSpeed = 0.0f;
				}
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel / 4.0f))
				{
					if (Constants::MOTOS_P_ABS(vRotate()[1] - Constants::TSP_DEG2RAD(0.0f)) < m_fRotSpeed)
					{
						vRotate()[1] = Constants::TSP_DEG2RAD(0.0f);
					}
					if (Constants::MOTOS_P_ABS(vRotate()[1] - Constants::TSP_DEG2RAD(180.0f)) < m_fRotSpeed)
					{
						vRotate()[1] = Constants::TSP_DEG2RAD(180.0f);
					}
					if (Constants::MOTOS_P_ABS(vRotate()[1] - Constants::TSP_DEG2RAD(360.0f)) < m_fRotSpeed)
					{
						vRotate()[1] = Constants::TSP_DEG2RAD(0.0f);
					}
					if (vRotate()[1] == Constants::TSP_DEG2RAD(0.0f) || vRotate()[1] == Constants::TSP_DEG2RAD(180.0f))
					{
						int nWait5 = s_ModeTbl[nAction()]->m_nWait;
						nAction() = (nAction() + 1) % 19;
						cTaskBoss01::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait5);
					}
				}
				return true;
			}
		}
		else if (8 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				int num = nMovePoint();
				if ((uint)(num - 2) <= 1u)
				{
					SpawnBomb(1, 6.5f, 0.4f, 5.7f);
				}
				else
				{
					SpawnBomb(2, 6.5f, 0.4f, 5.7f);
				}
				int nWait6 = s_ModeTbl[nAction()]->m_nWait;
				nAction() = (nAction() + 1) % 19;
				cTaskBoss01::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait6);
				return true;
			}
		}
		else if (9 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				if (0 >= --nTimer())
				{
					SetMode(ReturnMode());
				}
				return true;
			}
		}
		else if (10 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nTimer() = 0;
				nLTimer() = 0;
				::Constants::jetGeoVecUnit(vSpeed());
				fCoreFogNear() = -25.0f;
				fCoreFogFar() = 25.0f;
				m_fCenterPosY = vMTrans()[1];
				BombAllExplode();
				SetMortionReverse(2u, "ANM_CHAR_BOSS_1_A03", 0u, 2u);
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				ModeDeath();
				return true;
			}
		}
	}
	return true;
}

void cTaskBoss01::RefAttacked(int nParts, int nGrobalID)
{
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nGrobalID);
	if (cTaskChar2 == nullptr)
	{
		return;
	}
	bool flag = true;
	if (RefHistoryOffset(1)->m_nRefID != 0 && RefHistoryOffset(1)->m_nCount > Constants::get_GAME_COUNTER_LOCAL() - 5)
	{
		flag = false;
	}
	m_nAttacked = true;
	if (nParts == 0 && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u) && (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(131072u) || KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(384u)))
	{
		m_nHitFlash = 15;
		nHp() -= HitDamage(cTaskChar2, g_HitDamage);
		if (nHp() <= 0)
		{
			SetMode(10);
		}
		if (flag)
		{
			SpawnBossColEffect_Core(cTaskChar2);
			SetMortion(1u, "ANM_CHAR_BOSS_1_A01");
		}
	}
	else if (flag)
	{
		SpawnBossColEffect(cTaskChar2);
	}
}

void cTaskBoss01::SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step)
{
	boss->Step() = step;
}

void cTaskBoss01::SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time)
{
	boss->Step() = 255;
	boss->ReturnStep(step);
	boss->nTimer() = time;
}

void cTaskBoss01::SET_STEP(cTaskCharBoss* boss, ushort step)
{
	cTaskBoss01::SET_STEP_NO_CLEAR(boss, step);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss01::SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time)
{
	cTaskBoss01::SET_STEP_WAIT_NO_CLEAR(boss, step, time);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss01::ApearCameraSet()
{
	switch (m_nCameraMode)
	{
	case 0u:
		if (vMTrans()[1] > 0.0f)
		{
			vMTrans()[1] -= 0.05f;
			KingdomGlobal::g_pMotosGame->pCamera()->vPosition()[1] -= 0.025f;
			::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vTarget(), Extentions::GetUnityPos(vMTrans()));
		}
		break;
	case 1u:
		{
			if (vMTrans()[1] > 0.0f)
			{
				float num = vMTrans()[1] / 10.0f;
				if (num > 0.05f)
				{
					num = 0.05f;
				}
				vMTrans()[1] -= num;
			}
			Vector4 v;
			::Constants::jetGeoVecCopy(v, pModel(4u)->vTrans());
			v[1] = 0.0f;
			::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vTarget(), Extentions::GetUnityPos(v));
			v[1] = 8.0f;
			v[2] += 4.0f;
			::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vPosition(), Extentions::GetUnityPos(v));
			break;
		}
	}
}

void cTaskBoss01::ModeAppear()
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) != 2 || !bTestFlag(4194304u))
	{
		if (bTestFlag(4194304u))
		{
			SetFrame(1u, 0.0f);
		}
		SetMortion(2u, "ANM_CHAR_BOSS_1_A02");
		SetMortion(2u, "ANM_CHAR_BOSS_1_A04", 0u, 2u);
		Constants::VEC3_SET(pModel(2u)->vScale(), 1.0f, 1.0f, 1.0f);
		OffFlag(4194304u);
		pModel(2u)->SetFogIdx(2);
		cTaskBoss01::SET_MODE_WAIT(this, 2, 10);
		vMTrans()[1] = 0.0f;
		SetFlash(2, 0.0f);
		return;
	}
	nApearDemoTimer()++;
	switch (Step())
	{
	case 0:
		switch (Step(1))
		{
		case 0:
			if (KingdomGlobal::g_pMotosGame->nStep(0) == 2)
			{
				Step(1)++;
				::Constants::jetGeoVecUnit(pModel(2u)->vScale());
			}
			break;
		case 1:
			Step(1)++;
			fFogNear() = -30.0f;
			fFogFar() = 40.0f;
			cTaskBoss01::SET_STEP_WAIT(this, 1, 30);
			pModel(2u)->SetFogIdx(3);
			SetFlash(2, 1.0f);
			break;
		}
		break;
	case 1:
		SpawnAppearEffect(this, Constants::VEC3_GET(-6.0f, 0.0f, -0.7f), 4.0f, 1.4f, 0.7f);
		SpawnAppearEffect(this, Constants::VEC3_GET(2.0f, 0.0f, -0.7f), 4.0f, 1.4f, 0.7f);
		fFogNear() = -30.0f;
		fFogFar() = 40.0f;
		cTaskBoss01::SET_STEP(this, 2);
		break;
	case 2:
		if (Step(1) != 8)
		{
			Step(1)++;
			pModel(2u)->vScale()[0] += 0.125f;
			pModel(2u)->vScale()[1] += 0.125f;
			pModel(2u)->vScale()[2] += 0.125f;
			break;
		}
		Constants::MOTOS_VEC4_SET(pModel(2u)->vScale(), 1.0f, 1.0f, 1.0f);
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE007_BOSS_DEATH, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		pModel(2u)->SetFogIdx(2);
		SetFlash(2, 0.0f);
		SetMortion(2u, "ANM_CHAR_BOSS_1_A03", 0u, 2u);
		if (vMTrans()[1] < 1.0f)
		{
			m_nCameraMode = 1u;
			cTaskBoss01::SET_STEP_WAIT(this, 3, 40);
		}
		break;
	case 3:
		if (Step(1) != 60)
		{
			Step(1)++;
		}
		else
		{
			cTaskBoss01::SET_STEP_WAIT(this, 4, 30);
		}
		break;
	case 4:
		if (Step(1) != 0)
		{
			(void)(1);
			break;
		}
		OffFlag(4194304u);
		Step(1)++;
		bApearDemoEnd() = true;
		cTaskBoss01::SET_MODE_WAIT(this, 2, 10);
		SetMortion(2u, "ANM_CHAR_BOSS_1_A02");
		SetMortion(2u, "ANM_CHAR_BOSS_1_A04", 0u, 2u);
		break;
	case 255:
		if (--nTimer() <= 0)
		{
			Step() = ReturnStep();
		}
		break;
	}
}

void cTaskBoss01::ModeDeath()
{
	cTaskMotosFade* cTaskMotosFade2 = KingdomGlobal::g_pMotosGame->pTaskFade();
	switch (Step())
	{
	case 0:
	_switch24_0:
		pModel()->SetUpdateRateAnmChr(0u, 0.0f);
		pModel(1u)->SetUpdateRateAnmChr(0u, 0.0f);
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		if (Step(1) < 30)
		{
			Step(1)++;
		}
		else
		{
			nLTimer() = 90;
			cTaskBoss01::SET_STEP(this, 1);
		}
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		break;
	case 1:
	_switch24_1:
		SpawnExplodeRandom(this, Constants::VEC3_GET(-5.0f, 0.0f, -0.5f), 3.5f, 1.0f, 0.5f, 240);
		SpawnExplodeRandom(this, Constants::VEC3_GET(-5.0f, 0.0f, -0.5f), 3.5f, 1.0f, 0.5f, 240);
		SpawnExplodeRandom(this, Constants::VEC3_GET(1.0f, 0.0f, -0.5f), 3.5f, 1.0f, 0.5f, 240);
		SpawnExplodeRandom(this, Constants::VEC3_GET(1.0f, 0.0f, -0.5f), 3.5f, 1.0f, 0.5f, 240);
		cTaskBoss01::SET_STEP(this, 2);
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		break;
	case 2:
	_switch24_2:
		if (nLTimer() <= 0)
		{
			cTaskBoss01::SET_STEP(this, 3);
		}
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		ExplodeSE();
		break;
	case 3:
	_switch24_3:
		switch (Step(2))
		{
		case 0:
		_switch25_0:
			if (++Step(1) > 60)
			{
				Step(1) = 0;
				Step(2)++;
			}
			ExplodeSE();
			cTaskMotosFade2->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE, 1000);
			break;
		case 1:
		_switch25_1:
			if (++Step(1) > 30)
			{
				Constants::SpawnEffect(21, vMTrans());
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.2f, 20);
				Step(1) = 0;
				Step(2)++;
			}
			goto _switch25_2;
		case 2:
		_switch25_2:
			if (++Step(1) > 10)
			{
				Constants::SpawnEffect(21, vMTrans());
				ExHitParam(0)->m_nKind = 2;
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.2f, 20);
				Step(1) = 0;
				Step(2)++;
			}
			break;
		case 3:
		_switch25_3:
			if (++Step(1) == 20)
			{
				Step(1)++;
				Constants::SpawnEffect(21, vMTrans());
				KingdomGlobal::g_pMotosGame->nScore() += nScore();
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.42f, 30);
			}
			else if (Step(1) > 20 && cTaskMotosFade2->IsEnd())
			{
				Step(1) = 0;
				Step(2)++;
			}
			break;
		case 4:
		_switch25_4:
			if (Step(1)++ > 30)
			{
				OnFlag(33u);
				cTaskMotosFade2->Fade(EFadeKind::eFADE_TYPE_IN_WHITE, 1000);
				Step(2)++;
				for (uint num = 0u; pModel(num) != nullptr; num++)
				{
					::Constants::jetGeoVecUnit(pModel(num)->vScale());
				}
			}
			break;
		case 5:
		_switch25_5:
			if (cTaskMotosFade2->IsEnd())
			{
				cTaskBoss01::SET_STEP(this, 4);
			}
			break;
		}
		m_fCenterPosY = (float)((double)Random::Range(0, 10) * 0.02);
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		break;
	case 4:
	_switch24_4:
		if (++Step(1) > 50)
		{
			ChangeTaiki();
		}
		break;
	}
	if (nLTimer() != 0 && --nLTimer() > 0)
	{
		fCoreFogNear() -= -1.0f / 18.0f;
		fCoreFogFar() -= 1.0f / 6.0f;
		uint num2 = (uint)(2 * (90 - nLTimer()));
		if (num2 > 255)
		{
			num2 = 255u;
		}
		nCoreFogColor() = 0xFF0000FFu | (num2 << 16) | (num2 << 8);
		m_fCenterPosY = (float)((double)Random::Range(0, 10) * 0.02 * (double)(1.0f - (float)(nLTimer() / 90)));
	}
}

}
