#include "kingdom/game/g07/cTaskBoss03.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/g07/cParamBoss03.hpp"
#include "kingdom/game/g07/cParamDamage.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskPanelMan.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g07
{

Vector4 cTaskBoss03::s_vPos[8] = {};
int cTaskBoss03::g_HitDamage[6] = {};
const Constants::MO_BOSS_MODE_INFO* cTaskBoss03::s_ModeTbl[20] = {[]() { Constants::MO_BOSS_MODE_INFO* _o1 = new Constants::MO_BOSS_MODE_INFO(); _o1->m_nMode = 3; _o1->m_nMove = 1; _o1->m_nWait = 10; return _o1; }(), []() { Constants::MO_BOSS_MODE_INFO* _o2 = new Constants::MO_BOSS_MODE_INFO(); _o2->m_nMode = 4; _o2->m_nMove = -1; _o2->m_nWait = 10; return _o2; }(), []() { Constants::MO_BOSS_MODE_INFO* _o3 = new Constants::MO_BOSS_MODE_INFO(); _o3->m_nMode = 5; _o3->m_nMove = 2; _o3->m_nWait = 30; return _o3; }(), []() { Constants::MO_BOSS_MODE_INFO* _o4 = new Constants::MO_BOSS_MODE_INFO(); _o4->m_nMode = 6; _o4->m_nMove = -1; _o4->m_nWait = 30; return _o4; }(), []() { Constants::MO_BOSS_MODE_INFO* _o5 = new Constants::MO_BOSS_MODE_INFO(); _o5->m_nMode = 8; _o5->m_nMove = -1; _o5->m_nWait = 20; return _o5; }(), []() { Constants::MO_BOSS_MODE_INFO* _o6 = new Constants::MO_BOSS_MODE_INFO(); _o6->m_nMode = 3; _o6->m_nMove = 3; _o6->m_nWait = 30; return _o6; }(), []() { Constants::MO_BOSS_MODE_INFO* _o7 = new Constants::MO_BOSS_MODE_INFO(); _o7->m_nMode = 6; _o7->m_nMove = -1; _o7->m_nWait = 30; return _o7; }(), []() { Constants::MO_BOSS_MODE_INFO* _o8 = new Constants::MO_BOSS_MODE_INFO(); _o8->m_nMode = 8; _o8->m_nMove = -1; _o8->m_nWait = 20; return _o8; }(), []() { Constants::MO_BOSS_MODE_INFO* _o9 = new Constants::MO_BOSS_MODE_INFO(); _o9->m_nMode = 4; _o9->m_nMove = -1; _o9->m_nWait = 10; return _o9; }(), []() { Constants::MO_BOSS_MODE_INFO* _o10 = new Constants::MO_BOSS_MODE_INFO(); _o10->m_nMode = 5; _o10->m_nMove = 4; _o10->m_nWait = 30; return _o10; }(), []() { Constants::MO_BOSS_MODE_INFO* _o11 = new Constants::MO_BOSS_MODE_INFO(); _o11->m_nMode = 6; _o11->m_nMove = -1; _o11->m_nWait = 30; _o11->m_nWork[0] = 1; _o11->m_nWork[1] = 0; _o11->m_nWork[2] = 0; _o11->m_nWork[3] = 0; return _o11; }(), []() { Constants::MO_BOSS_MODE_INFO* _o12 = new Constants::MO_BOSS_MODE_INFO(); _o12->m_nMode = 8; _o12->m_nMove = -1; _o12->m_nWait = 20; return _o12; }(), []() { Constants::MO_BOSS_MODE_INFO* _o13 = new Constants::MO_BOSS_MODE_INFO(); _o13->m_nMode = 3; _o13->m_nMove = 5; _o13->m_nWait = 30; return _o13; }(), []() { Constants::MO_BOSS_MODE_INFO* _o14 = new Constants::MO_BOSS_MODE_INFO(); _o14->m_nMode = 6; _o14->m_nMove = -1; _o14->m_nWait = 30; _o14->m_nWork[0] = 1; _o14->m_nWork[1] = 0; _o14->m_nWork[2] = 0; _o14->m_nWork[3] = 0; return _o14; }(), []() { Constants::MO_BOSS_MODE_INFO* _o15 = new Constants::MO_BOSS_MODE_INFO(); _o15->m_nMode = 8; _o15->m_nMove = -1; _o15->m_nWait = 20; return _o15; }(), []() { Constants::MO_BOSS_MODE_INFO* _o16 = new Constants::MO_BOSS_MODE_INFO(); _o16->m_nMode = 3; _o16->m_nMove = 6; _o16->m_nWait = 30; return _o16; }(), []() { Constants::MO_BOSS_MODE_INFO* _o17 = new Constants::MO_BOSS_MODE_INFO(); _o17->m_nMode = 4; _o17->m_nMove = -1; _o17->m_nWait = 10; return _o17; }(), []() { Constants::MO_BOSS_MODE_INFO* _o18 = new Constants::MO_BOSS_MODE_INFO(); _o18->m_nMode = 5; _o18->m_nMove = 7; _o18->m_nWait = 60; return _o18; }(), []() { Constants::MO_BOSS_MODE_INFO* _o19 = new Constants::MO_BOSS_MODE_INFO(); _o19->m_nMode = 7; _o19->m_nMove = 0; _o19->m_nWait = 0; _o19->m_nWork[0] = 1; _o19->m_nWork[1] = 0; _o19->m_nWork[2] = 0; _o19->m_nWork[3] = 0; return _o19; }(), []() { Constants::MO_BOSS_MODE_INFO* _o20 = new Constants::MO_BOSS_MODE_INFO(); _o20->m_nMode = 9; _o20->m_nMove = -1; _o20->m_nWait = 30; _o20->m_nWork[0] = 1; _o20->m_nWork[1] = 0; _o20->m_nWork[2] = 0; _o20->m_nWork[3] = 0; return _o20; }()};

float cTaskBoss03::get_APPEAR_ADD_NEAR_FOG()
{
	return 0.0f;
}

float cTaskBoss03::get_APPEAR_ADD_FAR_FOG()
{
	return 0.0f;
}

float cTaskBoss03::DAMAGE_BASE_HP(cTaskCharBoss* boss)
{
	return (int)((float)boss->nMaxHp() * 0.6f);
}

float cTaskBoss03::DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss)
{
	return 55.0f / cTaskBoss03::DAMAGE_BASE_HP(boss);
}

