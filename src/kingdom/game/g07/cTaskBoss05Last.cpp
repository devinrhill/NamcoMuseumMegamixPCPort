#include "kingdom/game/g07/cTaskBoss05Last.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/g07/cParamBoss05Last.hpp"
#include "kingdom/game/g07/cParamDamage.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "kingdom/game/g07/cTaskPanelMan.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g07
{

Vector4 cTaskBoss05Last::s_vPos[1] = {};
int cTaskBoss05Last::g_HitDamage[6] = {};
Constants::MO_BOSS_MODE_INFO* cTaskBoss05Last::s_MdeTbl[4] = {[]() { Constants::MO_BOSS_MODE_INFO* _o1 = new Constants::MO_BOSS_MODE_INFO(); _o1->m_nMode = 4; _o1->m_nMove = -1; _o1->m_nWait = 20; return _o1; }(), []() { Constants::MO_BOSS_MODE_INFO* _o2 = new Constants::MO_BOSS_MODE_INFO(); _o2->m_nMode = 3; _o2->m_nMove = -1; _o2->m_nWait = 1; return _o2; }(), []() { Constants::MO_BOSS_MODE_INFO* _o3 = new Constants::MO_BOSS_MODE_INFO(); _o3->m_nMode = 5; _o3->m_nMove = 0; _o3->m_nWait = 5; return _o3; }(), []() { Constants::MO_BOSS_MODE_INFO* _o4 = new Constants::MO_BOSS_MODE_INFO(); _o4->m_nMode = 6; _o4->m_nMove = -1; _o4->m_nWait = 15; return _o4; }()};

float cTaskBoss05Last::DAMAGE_BASE_HP(cTaskCharBoss* boss)
{
	return (int)((float)boss->nMaxHp() * 0.6f);
}

float cTaskBoss05Last::DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss)
{
	return 60.0f / cTaskBoss05Last::DAMAGE_BASE_HP(boss);
}

float cTaskBoss05Last::DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss)
{
	return 30.0f / cTaskBoss05Last::DAMAGE_BASE_HP(boss);
}

void cTaskBoss05Last::SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time)
{
	boss->nTimer() = time;
	if (boss->nTimer() != 0)
	{
		boss->SetMode(8);
		boss->ReturnMode((ushort)mode);
	}
	else
	{
		boss->SetMode(mode);
	}
}

void cTaskBoss05Last::SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step)
{
	boss->Step() = step;
}

void cTaskBoss05Last::SET_STEP(cTaskCharBoss* boss, ushort step)
{
	cTaskBoss05Last::SET_STEP_NO_CLEAR(boss, step);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

cTaskBoss05Last::cTaskBoss05Last()
	
{
	Init();
}

Vector4& cTaskBoss05Last::vRandTarget()
{
	return m_vRandTarget;
}

float& cTaskBoss05Last::fRushAccel()
{
	return m_fRushAccel;
}

float& cTaskBoss05Last::fRushSpeed()
{
	return m_fRushSpeed;
}

float& cTaskBoss05Last::fRotSpeed()
{
	return m_fRushSpeed;
}

float& cTaskBoss05Last::fNowAccel()
{
	return m_fNowAccel;
}

float& cTaskBoss05Last::fNowMaxSpeed()
{
	return m_fNowMaxSpeed;
}

bool& cTaskBoss05Last::IsTakeOver()
{
	return m_IsTakeOver;
}

void cTaskBoss05Last::Init()
{
	m_fNowAccel = 0.0f;
	m_fNowMaxSpeed = 0.0f;
	m_fRushAccel = 0.0f;
	m_fRushSpeed = 0.0f;
	m_nFrameCnt = 0;
	m_nHitFlash = 0;
	nMovePoint() = 0;
	m_fSlopeMaxRad = 0.0f;
	m_fSlopeAccel = 0.0f;
	std::fill(std::begin(m_fRotSpeed), std::end(m_fRotSpeed), 0.0f);
	m_nHomingFrame = 0;
	m_nRandomFrame = 0;
	m_nStopFrame = 0;
	nChangeMoveFrame() = 8;
	m_IsTakeOver = false;
	fChargeDamageRate() = 0.0f;
	m_fScaleAcsel = 0.0f;
	m_fGoScale = 0.0f;
	m_fUpScale = 0.0f;
	m_fTopAlpha = 0.0f;
	fFogNear() = 0.0f;
	fFogFar() = 0.0f;
	nCoreFogColor() = 4278190335u;
	m_nAction = 0;
	m_nActionNum = 0;
	::Constants::jetGeoVecUnit(m_vRandTarget);
	OnFlag(2048u);
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	Constants::VEC3_SET(s_vPos[0], cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f + 0.5f);
	cParamBoss05Last* cParamBoss05Last2 = new cParamBoss05Last("CSV_BOSS5_2_PARAM");
	m_fRotSpeed[0] = cParamBoss05Last2->GetValueF32(1, 4);
	m_fRotSpeed[1] = cParamBoss05Last2->GetValueF32(1, 5);
	m_nHomingFrame = cParamBoss05Last2->GetValueS32(1, 6);
	m_nRandomFrame = cParamBoss05Last2->GetValueS32(1, 7);
	m_nStopFrame = cParamBoss05Last2->GetValueS32(1, 8);
	m_fSlopeMaxRad = Constants::TSP_DEG2RAD(cParamBoss05Last2->GetValueF32(1, 9));
	nBombMax() = cParamBoss05Last2->GetValueS32(1, 2);
	nBombTime() = cParamBoss05Last2->GetValueS32(1, 3);
	nHp() = cParamBoss05Last2->GetValueS32(1, 1);
	nHp() *= 100;
	nScore() = cParamBoss05Last2->GetValueS32(1, 10);
	nMaxHp() = nHp();
	cParamDamage* cParamDamage2 = new cParamDamage("CSV_HIT_DAMAGE");
	for (int i = 0; i < 6; i++)
	{
		g_HitDamage[i] = (int)(cParamDamage2->GetValueF32(i + 1, 6) * 100.0f);
	}
	fChargeDamageRate() = cParamDamage2->GetValueF32(7, 6);
	nKind() = 26u;
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay() && KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHPLast > 0)
	{
		nHp() = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHPLast;
	}
	else
	{
		KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHPLast = (short)nMaxHp();
	}
	(void)((float)nHp());
	cTaskBoss05Last::DAMAGE_BASE_HP(this);
	CreateBomb();
	KingdomGlobal::g_pMotosGame->pBoss() = this;
	::Constants::jetGeoVecUnit(vRotate());
	ForceMode(1);
}

void cTaskBoss05Last::Term()
{
	cTaskCharBoss::Term();
}

bool cTaskBoss05Last::FallMove()
{
	::Constants::jetGeoVecCopy(vMTransOLD(), vMTrans());
	switch (Step())
	{
	case 0:
		vSpeed()[0] = 0.0f;
		vSpeed()[2] = 0.0f;
		Step()++;
		Step(1) = 0;
		vSpeed()[1] = 0.0f;
		break;
	case 1:
		{
			Vector4 v;
			::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->pGravVector());
			Constants::VEC3_MUL(v, v, 0.01f);
			Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
			if (vRotate()[1] > (float)std::numbers::pi)
			{
				vRotate()[1] += 0.1f;
			}
			else
			{
				vRotate()[1] -= 0.1f;
			}
			break;
		}
	}
	Constants::VEC3_ADD(vMTrans(), vMTrans(), vSpeed());
	if (Constants::IS_MYCHAR(this) && vMTrans()[1] < -40.0f)
	{
		vMTrans()[2] = 50.0f;
	}
	return false;
}

bool cTaskBoss05Last::ActionBoth()
{
	SlantFloor();
	if (GetMode() != 7)
	{
		(void)((float)nHp());
		cTaskBoss05Last::DAMAGE_BASE_HP(this);
	}
	if (IsFall())
	{
		ChangeFallMove();
	}
	return true;
}

bool cTaskBoss05Last::ActionLocal()
{
	if (CustomKingdomUtil::IF(m_nHitFlash))
	{
		m_nHitFlash--;
		int value = m_nHitFlash % 8 / 4;
		if (!CustomKingdomUtil::IF(m_nHitFlash))
		{
			pModel(5u)->SetFogIdx(2);
			pModel(4u)->SetFogIdx(2);
			SetFlash(5, 0.0f);
			SetFlash(4, 0.0f);
		}
		else if (CustomKingdomUtil::IF(value))
		{
			pModel(5u)->SetFogIdx(3);
			pModel(4u)->SetFogIdx(3);
			SetFlash(5, 1.0f);
			SetFlash(4, 1.0f);
		}
		else
		{
			pModel(5u)->SetFogIdx(2);
			pModel(4u)->SetFogIdx(2);
			SetFlash(5, 0.0f);
			SetFlash(4, 0.0f);
		}
	}
	if (bTestFlag(64u))
	{
		return true;
	}
	return ExecModeFunc() != 0;
}