float cTaskBoss03::DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss)
{
	return 35.0f / cTaskBoss03::DAMAGE_BASE_HP(boss);
}

void cTaskBoss03::SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time)
{
	boss->nTimer() = time;
	if (boss->nTimer() != 0)
	{
		boss->SetMode(10);
		boss->ReturnMode((ushort)mode);
	}
	else
	{
		boss->SetMode(mode);
	}
}

cTaskBoss03::cTaskBoss03()
{
	Init();
}

float& cTaskBoss03::fTargetRot()
{
	return m_fTargetRot;
}

float& cTaskBoss03::fRushAccel()
{
	return m_fRushAccel;
}

float& cTaskBoss03::fRushSpeed()
{
	return m_fRushSpeed;
}

float& cTaskBoss03::fRotSpeed()
{
	return m_fRushSpeed;
}

float& cTaskBoss03::fNowAccel()
{
	return m_fNowAccel;
}

float& cTaskBoss03::fNowMaxSpeed()
{
	return m_fNowMaxSpeed;
}

Matrix4x4& cTaskBoss03::mRoll()
{
	return m_mRoll;
}

float& cTaskBoss03::fCenterPosY()
{
	return m_fCenterPosY;
}

void cTaskBoss03::SetApearScale(float x, float y, float z)
{
	Vector4 v = Vector4(x, y, z, 1.0f);
	::Constants::jetGeoVecCopy(pModel(2u)->vScale(), v);
}