bool cTaskBoss05Last::ActionOther()
{
	return true;
}

void cTaskBoss05Last::AdjustMaxSpeed()
{
	float num = ::Constants::jetGeoVecLength(vSpeed());
	if (num > fNowMaxSpeed())
	{
		num = fNowMaxSpeed() / num;
		Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
	}
}

bool cTaskBoss05Last::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	cTaskFade* cTaskFade2 = KingdomGlobal::g_pMotosGame->pTaskFade();
	if (GetMode() != 0)
	{
		if (1 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				fFogNear() = 0.0f;
				fFogFar() = 0.0f;
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				cTaskBoss05Last::SET_MODE_WAIT(this, 2, 10);
				return true;
			}
		}
		else if (2 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				OffFlag(32u);
				OffFlag(16385u);
				return true;
			}
			if (3 == nMes)
			{
				KingdomGlobal::g_pMotosGame->IsClearReserve() = false;
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				if (Constants::motosCharMove(this, s_vPos[0]))
				{
					nAction() = 0;
					cTaskBoss05Last::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, s_MdeTbl[nAction()]->m_nWait);
				}
				SetMortion(5u, "ANM_CHAR_BOSS_5_A10");
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nTimer() = m_nHomingFrame;
				::Constants::jetGeoVecUnit(vSpeed());
				nInputDir() = 0;
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				nInputDir() = 0;
				return true;
			}
			if (1 == nMes)
			{
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				nInputDir() = (ushort)Input();
				vRotate()[1] += m_fRotSpeed[0];
				if (nTimer()-- <= 0)
				{
					SetNextMode();
				}
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nTimer() = m_nRandomFrame;
				::Constants::jetGeoVecCopy(vRandTarget(), vSetRandTarget());
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
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				if (Constants::motosCharMove(this, vRandTarget(), 1.0f, m_fNowAccel))
				{
					::Constants::jetGeoVecCopy(vRandTarget(), vSetRandTarget());
				}
				vRotate()[1] += m_fRotSpeed[1];
				if (nTimer() % 40 == 0 && ++Step(1) <= 3)
				{
					SpawnBombRot(vRotate()[1] + Constants::TSP_DEG2RAD(45.0f), 6.0f, 0.55f, 0.05f, fDefHitSize() + 0.2f);
					SpawnBombRot(vRotate()[1] + Constants::TSP_DEG2RAD(135.0f), 6.0f, 0.55f, 0.05f, fDefHitSize() + 0.2f);
					SpawnBombRot(vRotate()[1] + Constants::TSP_DEG2RAD(225.0f), 6.0f, 0.55f, 0.05f, fDefHitSize() + 0.2f);
					SpawnBombRot(vRotate()[1] + Constants::TSP_DEG2RAD(315.0f), 6.0f, 0.55f, 0.05f, fDefHitSize() + 0.2f);
				}
				if (--nTimer() <= 0)
				{
					SetNextMode();
				}
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
		}
		else if (5 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nMovePoint() = s_MdeTbl[nAction()]->m_nMove;
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
				vRotate()[1] += m_fRotSpeed[1];
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel))
				{
					SetNextMode();
				}
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE025_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				return true;
			}
		}
		else if (6 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nTimer() = m_nStopFrame;
				::Constants::jetGeoVecUnit(vSpeed());
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.1f, nTimer());
				return true;
			}
			if (3 == nMes)
			{
				vMTrans()[1] = 0.0f;
				return true;
			}
			if (1 == nMes)
			{
				vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
				if (nTimer()-- <= 0)
				{
					SetNextMode();
				}
				return true;
			}
		}
		else if (7 == GetMode())
		{
			if (2 == nMes)
			{
				SetMortion(5u, "ANM_CHAR_BOSS_5_A09");
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				fFogNear() = 0.0f;
				fFogFar() = 0.0f;
				fCoreFogNear() = -30.0f;
				fCoreFogFar() = 30.0f;
				BombAllExplode();
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.1f, 120);
				::Constants::jetGeoVecCopy(m_vDeathPoint, vMTrans());
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				::Constants::jetGeoVecCopy(vMTrans(), m_vDeathPoint);
				switch (Step())
				{
				case 0:
					{
						bool flag = true;
						if (++Step(1) < 45)
						{
							fFogNear() -= 1.0f / 3.0f;
							fFogFar() -= -1.0f / 3.0f;
							flag = false;
						}
						if (++Step(2) < 45)
						{
							fCoreFogNear() -= 0.0f;
							fCoreFogFar() -= 0.0f;
							uint num = (uint)(2 * (90 - (90 - Step(2))));
							if (num > 255)
							{
								num = 255u;
							}
							nCoreFogColor() = 0xFF0000FFu | (num << 16) | (num << 8);
							flag = false;
						}
						if (flag)
						{
							Step(1) = 0;
							Step(2) = 0;
							Step()++;
						}
						vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
						break;
					}
				case 1:
					switch (Step(2))
					{
					case 0:
						if (++Step(1) > 15)
						{
							Constants::SpawnEffect(21, vMTrans());
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.25f, 20);
							Step(1) = 0;
							Step(2)++;
							cTaskFade2->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE, 1000);
						}
						break;
					case 1:
						if (++Step(1) > 10)
						{
							Constants::SpawnEffect(21, vMTrans());
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.25f, 20);
							Step(1) = 0;
							Step(2)++;
						}
						break;
					case 2:
						if (++Step(1) > 20)
						{
							Constants::SpawnEffect(21, vMTrans());
							OnFlag(1u);
							KingdomGlobal::g_pMotosGame->nScore() += nScore();
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.45f, 40);
							fHitSize() = 0.0f;
							Step(1) = 0;
							Step()++;
						}
						break;
					}
					break;
				case 2:
					if (cTaskFade2->IsEnd())
					{
						OnFlag(33u);
						cTaskFade2->Fade(EFadeKind::eFADE_TYPE_IN_WHITE, 1000);
						Step()++;
						Step(1) = 0;
						for (uint num2 = 0u; pModel(num2) != nullptr; num2++)
						{
							::Constants::jetGeoVecUnit(pModel(num2)->vScale());
						}
					}
					break;
				case 3:
					if (++Step(1) > 100)
					{
						Step()++;
						ChangeTaiki();
					}
					break;
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
				if (0 >= --nTimer())
				{
					SetMode(ReturnMode());
				}
				return true;
			}
		}
	}
	return true;
}