void cTaskBoss03::Init()
{
	m_fTargetRot = 0.0f;
	m_fRushAccel = 0.0f;
	m_fRushSpeed = 0.0f;
	m_fAccelRotSpeed = 0.0f;
	m_fRotSpeed = 0.0f;
	m_fRotMaxSpeed = 0.0f;
	m_fRotAccel = 0.0f;
	m_fNowAccel = 0.0f;
	m_fNowMaxSpeed = 0.0f;
	m_nHitFlash = 0;
	nAction() = 0;
	nMovePoint() = 0;
	m_nAttacked = false;
	fChargeDamageRate() = 0.0f;
	m_fAccelRotSpeed = 0.0f;
	m_fMaxAccelRotSpeed = 0.0f;
	m_fMinAccelRotSpeed = 0.0f;
	m_nAccelRotTime = 0;
	m_nSlowDownRotTime = 0;
	m_nStopRotTime = 0;
	m_fCenterPosY = 0.0f;
	SetMode(1);
	ExHitAlloc(2);
	ExHitRegistRect(0, Constants::VEC3_GET(-5.5f, 0.0f, -0.5f), 11.0f, 1.0f);
	ExHitRegistRect(1, Constants::VEC3_GET(-0.5f, 0.0f, -5.5f), 1.0f, 11.0f);
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	Constants::VEC3_SET(s_vPos[0], cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f + 0.5f);
	Constants::VEC3_SET(s_vPos[1], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() / 2 - 2), 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 - 1));
	Constants::VEC3_SET(s_vPos[2], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() - 2), 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2) - 1.0f);
	Constants::VEC3_SET(s_vPos[3], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() - 2), 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 + 2));
	Constants::VEC3_SET(s_vPos[4], cTaskPanelMan2->fPanelSize() * 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 + 2));
	Constants::VEC3_SET(s_vPos[5], cTaskPanelMan2->fPanelSize() * 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 - 1));
	Constants::VEC3_SET(s_vPos[6], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() / 2 - 2), 0.0f, cTaskPanelMan2->fPanelSize() * 3.0f);
	Constants::VEC3_SET(s_vPos[7], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() / 2 + 2), 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() - 2));
	cParamBoss03* cParamBoss6 = new cParamBoss03("CSV_BOSS3_PARAM");
	m_fRushAccel = cParamBoss6->GetValueF32(1, 4);
	m_fRushSpeed = cParamBoss6->GetValueF32(1, 5);
	m_fRotMaxSpeed = cParamBoss6->GetValueF32(1, 6);
	m_fMaxAccelRotSpeed = cParamBoss6->GetValueF32(1, 8);
	m_fMinAccelRotSpeed = cParamBoss6->GetValueF32(1, 9);
	m_nAccelRotTime = (int)cParamBoss6->GetValueF32(1, 10);
	m_nSlowDownRotTime = (int)cParamBoss6->GetValueF32(1, 11);
	m_nStopRotTime = (int)cParamBoss6->GetValueF32(1, 12);
	m_fRotAccel = cParamBoss6->GetValueF32(1, 7);
	nHp() = (int)cParamBoss6->GetValueF32(1, 1);
	nHp() *= 100;
	nBombMax() = (int)cParamBoss6->GetValueF32(1, 2);
	nBombTime() = (int)cParamBoss6->GetValueF32(1, 3);
	nScore() = (int)cParamBoss6->GetValueF32(1, 13);
	m_fRotSpeed = 0.0f;
	m_fRotSpeedAccel = 0.003f;
	nMaxHp() = nHp();
	cParamDamage* cParamDamage2 = new cParamDamage("CSV_HIT_DAMAGE");
	for (int i = 0; i < 6; i++)
	{
		g_HitDamage[i] = (int)(cParamDamage2->GetValueF32(i + 1, 3) * 100.0f);
	}
	fChargeDamageRate() = cParamDamage2->GetValueF32(7, 3);
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay() && KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP > 0)
	{
		nHp() = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP;
	}
	if ((float)nHp() < cTaskBoss03::DAMAGE_BASE_HP(this))
	{
		fCoreFogNear() = 30.0f - cTaskBoss03::DAMAGE_ADD_NEAR_FOG(this) * (cTaskBoss03::DAMAGE_BASE_HP(this) - (float)nHp());
		fCoreFogFar() = 60.0f - cTaskBoss03::DAMAGE_ADD_FAR_FOG(this) * (cTaskBoss03::DAMAGE_BASE_HP(this) - (float)nHp());
	}
	CreateBomb();
	Constants::tspVuMtxUnit(m_mRoll);
	nCoreFogColor() = 4278190335u;
	OnFlag(4194304u);
}

void cTaskBoss03::Term()
{
	cTaskCharBoss::Term();
}

bool cTaskBoss03::ActionBoth()
{
	if (GetMode() != 11 && (float)nHp() < cTaskBoss03::DAMAGE_BASE_HP(this))
	{
		fCoreFogNear() = 30.0f - cTaskBoss03::DAMAGE_ADD_NEAR_FOG(this) * (cTaskBoss03::DAMAGE_BASE_HP(this) - (float)nHp());
		fCoreFogFar() = 60.0f - cTaskBoss03::DAMAGE_ADD_FAR_FOG(this) * (cTaskBoss03::DAMAGE_BASE_HP(this) - (float)nHp());
	}
	return true;
}

bool cTaskBoss03::ActionLocal()
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
			SetMortion(1u, "ANM_CHAR_BOSS_3_A00");
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
	bool result = ExecModeFunc() != 0;
	m_nAttacked = false;
	return result;
}

bool cTaskBoss03::ActionOther()
{
	return true;
}

void cTaskBoss03::AdjustMaxSpeed()
{
	float num = ::Constants::jetGeoVecLength(vSpeed());
	if (num > fNowMaxSpeed())
	{
		num = fNowMaxSpeed() / num;
		Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
	}
}

bool cTaskBoss03::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (GetMode() != 0)
	{
		if (1 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				Step() = 0;
				::Constants::jetGeoVecUnit(vSpeed());
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				OffFlag(1u);
				OnFlag(32u);
				::Constants::jetGeoVecCopy(vMTrans(), s_vPos[0]);
				fFogNear() = -30.0f;
				fFogFar() = 30.0f;
				vRotate()[1] = Constants::TSP_DEG2RAD(45.0f);
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
				nMovePoint() = 0;
				m_fRotSpeed = 0.0f;
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
			if (3 == nMes)
			{
				m_fRotSpeed = 0.0f;
				return true;
			}
			if (1 == nMes)
			{
				Vector4 _out;
				Constants::VEC3_SET(_out, s_vPos[0][0], s_vPos[0][1], s_vPos[1][2]);
				if (!m_nAttacked)
				{
					m_fRotSpeed += m_fRotAccel;
					if (Step(1) == 0)
					{
						vRotate()[1] += m_fRotSpeed;
						float value;
						if ((value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(0.0f), m_fRotSpeed)) != -1.0f || (value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(90.0f), m_fRotSpeed)) != -1.0f || (value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(180.0f), m_fRotSpeed)) != -1.0f || (value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(270.0f), m_fRotSpeed)) != -1.0f)
						{
							vRotate()[1] = value;
							Step(1)++;
						}
					}
				}
				else
				{
					m_fRotSpeed = 0.0f;
				}
				if (Constants::motosCharMove(this, _out))
				{
					nAction() = 0;
					if (Step(1) != 0)
					{
						cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, s_ModeTbl[nAction()]->m_nWait);
					}
				}
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
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
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
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel))
				{
					int nWait = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 20;
					cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait);
				}
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				nTimer() = Constants::FPS_TIME(1.0f);
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (3 == nMes)
			{
				vMTrans()[1] = 0.0f;
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
				if (--nTimer() == 0)
				{
					int nWait2 = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 20;
					cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait2);
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
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
			if (3 == nMes)
			{
				vMTrans()[1] = 0.0f;
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
				m_fNowAccel = fRushAccel();
				m_fNowMaxSpeed = fRushSpeed();
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel))
				{
					int nWait3 = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 20;
					cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait3);
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
					nAction() = (nAction() + 1) % 20;
					cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait4);
				}
				return true;
			}
		}
		else if (9 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				if (CustomKingdomUtil::IF(s_ModeTbl[nAction()]->m_nWork[0]))
				{
					m_fAccelRotSpeed = m_fMinAccelRotSpeed;
				}
				else
				{
					m_fAccelRotSpeed = 0.0f - m_fMinAccelRotSpeed;
				}
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				nLTimer() = 40;
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				if (Step() != 3 && --nLTimer() <= 0)
				{
					nLTimer() = (int)(40.0f - 36.0f / Constants::fabs(m_fMaxAccelRotSpeed) * Constants::fabs(m_fAccelRotSpeed));
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				}
				switch (Step())
				{
				case 0:
				_switch21_0:
					ExHitParam(0)->m_nFlag |= 16;
					ExHitParam(1)->m_nFlag |= 16;
					nTimer() = m_nAccelRotTime;
					Step()++;
					goto _switch21_1;
				case 1:
				_switch21_1:
					if (--nTimer() > 0)
					{
						m_fAccelRotSpeed = m_fMinAccelRotSpeed + (m_fMaxAccelRotSpeed - m_fMinAccelRotSpeed) / (float)m_nAccelRotTime * (float)(m_nAccelRotTime - nTimer());
						if (m_fAccelRotSpeed > m_fMaxAccelRotSpeed)
						{
							m_fAccelRotSpeed = m_fMaxAccelRotSpeed;
						}
					}
					else
					{
						m_fAccelRotSpeed = m_fMaxAccelRotSpeed;
						nTimer() = m_nSlowDownRotTime;
						Step()++;
					}
					break;
				case 2:
				_switch21_2:
					if (--nTimer() > 0)
					{
						m_fAccelRotSpeed = m_fMaxAccelRotSpeed / (float)m_nSlowDownRotTime * (float)nTimer();
						if (m_fAccelRotSpeed < m_fMinAccelRotSpeed)
						{
							m_fAccelRotSpeed = m_fMinAccelRotSpeed;
						}
					}
					else
					{
						int num = -17;
						ExHitParam(0)->m_nFlag &= (ushort)num;
						ExHitParam(1)->m_nFlag &= (ushort)num;
						nTimer() = m_nStopRotTime;
						Step(1) = 0;
						Step()++;
					}
					break;
				case 3:
				_switch21_3:
					{
						float value2;
						if (Step(1) == 0 && ((value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(0.0f), m_fAccelRotSpeed)) != -1.0f || (value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(90.0f), m_fAccelRotSpeed)) != -1.0f || (value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(180.0f), m_fAccelRotSpeed)) != -1.0f || (value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(270.0f), m_fAccelRotSpeed)) != -1.0f))
						{
							vRotate()[1] = value2;
							Step(1)++;
							m_fAccelRotSpeed = 0.0f;
						}
						if (--nTimer() < 0 && Step(1) != 0)
						{
							int nWait5 = s_ModeTbl[nAction()]->m_nWait;
							nAction() = (nAction() + 1) % 20;
							cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait5);
						}
						break;
					}
				}
				if (CustomKingdomUtil::IF(s_ModeTbl[nAction()]->m_nWork[0]))
				{
					vRotate()[1] += m_fAccelRotSpeed;
				}
				else
				{
					vRotate()[1] -= m_fAccelRotSpeed;
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
				nMovePoint() = s_ModeTbl[nAction()]->m_nMove;
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
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
				if (CustomKingdomUtil::IF(s_ModeTbl[nAction()]->m_nWork[0]))
				{
					vRotate()[1] += m_fMinAccelRotSpeed;
				}
				else
				{
					vRotate()[1] -= m_fMinAccelRotSpeed;
				}
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel))
				{
					int nWait6 = s_ModeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 20;
					cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait6);
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
				switch (nMovePoint())
				{
				case 2:
				case 3:
					SpawnBomb(4, 6.5f, 0.5f, 5.7f);
					cTaskBoss03::SET_MODE_WAIT(this, 3, 30);
					break;
				case 4:
				case 5:
					SpawnBomb(8, 6.5f, 0.5f, 5.7f);
					cTaskBoss03::SET_MODE_WAIT(this, 3, 30);
					break;
				}
				int nWait7 = s_ModeTbl[nAction()]->m_nWait;
				nAction() = (nAction() + 1) % 20;
				cTaskBoss03::SET_MODE_WAIT(this, s_ModeTbl[nAction()]->m_nMode, nWait7);
				return true;
			}
		}
		else if (10 == GetMode())
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
		else if (11 == GetMode())
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
				SetMortionReverse(2u, "ANM_CHAR_BOSS_3_A03", 0u, 2u);
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