void cTaskBoss05Last::RefAttacked(int nParts, int nGrobalID)
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
	if (nParts == 0 && !cTaskChar2->bTestFlag(384u) && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u) && (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(131072u) || KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(384u)))
	{
		m_nHitFlash = 15;
		nHp() -= HitDamage(cTaskChar2, g_HitDamage);
		if (nHp() <= 0)
		{
			SetMode(7);
		}
		if (flag)
		{
			SpawnBossColEffect_Core(cTaskChar2);
		}
	}
}

void cTaskBoss05Last::SlantFloor()
{
	float num = vMTrans()[0] - s_vPos[0][0];
	if (Constants::fabs(num) < 0.25f)
	{
		num = 0.0f;
		if (m_fSlopeAccel > 0.0f)
		{
			if (!(m_fSlopeAccel - 2E-05f < 0.0f))
			{
				m_fSlopeAccel -= 2E-05f;
			}
		}
		else if (m_fSlopeAccel < 0.0f && !(m_fSlopeAccel + 2E-05f > 0.0f))
		{
			m_fSlopeAccel += 2E-05f;
		}
	}
	m_fSlopeAccel += 0.0004f / s_vPos[0][0] * (0.0f - num);
	if (KingdomGlobal::g_pMotosGame->fSlopeRad() > 0.0f)
	{
		m_fSlopeAccel -= 0.0001f * (KingdomGlobal::g_pMotosGame->fSlopeRad() / Constants::TSP_DEG2RAD(15.0f));
	}
	else if (KingdomGlobal::g_pMotosGame->fSlopeRad() < 0.0f)
	{
		m_fSlopeAccel += 0.0001f * ((0.0f - KingdomGlobal::g_pMotosGame->fSlopeRad()) / Constants::TSP_DEG2RAD(15.0f));
	}
	KingdomGlobal::g_pMotosGame->fSlopeRad() += m_fSlopeAccel;
	if (KingdomGlobal::g_pMotosGame->fSlopeRad() > m_fSlopeMaxRad)
	{
		if (m_fSlopeAccel > 0.0f)
		{
			m_fSlopeAccel = (0.0f - m_fSlopeAccel) * 0.5f;
		}
		KingdomGlobal::g_pMotosGame->fSlopeRad() = m_fSlopeMaxRad;
	}
	else if (KingdomGlobal::g_pMotosGame->fSlopeRad() < 0.0f - m_fSlopeMaxRad)
	{
		if (m_fSlopeAccel < 0.0f)
		{
			m_fSlopeAccel = (0.0f - m_fSlopeAccel) * 0.5f;
		}
		KingdomGlobal::g_pMotosGame->fSlopeRad() = 0.0f - m_fSlopeMaxRad;
	}
}