void cTaskBoss03::RefAttacked(int nParts, int nGrobalID)
{
	bool flag = true;
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nGrobalID);
	if (cTaskChar2 == nullptr)
	{
		return;
	}
	if (RefHistoryOffset(1)->m_nRefID != 0 && GetMode() != 9 && RefHistoryOffset(1)->m_nCount > Constants::get_GAME_COUNTER_LOCAL() - 5)
	{
		flag = false;
	}
	m_nAttacked = true;
	if (nParts == 0)
	{
		if (!CustomKingdomUtil::IF(ExHitParam(0)->m_nFlag & 0x10) && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u) && (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(131072u) || KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(384u)))
		{
			m_nHitFlash = 15;
			nHp() -= HitDamage(cTaskChar2, g_HitDamage);
			if (nHp() <= 0)
			{
				SetMode(11);
			}
			else
			{
				SetMortion(1u, "ANM_CHAR_BOSS_3_A01");
			}
			if (flag)
			{
				SpawnBossColEffect_Core(cTaskChar2);
			}
		}
	}
	else if (flag)
	{
		SpawnBossColEffect(cTaskChar2);
	}
}

void cTaskBoss03::SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step)
{
	boss->Step() = step;
}

void cTaskBoss03::SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time)
{
	boss->Step() = 5;
	boss->ReturnStep(step);
	boss->nTimer() = time;
}