uint cTaskBoss05Last::Input()
{
	float num = 1.0f - HereticRate();
	ushort num2 = nInputDir();
	if (bTestFlag(1088u))
	{
		return 0u;
	}
	if (bTestFlag(2097152u))
	{
		return 0u;
	}
	cTaskCharPlayer* cTaskCharPlayer2 = KingdomGlobal::g_pMotosGame->pPlayers(0u);
	Vector4 v;
	if (cTaskCharPlayer2 != nullptr)
	{
		::Constants::jetGeoVecCopy(v, cTaskCharPlayer2->vMTrans());
		if (cTaskCharPlayer2->bTestFlag(384u))
		{
			Vector4 _out;
			Constants::VEC3_SUB(_out, vMTrans(), cTaskCharPlayer2->vMTrans());
			if (fHitSize() * 1.5f > Constants::tspVector3Length(_out))
			{
				::Constants::jetGeoVecCopy(v, s_vPos[0]);
			}
		}
	}
	else
	{
		::Constants::jetGeoVecCopy(v, s_vPos[0]);
	}
	float fDx = v[0] - vMTrans()[0];
	float fDz = v[2] - vMTrans()[2];
	if (nHamariTimer() > (int)fExParam(0) && bTestFlag(2048u) && IsFall(vMTrans()) && !CheckFall(vMTrans()))
	{
		num2 = nHamariRecoverDir(vMTrans(), v);
	}
	else
	{
		if ((long)Constants::get_GAME_COUNTER() % (long)nChangeMoveFrame() == 0L)
		{
			num2 = nTargetDir(fDx, fDz);
		}
		ushort num3 = nFallPassDirTarget(num2, v, vSpeed(), fAccelSpeed() * num);
		if (num3 != num2)
		{
			num2 = num3;
			::Constants::jetGeoVecUnit(vSpeed());
		}
	}
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, Constants::motosDir2Vector(num2));
	Constants::VEC3_MUL(v2, v2, fAccelSpeed() * num);
	Constants::VEC3_ADD(vSpeed(), vSpeed(), v2);
	return num2;
}

Vector4 cTaskBoss05Last::vSetRandTarget()
{
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	float num = cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth();
	float num2 = cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth();
	::Constants::jetGeoVecUnit(m_vSTarget);
	float n = (float)Random::Range(0, 100) * 0.1f + 1.0f;
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(nRandomDir()));
	Constants::VEC3_MUL(m_vSTarget, v, n);
	Constants::VEC3_ADD(m_vSTarget, vMTrans(), m_vSTarget);
	if (m_vSTarget[0] + 3.5f > num || m_vSTarget[0] - 3.5f < 0.0f || m_vSTarget[2] + 3.5f > num2 || m_vSTarget[2] - 3.5f < 0.0f)
	{
		::Constants::jetGeoVecUnit(vSpeed());
		::Constants::jetGeoVecCopy(m_vSTarget, vMTrans());
	}
	return m_vSTarget;
}

int cTaskBoss05Last::NextMode()
{
	return s_MdeTbl[(nAction() + 1) % 4]->m_nMode;
}

void cTaskBoss05Last::SetNextMode()
{
	if (s_MdeTbl[nAction()]->m_nMode == NextMode())
	{
		StepInit();
	}
	int nWait = s_MdeTbl[nAction()]->m_nWait;
	nAction() = (nAction() + 1) % 4;
	cTaskBoss05Last::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait);
}

}