void cTaskBoss03::SET_STEP(cTaskCharBoss* boss, ushort step)
{
	cTaskBoss03::SET_STEP_NO_CLEAR(boss, step);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss03::SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time)
{
	cTaskBoss03::SET_STEP_WAIT_NO_CLEAR(boss, step, time);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss03::ModeAppear()
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) != 2 || !bTestFlag(4194304u))
	{
		if (bTestFlag(4194304u))
		{
			SetFrame(0u, 0.0f);
		}
		SetApearScale(1.0f, 1.0f, 1.0f);
		OffFlag(4194304u);
		InitFogData();
		pModel(2u)->SetFogIdx(2);
		SetFlash(2, 0.0f);
		SetMortion(2u, "ANM_CHAR_BOSS_3_A02");
		SetMortion(2u, "ANM_CHAR_BOSS_3_A04", 0u, 2u);
		cTaskBoss03::SET_MODE_WAIT(this, 2, 10);
		return;
	}
	switch (Step())
	{
	case 0:
		::Constants::jetGeoVecUnit(m_vApearScale);
		SetApearScale(m_vApearScale[0], m_vApearScale[1], m_vApearScale[2]);
		pModel(2u)->SetFogIdx(3);
		SetFlash(2, 1.0f);
		switch (Step(1))
		{
		case 0:
			if (KingdomGlobal::g_pMotosGame->nStep(0) == 2)
			{
				Step(1)++;
			}
			break;
		case 1:
			fFogNear() = -30.0f;
			fFogFar() = 30.0f;
			cTaskBoss03::SET_STEP_WAIT(this, 1, 30);
			break;
		}
		break;
	case 1:
		SpawnAppearEffect(this, Constants::VEC3_GET(-5.0f, 0.0f, -0.7f), 4.0f, 1.4f, 0.8f);
		SpawnAppearEffect(this, Constants::VEC3_GET(3.0f, 0.0f, -0.7f), 4.0f, 1.4f, 0.8f);
		SpawnAppearEffect(this, Constants::VEC3_GET(0.3f, 0.0f, -6.0f), 1.4f, 4.0f, 0.8f);
		SpawnAppearEffect(this, Constants::VEC3_GET(0.3f, 0.0f, 2.0f), 1.4f, 4.0f, 0.8f);
		fFogNear() = -30.0f;
		fFogFar() = 30.0f;
		cTaskBoss03::SET_STEP(this, 2);
		break;
	case 2:
		if (Step(1) != 8)
		{
			Step(1)++;
			m_vApearScale[0] += 0.125f;
			m_vApearScale[1] += 0.125f;
			m_vApearScale[2] += 0.125f;
			SetApearScale(m_vApearScale[0], m_vApearScale[1], m_vApearScale[2]);
		}
		else
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE038_BOSS_APPEAR2, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
			cTaskBoss03::SET_STEP_WAIT(this, 3, 40);
			SetMortion(2u, "ANM_CHAR_BOSS_3_A03");
		}
		break;
	case 3:
		if (Step(1) != 60)
		{
			Step(1)++;
			float rate = 1.0f - (float)(int)Step(1) / 60.0f;
			SetFlash(2, rate);
		}
		else
		{
			SetApearScale(1.0f, 1.0f, 1.0f);
			cTaskBoss03::SET_STEP_WAIT(this, 4, 30);
			InitFogData();
			pModel(2u)->SetFogIdx(2);
			SetFlash(2, 0.0f);
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
		SetMortion(2u, "ANM_CHAR_BOSS_3_A04", 0u, 2u);
		break;
	case 5:
		if (--nTimer() <= 0)
		{
			Step() = ReturnStep();
		}
		break;
	}
}

void cTaskBoss03::ModeDeath()
{
	cTaskFade* cTaskFade2 = KingdomGlobal::g_pMotosGame->pTaskFade();
	switch (Step())
	{
	case 0:
		if (Step(1) < 30)
		{
			Step(1)++;
		}
		else
		{
			nLTimer() = 90;
			cTaskBoss03::SET_STEP(this, 1);
		}
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE024_BOSS_ROT1);
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1);
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		break;
	case 1:
		SpawnExplodeRandom(this, Constants::VEC3_GET(-5.0f, 0.0f, -0.5f), 3.5f, 1.0f, 0.5f, 240);
		SpawnExplodeRandom(this, Constants::VEC3_GET(1.0f, 0.0f, -0.5f), 3.5f, 1.0f, 0.5f, 240);
		SpawnExplodeRandom(this, Constants::VEC3_GET(-0.5f, 0.0f, -5.0f), 1.0f, 3.5f, 0.5f, 240);
		SpawnExplodeRandom(this, Constants::VEC3_GET(-0.5f, 0.0f, 1.0f), 1.0f, 3.5f, 0.5f, 240);
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		cTaskBoss03::SET_STEP(this, 2);
		break;
	case 2:
		if (nLTimer() <= 0)
		{
			cTaskBoss03::SET_STEP(this, 3);
		}
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		ExplodeSE();
		break;
	case 3:
		switch (Step(2))
		{
		case 0:
			if (++Step(1) > 60)
			{
				Step(1) = 0;
				Step(2)++;
				cTaskFade2->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE, 1000);
			}
			ExplodeSE();
			break;
		case 1:
			if (++Step(1) > 30)
			{
				Constants::SpawnEffect(21, vMTrans());
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.2f, 20);
				Step(1) = 0;
				Step(2)++;
				KingdomGlobal::g_pMotosGame->nScore() += nScore();
			}
			break;
		case 2:
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
			if (++Step(1) == 20)
			{
				Constants::SpawnEffect(21, vMTrans());
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.42f, 30);
				OnFlag(33u);
				Step(1)++;
			}
			else if (Step(1) > 20 && cTaskFade2->IsEnd())
			{
				Step(1) = 0;
				Step(2)++;
			}
			break;
		case 4:
			if (Step(1)++ > 30)
			{
				OnFlag(33u);
				cTaskFade2->Fade(EFadeKind::eFADE_TYPE_IN_WHITE, 1000);
				Step(2)++;
				for (uint num = 0u; pModel(num) != nullptr; num++)
				{
					::Constants::jetGeoVecUnit(pModel(num)->vScale());
				}
			}
			break;
		case 5:
			if (cTaskFade2->IsEnd())
			{
				cTaskBoss03::SET_STEP(this, 4);
			}
			break;
		}
		m_fCenterPosY = (float)((double)Random::Range(0, 10) * 0.02);
		vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
		break;
	case 4:
		if (++Step(1) > 50)
		{
			ChangeTaiki();
		}
		break;
	}
	if (nLTimer() != 0 && --nLTimer() > 0)
	{
		fCoreFogNear() -= -1.0f / 18.0f;
		fCoreFogFar() -= 1.0f / 18.0f;
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
