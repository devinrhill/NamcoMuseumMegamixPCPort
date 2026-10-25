#include "kingdom/game/g07/cTaskBoss04.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/g07/cParamBoss04.hpp"
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

Vector4 cTaskBoss04::s_vPos[7] = {};
int cTaskBoss04::g_HitDamage[6] = {};
const Constants::MO_BOSS_MODE_INFO* cTaskBoss04::s_MdeTbl[18] = {[]() { Constants::MO_BOSS_MODE_INFO* _o1 = new Constants::MO_BOSS_MODE_INFO(); _o1->m_nMode = 3; _o1->m_nMove = -1; _o1->m_nWait = 20; return _o1; }(), []() { Constants::MO_BOSS_MODE_INFO* _o2 = new Constants::MO_BOSS_MODE_INFO(); _o2->m_nMode = 4; _o2->m_nMove = 3; _o2->m_nWait = 0; return _o2; }(), []() { Constants::MO_BOSS_MODE_INFO* _o3 = new Constants::MO_BOSS_MODE_INFO(); _o3->m_nMode = 7; _o3->m_nMove = -1; _o3->m_nWait = 20; return _o3; }(), []() { Constants::MO_BOSS_MODE_INFO* _o4 = new Constants::MO_BOSS_MODE_INFO(); _o4->m_nMode = 7; _o4->m_nMove = -1; _o4->m_nWait = 20; return _o4; }(), []() { Constants::MO_BOSS_MODE_INFO* _o5 = new Constants::MO_BOSS_MODE_INFO(); _o5->m_nMode = 7; _o5->m_nMove = -1; _o5->m_nWait = 10; return _o5; }(), []() { Constants::MO_BOSS_MODE_INFO* _o6 = new Constants::MO_BOSS_MODE_INFO(); _o6->m_nMode = 3; _o6->m_nMove = -1; _o6->m_nWait = 20; return _o6; }(), []() { Constants::MO_BOSS_MODE_INFO* _o7 = new Constants::MO_BOSS_MODE_INFO(); _o7->m_nMode = 4; _o7->m_nMove = 6; _o7->m_nWait = 0; return _o7; }(), []() { Constants::MO_BOSS_MODE_INFO* _o8 = new Constants::MO_BOSS_MODE_INFO(); _o8->m_nMode = 7; _o8->m_nMove = -1; _o8->m_nWait = 15; return _o8; }(), []() { Constants::MO_BOSS_MODE_INFO* _o9 = new Constants::MO_BOSS_MODE_INFO(); _o9->m_nMode = 7; _o9->m_nMove = -1; _o9->m_nWait = 15; return _o9; }(), []() { Constants::MO_BOSS_MODE_INFO* _o10 = new Constants::MO_BOSS_MODE_INFO(); _o10->m_nMode = 7; _o10->m_nMove = -1; _o10->m_nWait = 10; return _o10; }(), []() { Constants::MO_BOSS_MODE_INFO* _o11 = new Constants::MO_BOSS_MODE_INFO(); _o11->m_nMode = 3; _o11->m_nMove = -1; _o11->m_nWait = 20; return _o11; }(), []() { Constants::MO_BOSS_MODE_INFO* _o12 = new Constants::MO_BOSS_MODE_INFO(); _o12->m_nMode = 4; _o12->m_nMove = 0; _o12->m_nWait = 0; return _o12; }(), []() { Constants::MO_BOSS_MODE_INFO* _o13 = new Constants::MO_BOSS_MODE_INFO(); _o13->m_nMode = 7; _o13->m_nMove = -1; _o13->m_nWait = 15; return _o13; }(), []() { Constants::MO_BOSS_MODE_INFO* _o14 = new Constants::MO_BOSS_MODE_INFO(); _o14->m_nMode = 7; _o14->m_nMove = -1; _o14->m_nWait = 15; return _o14; }(), []() { Constants::MO_BOSS_MODE_INFO* _o15 = new Constants::MO_BOSS_MODE_INFO(); _o15->m_nMode = 7; _o15->m_nMove = -1; _o15->m_nWait = 10; return _o15; }(), []() { Constants::MO_BOSS_MODE_INFO* _o16 = new Constants::MO_BOSS_MODE_INFO(); _o16->m_nMode = 5; _o16->m_nMove = -1; _o16->m_nWait = 0; _o16->m_nWork[0] = 1; _o16->m_nWork[1] = 0; _o16->m_nWork[2] = 0; _o16->m_nWork[3] = 0; return _o16; }(), []() { Constants::MO_BOSS_MODE_INFO* _o17 = new Constants::MO_BOSS_MODE_INFO(); _o17->m_nMode = 5; _o17->m_nMove = -1; _o17->m_nWait = 0; return _o17; }(), []() { Constants::MO_BOSS_MODE_INFO* _o18 = new Constants::MO_BOSS_MODE_INFO(); _o18->m_nMode = 5; _o18->m_nMove = -1; _o18->m_nWait = 20; _o18->m_nWork[0] = 1; _o18->m_nWork[1] = 0; _o18->m_nWork[2] = 0; _o18->m_nWork[3] = 0; return _o18; }()};

float cTaskBoss04::get_APPEAR_ADD_NEAR_FOG()
{
	return 0.0f;
}

float cTaskBoss04::get_APPEAR_ADD_FAR_FOG()
{
	return 0.0f;
}

float cTaskBoss04::DAMAGE_BASE_HP(cTaskCharBoss* boss)
{
	return (int)((float)boss->nMaxHp() * 0.6f);
}

float cTaskBoss04::DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss)
{
	return 60.0f / cTaskBoss04::DAMAGE_BASE_HP(boss);
}

float cTaskBoss04::DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss)
{
	return 30.0f / cTaskBoss04::DAMAGE_BASE_HP(boss);
}

void cTaskBoss04::SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time)
{
	boss->nTimer() = time;
	boss->SetMode(8);
	boss->ReturnMode((ushort)mode);
}

void cTaskBoss04::SET_MODE_PIYO(cTaskCharBoss* boss, int mode)
{
	boss->SetMode(6);
	boss->ReturnMode((ushort)mode);
}

cTaskBoss04::cTaskBoss04()
{
	Init();
}

Vector4& cTaskBoss04::vTarget()
{
	return m_vTarget;
}

Constants::MO_RING_INFO* cTaskBoss04::pRingInfo(uint n)
{
	if ((long)n >= (long)::Constants::ARRAY_SIZE(m_RingInfo))
	{
		return nullptr;
	}
	return m_RingInfo[n];
}

float& cTaskBoss04::fCenterPosY()
{
	return m_fCenterPosY;
}

Vector4& cTaskBoss04::vPillarScale()
{
	return m_vPillarScale;
}

byte& cTaskBoss04::nRingAlpha()
{
	return m_nRingAlpha;
}

cKingdomPtkHnd*& cTaskBoss04::pHover(int n)
{
	return m_pHover[n];
}

float& cTaskBoss04::fNowAccel()
{
	return m_fNowAccel;
}

float& cTaskBoss04::fNowMaxSpeed()
{
	return m_fNowMaxSpeed;
}

float& cTaskBoss04::fRandomAccel()
{
	return m_fRandomAccel;
}

float& cTaskBoss04::fRandomMaxSpeed()
{
	return m_fRandomMaxSpeed;
}

float& cTaskBoss04::fSlopeAccel()
{
	return m_fSlopeAccel;
}

float& cTaskBoss04::fSlopeMaxSpeed()
{
	return m_fSlopeMaxSpeed;
}

void cTaskBoss04::SetApearScale(float x, float y, float z)
{
	Vector4 v = Vector4(x, y, z, 1.0f);
	::Constants::jetGeoVecCopy(pModel(1u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(2u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(3u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(4u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(5u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(6u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(7u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(8u)->vScale(), v);
}

void cTaskBoss04::Init()
{
	m_fRotSpeed = 0.0f;
	m_nRandomFrame = 0;
	m_nDownFrame = 0;
	m_nPiyoFrame = 0;
	m_fNowAccel = 0.0f;
	m_fNowMaxSpeed = 0.0f;
	m_fCenterPosY = 0.0f;
	m_nRestTime = 0;
	m_nRingHP = 0;
	m_nHitFlash = 0;
	m_nSlopeTime = 0;
	m_nSlopeStopTime = 0;
	m_fSlopeRadMax = 0.0f;
	m_fAddSlopeRad = 0.0f;
	m_fAccelRotSpeed = 0.0f;
	m_fAddAccelRotSpeed = 0.0f;
	m_fMaxAccelRotSpeed = 0.0f;
	m_nAccelRotTime = 0;
	m_fRandomAccel = 0.0f;
	m_fRandomMaxSpeed = 0.0f;
	m_fSlopeAccel = 0.0f;
	m_fSlopeMaxSpeed = 0.0f;
	m_nMoveSEPitch = 0;
	m_nRoteSEPitch = 0;
	m_fLightMot = 0.0f;
	m_fLightMotAdd = 0.5f;
	m_fLightAmbAdd = 0.0f;
	nAction() = 0;
	nMovePoint() = 0;
	m_fCenterSpeed = 0.0f;
	fChargeDamageRate() = 0.0f;
	std::fill(std::begin(m_fPillarMot), std::end(m_fPillarMot), 0.0f);
	LampForceDell();
	std::fill(std::begin(m_pHover), std::end(m_pHover), nullptr);
	for (int i = 0; i < (int)std::size(m_pHover); i++)
	{
		m_pHover[i] = new cKingdomPtkHnd();
	}
	std::fill(std::begin(m_RingInfo), std::end(m_RingInfo), nullptr);
	for (int j = 0; j < (int)std::size(m_RingInfo); j++)
	{
		m_RingInfo[j] = new Constants::MO_RING_INFO();
	}
	SetMode(1);
	ExHitAlloc(6);
	ExHitRegistCircle(0, Constants::VEC3_GET(2.5f, 0.0f, 0.0f), 0.6f, 7);
	ExHitRegistCircle(1, Constants::VEC3_GET(0.0f, 0.0f, -2.5f), 0.6f, 7);
	ExHitRegistCircle(2, Constants::VEC3_GET(-2.5f, 0.0f, 0.0f), 0.6f, 7);
	ExHitRegistCircle(3, Constants::VEC3_GET(0.0f, 0.0f, 2.5f), 0.6f, 7);
	ExHitRegistRect(4, Constants::VEC3_GET(-2.5f, 0.0f, -0.3f), 5.0f, 0.6f);
	ExHitRegistRect(5, Constants::VEC3_GET(-0.3f, 0.0f, -2.5f), 0.6f, 5.0f);
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	Constants::VEC3_SET(s_vPos[0], cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f + 0.5f);
	Constants::VEC3_SET(s_vPos[1], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() / 2 - 4), 0.0f, s_vPos[0][2]);
	Constants::VEC3_SET(s_vPos[2], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() / 2 + 4), 0.0f, s_vPos[0][2]);
	Constants::VEC3_SET(s_vPos[3], s_vPos[1][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 - 3));
	Constants::VEC3_SET(s_vPos[4], s_vPos[1][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 + 4));
	Constants::VEC3_SET(s_vPos[5], s_vPos[2][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 - 3));
	Constants::VEC3_SET(s_vPos[6], s_vPos[2][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 + 4));
	cParamBoss04* cParamBoss6 = new cParamBoss04("CSV_BOSS4_PARAM");
	m_fRotSpeed = cParamBoss6->GetValueF32(1, 4);
	m_nRandomFrame = cParamBoss6->GetValueS32(1, 9);
	m_nDownFrame = cParamBoss6->GetValueS32(1, 10);
	m_nPiyoFrame = cParamBoss6->GetValueS32(1, 11);
	m_nRingHP = cParamBoss6->GetValueS32(1, 12);
	m_fRandomAccel = cParamBoss6->GetValueF32(1, 5);
	m_fRandomMaxSpeed = cParamBoss6->GetValueF32(1, 6);
	m_fSlopeAccel = cParamBoss6->GetValueF32(1, 7);
	m_fSlopeMaxSpeed = cParamBoss6->GetValueF32(1, 8);
	m_nSlopeTime = cParamBoss6->GetValueS32(1, 14);
	m_nSlopeStopTime = cParamBoss6->GetValueS32(1, 15);
	m_fSlopeRadMax = Constants::TSP_DEG2RAD(cParamBoss6->GetValueF32(1, 16));
	m_nAccelRotTime = cParamBoss6->GetValueS32(1, 18);
	m_fMaxAccelRotSpeed = cParamBoss6->GetValueF32(1, 17);
	nHp() = cParamBoss6->GetValueS32(1, 1);
	nHp() *= 100;
	nBombMax() = cParamBoss6->GetValueS32(1, 2);
	nBombTime() = cParamBoss6->GetValueS32(1, 3);
	nScore() = cParamBoss6->GetValueS32(1, 13);
	nMaxHp() = nHp();
	cParamDamage* cParamDamage2 = new cParamDamage("CSV_HIT_DAMAGE");
	for (int k = 0; k < 6; k++)
	{
		g_HitDamage[k] = (int)(cParamDamage2->GetValueF32(k + 1, 4) * 100.0f);
	}
	fChargeDamageRate() = cParamDamage2->GetValueF32(7, 4);
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay() && KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP > 0)
	{
		nHp() = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP;
	}
	if ((float)nHp() < cTaskBoss04::DAMAGE_BASE_HP(this))
	{
		fCoreFogNear() = 30.0f - cTaskBoss04::DAMAGE_ADD_NEAR_FOG(this) * (cTaskBoss04::DAMAGE_BASE_HP(this) - (float)nHp());
		fCoreFogFar() = 60.0f - cTaskBoss04::DAMAGE_ADD_FAR_FOG(this) * (cTaskBoss04::DAMAGE_BASE_HP(this) - (float)nHp());
	}
	if (m_nRingHP == 0)
	{
		m_nRingHP = 1;
	}
	if (nBombMax() > 32)
	{
		nBombMax() = 32;
	}
	CreateBomb();
	nCoreFogColor() = 4278190335u;
	m_nRingAlpha = 0;
	Constants::VEC3_SET(m_vPillarScale, 0.0f, 0.0f, 0.0f);
	OnFlag(4194304u);
}

void cTaskBoss04::Term()
{
	cTaskCharBoss::Term();
}

bool cTaskBoss04::ActionBoth()
{
	return true;
}

bool cTaskBoss04::ActionLocal()
{
	for (int i = 0; i < 4; i++)
	{
		if (pHover(i) != nullptr)
		{
			Vector4 v = Vector4();
			pModel(1u)->GetLocalNodeTrans(v, "effect");
			::Constants::jetGeoVecRotY(v, v, vRotate()[1] + ::Constants::JET_DEG2RAD(90.0f) * (float)i);
			::Constants::jetGeoVecAdd(v, v, vMTrans());
			pHover(i)->vSetTrans(v);
			pHover(i)->UpdateMatrix(KingdomGlobal::g_pMotosGame->pSlopeMatrix());
		}
	}
	for (int j = 0; j < 4; j++)
	{
		Constants::MO_RING_INFO* mO_RING_INFO = m_RingInfo[j];
		SetRingDamageFrame(j);
		if (m_fCenterPosY < 0.0f && m_fCenterPosY + 2.0f <= mO_RING_INFO->m_vRingPos[1])
		{
			float fGoPosY = (mO_RING_INFO->m_vRingPos[1] = m_fCenterPosY + 2.0f);
			mO_RING_INFO->m_fGoPosY = fGoPosY;
			mO_RING_INFO->m_nHitCnt = 0;
			mO_RING_INFO->m_fUPSp = 0.0f;
		}
		if (mO_RING_INFO->m_vRingPos[1] < mO_RING_INFO->m_fGoPosY)
		{
			if ((mO_RING_INFO->m_vRingPos[1] += mO_RING_INFO->m_fUPSp) >= mO_RING_INFO->m_fGoPosY)
			{
				mO_RING_INFO->m_vRingPos[1] = mO_RING_INFO->m_fGoPosY;
				mO_RING_INFO->m_fUPSp = 0.0f;
			}
			else if (m_fCenterPosY < 0.0f && mO_RING_INFO->m_vRingPos[1] > 1.8f)
			{
				mO_RING_INFO->m_vRingPos[1] = (mO_RING_INFO->m_fGoPosY = 1.8f);
			}
			if ((mO_RING_INFO->m_fUPSp -= 0.05f) < 0.05f)
			{
				mO_RING_INFO->m_fUPSp = 0.05f;
			}
		}
		if (IsRingUP(j))
		{
			LampBurn(j);
		}
	}
	if (m_fCenterPosY != -2.0f)
	{
		OnFlag(256u);
		fHitSize() = fDefHitSize();
		ExHitParam(4)->m_nFlag = 4;
		ExHitParam(5)->m_nFlag = 4;
	}
	else
	{
		OffFlag(256u);
		fHitSize() = fDefHitSize();
		ExHitParam(4)->m_nFlag = 1;
		ExHitParam(5)->m_nFlag = 1;
	}
	if (CustomKingdomUtil::IF(m_nHitFlash))
	{
		m_nHitFlash--;
		int value = m_nHitFlash % 8 / 4;
		if (!CustomKingdomUtil::IF(m_nHitFlash))
		{
			pModel()->SetFogIdx(2);
			pModel(9u)->SetFogIdx(2);
			SetMortion(0u, "ANM_CHAR_BOSS_4_A00");
			SetFlash(0, 0.0f);
			SetFlash(9, 0.0f);
		}
		else if (CustomKingdomUtil::IF(value))
		{
			pModel()->SetFogIdx(3);
			pModel(9u)->SetFogIdx(3);
			SetFlash(0, 1.0f);
			SetFlash(9, 1.0f);
		}
		else
		{
			pModel()->SetFogIdx(2);
			pModel(9u)->SetFogIdx(2);
			SetFlash(0, 0.0f);
			SetFlash(9, 0.0f);
		}
	}
	return ExecModeFunc() != 0;
}

bool cTaskBoss04::ActionOther()
{
	return true;
}

void cTaskBoss04::AdjustMaxSpeed()
{
	float num = ::Constants::jetGeoVecLength(vSpeed());
	if (num > fNowMaxSpeed())
	{
		num = fNowMaxSpeed() / num;
		Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
	}
}

void cTaskBoss04::SetRingDamageFrame(int nRing)
{
	uint array[4] = {1u, 2u, 3u, 4u};
	uint obj[4] = {5u, 6u, 7u, 8u};
	uint idx = array[nRing];
	uint idx2 = obj[nRing];
	if (GetMode() == 6 || IsRingUP(nRing))
	{
		pModel(idx)->SetFrameMatClr(1u, 2.3333333f);
		pModel(idx2)->SetFrameMatClr(0u, 2.3333333f);
	}
	else
	{
		float num = m_RingInfo[nRing]->m_vRingPos[1] / 2.0f * 139.0f;
		pModel(idx)->SetFrameMatClr(1u, num);   // native frames (the C# passed Unity seconds)
		pModel(idx2)->SetFrameMatClr(0u, num);   // native frames (the C# passed Unity seconds)
	}
}

void cTaskBoss04::SpawnHover()
{
	Vector4 _out = Vector4();
	for (int i = 0; i < 4; i++)
	{
		if (pHover(i) == nullptr)
		{
			Constants::VEC3_SUB(_out, _out, vMTrans());
			Constants::tspVector3RotY(_out, _out, Constants::TSP_DEG2RAD(90 * i));
			Constants::VEC3_ADD(_out, _out, vMTrans());
			pHover(i) = Constants::SpawnEffect(28, pModel((uint)(1 + i))->vTrans(), nullptr, 0, false);
		}
	}
}

void cTaskBoss04::MoveSE()
{
	if (++m_nMoveSEPitch >= 25)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1);
		m_nMoveSEPitch = 0;
	}
}

void cTaskBoss04::MoveSEForce()
{
	SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1);
	m_nMoveSEPitch = 0;
}

void cTaskBoss04::RoteSE()
{
	if (++m_nRoteSEPitch >= 110)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1);
		m_nRoteSEPitch = 0;
	}
}

bool cTaskBoss04::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (GetMode() != 0)
	{
		if (1 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				Step() = 0;
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				m_fCenterPosY = -2.0f;
				OnFlag(32u);
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
				::Constants::jetGeoVecUnit(vSpeed());
				SpawnHover();
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				OffFlag(32u);
				MoveSEForce();
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				if (Constants::motosCharMove(this, s_vPos[0]))
				{
					nAction() = 0;
					cTaskBoss04::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, s_MdeTbl[nAction()]->m_nWait);
				}
				MoveSE();
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				if (m_nRestTime != 0)
				{
					nTimer() = m_nRestTime;
					m_nRestTime = 0;
				}
				else
				{
					nTimer() = m_nRandomFrame;
					::Constants::jetGeoVecCopy(vTarget(), vSetRandTarget());
				}
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
				m_fNowAccel = fRandomAccel();
				m_fNowMaxSpeed = fRandomMaxSpeed();
				if (Constants::motosCharMove(this, vTarget(), 0.05f, m_fNowAccel))
				{
					::Constants::jetGeoVecCopy(vTarget(), vSetRandTarget());
				}
				vRotate()[1] += m_fRotSpeed;
				if (IsAllRingUP())
				{
					m_nRestTime = nTimer();
					cTaskBoss04::SET_MODE_PIYO(this, 3);
				}
				else if (--nTimer() <= 0)
				{
					SetNextMode();
				}
				MoveSE();
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nMovePoint() = s_MdeTbl[nAction()]->m_nMove;
				::Constants::jetGeoVecUnit(vSpeed());
				MoveSEForce();
				return true;
			}
			if (3 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (1 == nMes)
			{
				if (Step(1) == 0)
				{
					float value;
					if ((value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(45.0f), m_fRotSpeed)) != -1.0f || (value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(135.0f), m_fRotSpeed)) != -1.0f || (value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(225.0f), m_fRotSpeed)) != -1.0f || (value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(315.0f), m_fRotSpeed)) != -1.0f)
					{
						vRotate()[1] = value;
						Step(1)++;
					}
					else
					{
						vRotate()[1] += m_fRotSpeed;
					}
				}
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel))
				{
					SetNextMode();
				}
				else if (IsAllRingUP())
				{
					cTaskBoss04::SET_MODE_PIYO(this, 4);
				}
				MoveSE();
				return true;
			}
		}
		else if (5 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				LampForceDell();
				if (!CustomKingdomUtil::IF(m_nSlopeTime) || !CustomKingdomUtil::IF(m_nAccelRotTime))
				{
					SetNextMode();
				}
				else
				{
					if (m_fAccelRotSpeed > m_fMaxAccelRotSpeed)
					{
						m_fAccelRotSpeed = m_fMaxAccelRotSpeed;
					}
					else if (m_fAccelRotSpeed < m_fRotSpeed)
					{
						m_fAccelRotSpeed = m_fRotSpeed;
					}
					if (m_nAccelRotTime != 0)
					{
						m_fAddAccelRotSpeed = (m_fMaxAccelRotSpeed - m_fAccelRotSpeed) / (float)m_nAccelRotTime;
					}
				}
				nLTimer() = 80;
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				if (Step() != 8 && Step() != 9 && --nLTimer() <= 0)
				{
					nLTimer() = (int)(80.0f - 50.0f / Constants::fabs(m_fMaxAccelRotSpeed) * Constants::fabs(m_fAccelRotSpeed));
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE024_BOSS_ROT1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				}
				switch (Step())
				{
				case 0:
				_switch19_0:
					if (!CustomKingdomUtil::IF(s_MdeTbl[nAction()]->m_nWork[0]))
					{
						m_vTarget[0] = s_vPos[2][0];
						m_vTarget[1] = 0.0f;
						m_vTarget[2] = vMTrans()[2];
					}
					else
					{
						m_vTarget[0] = s_vPos[1][0];
						m_vTarget[1] = 0.0f;
						m_vTarget[2] = vMTrans()[2];
					}
					AjustAccelRotate();
					Step()++;
					break;
				case 1:
				_switch19_1:
					{
						bool flag = false;
						m_fCenterSpeed = -2.0f / 35.0f;
						flag = CenterMove();
						if (flag)
						{
							m_fCenterPosY = -2.0f;
						}
						m_fNowAccel = fAccelSpeed();
						m_fNowMaxSpeed = fMaxSpeed();
						if (Constants::motosCharMove(this, m_vTarget, 0.05f, m_fNowAccel) && flag)
						{
							if (!CustomKingdomUtil::IF(s_MdeTbl[nAction()]->m_nWork[0]))
							{
								m_fAddSlopeRad = (m_fSlopeRadMax - KingdomGlobal::g_pMotosGame->fSlopeRad()) / (float)m_nSlopeTime;
							}
							else
							{
								m_fAddSlopeRad = (0.0f - m_fSlopeRadMax - KingdomGlobal::g_pMotosGame->fSlopeRad()) / (float)m_nSlopeTime;
							}
							Step()++;
						}
						AjustAccelRotate();
						break;
					}
				case 2:
				_switch19_2:
					{
						if (!CustomKingdomUtil::IF(s_MdeTbl[nAction()]->m_nWork[0]))
						{
							m_vTarget[0] = s_vPos[4][0];
						}
						else
						{
							m_vTarget[0] = s_vPos[6][0];
						}
						m_vTarget[1] = 0.0f;
						m_vTarget[2] = KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans()[2];
						if (m_vTarget[2] < s_vPos[3][2] + 1.5f)
						{
							m_vTarget[2] = s_vPos[3][2] + 1.5f;
						}
						else if (m_vTarget[2] > s_vPos[4][2] - 1.5f)
						{
							m_vTarget[2] = s_vPos[4][2] - 1.5f;
						}
						for (int j = 0; j < 6; j++)
						{
							ExHitParam(j)->m_nFlag |= 16;
						}
						AjustAccelRotate();
						Step()++;
						break;
					}
				case 3:
				_switch19_3:
					if (++Step(1) < m_nSlopeTime)
					{
						KingdomGlobal::g_pMotosGame->fSlopeRad() += m_fAddSlopeRad;
					}
					else
					{
						if (!CustomKingdomUtil::IF(s_MdeTbl[nAction()]->m_nWork[0]))
						{
							KingdomGlobal::g_pMotosGame->fSlopeRad() = m_fSlopeRadMax;
						}
						else
						{
							KingdomGlobal::g_pMotosGame->fSlopeRad() = 0.0f - m_fSlopeRadMax;
						}
						Step(1) = 0;
						Step()++;
					}
					AjustAccelRotate();
					break;
				case 4:
				_switch19_4:
					if (++Step(1) > 90)
					{
						Step(1) = 0;
						Step()++;
					}
					AjustAccelRotate();
					break;
				case 5:
				_switch19_5:
					m_fNowAccel = fSlopeAccel();
					m_fNowMaxSpeed = fSlopeMaxSpeed();
					if (Constants::motosCharMove(this, m_vTarget, 0.05f, m_fNowAccel))
					{
						if (m_nAccelRotTime != 0)
						{
							if (NextMode() == 5)
							{
								m_fAddAccelRotSpeed = (0.0f - m_fAccelRotSpeed) / (float)m_nAccelRotTime;
							}
							else
							{
								m_fAddAccelRotSpeed = (m_fRotSpeed - m_fAccelRotSpeed) / (float)m_nAccelRotTime;
							}
						}
						Step(1) = 0;
						Step()++;
					}
					goto _switch19_6;
				case 6:
				_switch19_6:
					if (++Step(1) > m_nSlopeStopTime)
					{
						Step(1) = 0;
						if (NextMode() == 5)
						{
							Step() = 255;
						}
						else
						{
							m_fAddSlopeRad = (0.0f - KingdomGlobal::g_pMotosGame->fSlopeRad()) / (float)(m_nSlopeTime / 2);
							Step()++;
						}
					}
					if (NextMode() == 5)
					{
						AjustAccelRotate(0.0f);
					}
					else
					{
						AjustAccelRotate();
					}
					break;
				case 7:
				_switch19_7:
					m_fNowAccel = fAccelSpeed();
					m_fNowMaxSpeed = fMaxSpeed();
					if (Constants::motosCharMove(this, s_vPos[0], 0.05f, m_fNowAccel))
					{
						Step()++;
					}
					AjustAccelRotate();
					break;
				case 8:
				_switch19_8:
					{
						int num = AjustAccelRotate();
						for (int i = 0; i < 6; i++)
						{
							int num2 = -17;
							ExHitParam(i)->m_nFlag &= (ushort)num2;
						}
						if (++Step(1) < m_nSlopeTime / 2)
						{
							KingdomGlobal::g_pMotosGame->fSlopeRad() += m_fAddSlopeRad;
						}
						else
						{
							KingdomGlobal::g_pMotosGame->fSlopeRad() = 0.0f;
						}
						if (KingdomGlobal::g_pMotosGame->fSlopeRad() == 0.0f && num != 0)
						{
							float add = (CustomKingdomUtil::IF(s_MdeTbl[nAction()]->m_nWork[0]) ? (0.0f - m_fAccelRotSpeed) : m_fAccelRotSpeed);
							float value2;
							if ((value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(45.0f), add)) != -1.0f || (value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(135.0f), add)) != -1.0f || (value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(225.0f), add)) != -1.0f || (value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(315.0f), add)) != -1.0f)
							{
								m_fAddAccelRotSpeed = 0.0f;
								vRotate()[1] = value2;
								Step()++;
							}
						}
						break;
					}
				case 9:
				_switch19_9:
					m_fCenterSpeed = 0.1f;
					if (CenterMove())
					{
						m_fCenterPosY = 0.0f;
						SetNextMode();
					}
					break;
				case 255:
				_switch19_255:
					if (AjustAccelRotate(0.0f) != 0)
					{
						m_fAddAccelRotSpeed = 0.0f;
						SetNextMode();
					}
					break;
				}
				return true;
			}
		}
		else if (7 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				m_nRoteSEPitch = 110;
				return true;
			}
			if (3 == nMes)
			{
				LampForceDell();
				return true;
			}
			if (1 == nMes)
			{
				for (int k = 0; k < 4; k++)
				{
					if (!IsRingUP(k))
					{
						LampDell(k);
					}
				}
				switch (Step(1))
				{
				case 0:
					{
						float value3;
						if ((value3 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(45.0f), m_fRotSpeed)) != -1.0f || (value3 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(135.0f), m_fRotSpeed)) != -1.0f || (value3 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(225.0f), m_fRotSpeed)) != -1.0f || (value3 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(315.0f), m_fRotSpeed)) != -1.0f)
						{
							vRotate()[1] = value3;
							Step(1)++;
							break;
						}
						vRotate()[1] += m_fRotSpeed;
						if (IsAllRingUP())
						{
							cTaskBoss04::SET_MODE_PIYO(this, 7);
						}
						else
						{
							RoteSE();
						}
						break;
					}
				case 1:
					m_fCenterSpeed = -2.0f / 35.0f;
					if (CenterMove())
					{
						m_fCenterPosY = -2.0f;
						Step(1)++;
					}
					break;
				case 2:
					switch (nMovePoint())
					{
					case 4:
						SpawnBomb(1, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(8, 4.0f, 0.3f, fDefHitSize());
						break;
					case 5:
						SpawnBomb(2, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(4, 4.0f, 0.3f, fDefHitSize());
						break;
					case 3:
						SpawnBomb(2, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(8, 4.0f, 0.3f, fDefHitSize());
						break;
					case 6:
						SpawnBomb(1, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(4, 4.0f, 0.3f, fDefHitSize());
						break;
					case 0:
						SpawnBomb(1, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(2, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(4, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(8, 4.0f, 0.3f, fDefHitSize());
						break;
					}
					if (NextMode() == 7)
					{
						SetNextMode();
					}
					else
					{
						Step(1)++;
					}
					break;
				case 3:
					if (++Step(3) >= m_nDownFrame)
					{
						Step(1)++;
					}
					break;
				case 4:
					m_fCenterSpeed = 0.1f;
					if (CenterMove())
					{
						m_fCenterPosY = 0.0f;
						SetNextMode();
					}
					break;
				}
				return true;
			}
		}
		else if (6 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				m_fPiyoDWSp = 0.001f;
				m_fLightMotAdd = 0.0f;
				return true;
			}
			if (3 == nMes)
			{
				LampForceDell();
				return true;
			}
			if (1 == nMes)
			{
				m_fLightAmb += m_fLightAmbAdd;
				switch (Step(1))
				{
				case 0:
					if (Step(2)++ >= 30)
					{
						Step(2) = 0;
						Step(1)++;
					}
					break;
				case 1:
					if (Step(2) % 2 == 0)
					{
						m_fCenterPosY -= 0.075f;
						if (m_fCenterPosY <= -0.15f)
						{
							Step(2)++;
						}
					}
					else
					{
						m_fCenterPosY += 0.075f;
						if (m_fCenterPosY >= 0.0f)
						{
							Step(2)++;
						}
					}
					if (Step(2) % 5 == 0)
					{
						SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE053_FAINT, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					}
					if (Step(2) == 21)
					{
						Step(1)++;
					}
					break;
				case 2:
					m_fCenterPosY -= m_fPiyoDWSp;
					m_fPiyoDWSp += m_fPiyoDWSp * 0.2f;
					if (m_fCenterPosY <= -2.0f)
					{
						m_fCenterPosY = -2.0f;
						KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.15f, 30);
						SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE047_BOSS_FALL, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
						Step(1)++;
					}
					break;
				case 3:
					if (++Step(3) >= m_nPiyoFrame)
					{
						Step(3) = 0;
						Step(1)++;
					}
					else if (m_nPiyoFrame - Step(3) < 90)
					{
						LampBlink();
					}
					break;
				case 4:
					LampDell();
					m_fCenterPosY += 0.05f;
					if (++Step(3) == 40)
					{
						m_fCenterPosY = 0.0f;
						SetMode(ReturnMode());
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
				else if (IsAllRingUP())
				{
					cTaskBoss04::SET_MODE_PIYO(this, ReturnMode());
				}
				return true;
			}
		}
		else if (9 == GetMode())
		{
			if (2 == nMes)
			{
				SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE024_BOSS_ROT1);
				SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1);
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				fCoreFogNear() = -30.0f;
				fCoreFogFar() = 30.0f;
				for (int l = 0; l < 4; l++)
				{
					if (pHover(l) != nullptr)
					{
						Vector4 v = Vector4::zero;
						::Constants::jetGeoVecUnit(v);
						pHover(l)->vSetTrans(v);
						pHover(l)->vSetScale(v);
						pHover(l)->vSetRotate(v);
						pHover(l)->Detach(true);
						pHover(l) = nullptr;
					}
				}
				m_fCenterPosY = -2.0f;
				BombAllExplode();
				SetMortionReverse(1u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
				SetMortionReverse(2u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
				SetMortionReverse(3u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
				SetMortionReverse(4u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				cTaskFade* cTaskFade2 = KingdomGlobal::g_pMotosGame->pTaskFade();
				switch (Step())
				{
				case 0:
				_switch23_0:
					SpawnExplode2Random(this, Constants::VEC3_GET(-3.0f, 0.0f, 0.0f), 1.0f, 2.0f, 0.5f, 210);
					SpawnExplode2Random(this, Constants::VEC3_GET(2.5f, 0.0f, 0.0f), 1.0f, 2.0f, 0.5f, 210);
					SpawnExplode2Random(this, Constants::VEC3_GET(0.0f, 0.0f, -3.0f), 1.0f, 2.0f, 0.5f, 210);
					SpawnExplode2Random(this, Constants::VEC3_GET(0.0f, 0.0f, 2.5f), 1.0f, 2.0f, 0.5f, 210);
					Step()++;
					goto _switch23_1;
				case 1:
				_switch23_1:
					{
						bool flag2 = true;
						if (Step(1) < 60)
						{
							Step(1)++;
							flag2 = false;
						}
						if (Step(2) < 45)
						{
							Step(2)++;
							flag2 = false;
						}
						if (flag2)
						{
							Step(1) = 0;
							Step(2) = 0;
							Step()++;
						}
						ExplodeSE();
						break;
					}
				case 2:
				_switch23_2:
					if (++Step(1) > 40)
					{
						Step(1) = 0;
						Step()++;
					}
					ExplodeSE();
					break;
				case 3:
				_switch23_3:
					SpawnExplode2(this, Constants::VEC3_GET(-3.0f, 0.0f, 0.0f), 1.0f, 3.0f, 0.5f);
					SpawnExplode2(this, Constants::VEC3_GET(2.5f, 0.0f, 0.0f), 1.0f, 3.0f, 0.5f);
					SpawnExplode2(this, Constants::VEC3_GET(0.0f, 0.0f, -3.0f), 1.0f, 3.0f, 0.5f);
					SpawnExplode2(this, Constants::VEC3_GET(0.0f, 0.0f, 2.5f), 1.0f, 3.0f, 0.5f);
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE047_BOSS_FALL, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					Step()++;
					Step(1) = 0;
					break;
				case 4:
				_switch23_4:
					switch (Step(2))
					{
					case 0:
					_switch24_0:
						ExHitParam(0)->m_nKind = 2;
						ExHitParam(1)->m_nKind = 2;
						ExHitParam(2)->m_nKind = 2;
						ExHitParam(3)->m_nKind = 2;
						Step(1) = 0;
						Step(2)++;
						cTaskFade2->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE, 1000);
						goto _switch24_1;
					case 1:
					_switch24_1:
						if (++Step(1) > 20)
						{
							Constants::SpawnEffect(21, vMTrans());
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.2f, 20);
							Step(1) = 0;
							Step(2)++;
						}
						break;
					case 2:
					_switch24_2:
						if (++Step(1) > 10)
						{
							Constants::SpawnEffect(21, vMTrans());
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.2f, 20);
							Step(1) = 0;
							Step(2)++;
						}
						break;
					case 3:
					_switch24_3:
						if (++Step(1) == 20)
						{
							Constants::SpawnEffect(21, vMTrans());
							KingdomGlobal::g_pMotosGame->nScore() += nScore();
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.42f, 30);
							ExHitParam(4)->m_nKind = 2;
							ExHitParam(5)->m_nKind = 2;
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
					_switch24_4:
						if (Step(1)++ > 30)
						{
							OnFlag(33u);
							cTaskFade2->Fade(EFadeKind::eFADE_TYPE_IN_WHITE, 1000);
							Step(2)++;
							for (uint num3 = 0u; pModel(num3) != nullptr; num3++)
							{
								::Constants::jetGeoVecUnit(pModel(num3)->vScale());
							}
						}
						break;
					case 5:
					_switch24_5:
						if (cTaskFade2->IsEnd())
						{
							Step()++;
						}
						break;
					}
					break;
				case 5:
				_switch23_5:
					if (Step(1)++ == 50)
					{
						ChangeTaiki();
					}
					break;
				}
				if (ExHitParam(0)->m_nKind != 2)
				{
					vMTrans()[1] = (float)((double)Random::Range(0, 10) * 0.01);
				}
				else
				{
					vMTrans()[1] = 0.0f;
				}
				return true;
			}
		}
	}
	return true;
}

Vector4 cTaskBoss04::vSetRandTarget()
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
		::Constants::jetGeoVecCopy(m_vSTarget, vMTrans());
	}
	else
	{
		MoveSEForce();
	}
	return m_vSTarget;
}

bool cTaskBoss04::IsRingUP(int n)
{
	if (!(m_RingInfo[n]->m_vRingPos[1] >= 2.0f))
	{
		return false;
	}
	return true;
}

bool cTaskBoss04::IsAllRingUP()
{
	for (int i = 0; i < 4; i++)
	{
		if (!IsRingUP(i))
		{
			return false;
		}
	}
	return true;
}

void cTaskBoss04::LampBurn(int n)
{
	if (m_fPillarMot[n] < 5.0f || m_fPillarMot[n] > 10.0f)
	{
		m_fPillarMot[n] = 5.0f;
	}
	else if ((m_fPillarMot[n] += 0.5f) > 10.0f)
	{
		m_fPillarMot[n] = 10.0f;
	}
}

void cTaskBoss04::LampBurn()
{
	for (int i = 0; i < 4; i++)
	{
		LampBurn(i);
	}
}

void cTaskBoss04::LampBlink(int n)
{
	m_fPillarMot[n]++;
	if (m_fPillarMot[n] < 1.0f)
	{
		m_fPillarMot[n] = 1.0f;
	}
	else if (m_fPillarMot[n] >= 15.0f)
	{
		m_fPillarMot[n] = 5.0f;
	}
}

void cTaskBoss04::LampBlink()
{
	for (int i = 0; i < 4; i++)
	{
		LampBlink(i);
	}
}

void cTaskBoss04::LampDell(int n)
{
	if (1.0f <= m_fPillarMot[n] && m_fPillarMot[n] < 5.0f)
	{
		if (++m_fPillarMot[n] >= 5.0f)
		{
			m_fPillarMot[n] = 5.0f;
		}
	}
	else if (5.0f < m_fPillarMot[n] && m_fPillarMot[n] < 15.0f)
	{
		if (++m_fPillarMot[n] >= 15.0f)
		{
			m_fPillarMot[n] = 5.0f;
		}
	}
	else
	{
		m_fPillarMot[n] = 5.0f;
	}
}

void cTaskBoss04::LampDell()
{
	for (int i = 0; i < 4; i++)
	{
		LampDell(i);
	}
}

void cTaskBoss04::LampForceDell(int n)
{
	m_fPillarMot[n] = 5.0f;
}

void cTaskBoss04::LampForceDell()
{
	for (int i = 0; i < 4; i++)
	{
		LampForceDell(i);
	}
}

void cTaskBoss04::RefAttacked(int nParts, int nGrobalID)
{
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)nGrobalID);
	if (cTaskChar2 == nullptr)
	{
		return;
	}
	bool flag = true;
	if (RefHistoryOffset(1)->m_nRefID != 0 && RefHistoryOffset(1)->m_nCount > Constants::get_GAME_COUNTER() - 5)
	{
		flag = false;
	}
	if (1 <= nParts && nParts <= 4)
	{
		bool flag2 = false;
		Constants::MO_RING_INFO* mO_RING_INFO = m_RingInfo[nParts - 1];
		if (mO_RING_INFO->m_vRingPos[1] < 2.0f)
		{
			float num = 2.0f / (float)m_nRingHP;
			if (cTaskChar2->bTestFlagOLD(4096u))
			{
				num *= 2.0f;
			}
			mO_RING_INFO->m_fGoPosY += num;
			if (mO_RING_INFO->m_fGoPosY > 2.0f)
			{
				mO_RING_INFO->m_fGoPosY = 2.0f;
			}
			mO_RING_INFO->m_fUPSp = num * 0.3f;
			if (mO_RING_INFO->m_vRingPos[1] < fCenterPosY() + 2.0f)
			{
				flag2 = true;
			}
		}
		if (flag2)
		{
			SpawnBossColEffect(cTaskChar2, 32);
		}
		else if (flag)
		{
			SpawnBossColEffect(cTaskChar2);
		}
	}
	else if (5 <= nParts && nParts <= 6)
	{
		if (flag)
		{
			SpawnBossColEffect(cTaskChar2);
		}
	}
	else if (!bTestFlag(256u) && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u) && (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(131072u) || KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(384u)))
	{
		if (flag)
		{
			m_nHitFlash = 15;
			nHp() -= HitDamage(cTaskChar2, g_HitDamage);
			if (nHp() <= 0)
			{
				SetMode(9);
			}
			else
			{
				SetMortion(0u, "ANM_CHAR_BOSS_4_A01");
			}
			SpawnBossColEffect_Core(cTaskChar2);
		}
	}
	else if (flag)
	{
		SpawnBossColEffect(cTaskChar2);
	}
}

void cTaskBoss04::SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step)
{
	boss->Step() = step;
}

void cTaskBoss04::SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time)
{
	boss->Step() = 5;
	boss->ReturnStep(step);
	boss->nTimer() = time;
}

void cTaskBoss04::SET_STEP(cTaskCharBoss* boss, ushort step)
{
	cTaskBoss04::SET_STEP_NO_CLEAR(boss, step);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss04::SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time)
{
	cTaskBoss04::SET_STEP_WAIT_NO_CLEAR(boss, step, time);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss04::ModeAppear()
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) != 2 || !bTestFlag(4194304u))
	{
		if (bTestFlag(4194304u))
		{
			SetFrame(3u, m_fLightMot);
		}
		m_fCenterPosY = 0.0f;
		SetApearScale(1.0f, 1.0f, 1.0f);
		Constants::VEC3_SET(vPillarScale(), 1.0f, 1.0f, 1.0f);
		pModel(5u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(6u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(7u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(8u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(5u)->SetFogIdx(2);
		pModel(6u)->SetFogIdx(2);
		pModel(7u)->SetFogIdx(2);
		pModel(8u)->SetFogIdx(2);
		pModel(1u)->SetFogIdx(2);
		pModel(2u)->SetFogIdx(2);
		pModel(3u)->SetFogIdx(2);
		pModel(4u)->SetFogIdx(2);
		InitFogData();
		SetFlash(5, 0.0f);
		SetFlash(6, 0.0f);
		SetFlash(7, 0.0f);
		SetFlash(8, 0.0f);
		SetFlash(1, 0.0f);
		SetFlash(2, 0.0f);
		SetFlash(3, 0.0f);
		SetFlash(4, 0.0f);
		SetMortion(1u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		SetMortion(2u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		SetMortion(3u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		SetMortion(4u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		OffFlag(4194304u);
		cTaskBoss04::SET_MODE_WAIT(this, 2, 10);
		return;
	}
	switch (Step())
	{
	case 0:
		SetApearScale(0.0f, 0.0f, 0.0f);
		::Constants::jetGeoVecUnit(vRotate());
		pModel(5u)->SetFogIdx(3);
		pModel(6u)->SetFogIdx(3);
		pModel(7u)->SetFogIdx(3);
		pModel(8u)->SetFogIdx(3);
		pModel(1u)->SetFogIdx(3);
		pModel(2u)->SetFogIdx(3);
		pModel(3u)->SetFogIdx(3);
		pModel(4u)->SetFogIdx(3);
		SetFlash(5, 1.0f);
		SetFlash(6, 1.0f);
		SetFlash(7, 1.0f);
		SetFlash(8, 1.0f);
		SetFlash(1, 1.0f);
		SetFlash(2, 1.0f);
		SetFlash(3, 1.0f);
		SetFlash(4, 1.0f);
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
			cTaskBoss04::SET_STEP_WAIT(this, 1, 10);
			break;
		}
		break;
	case 1:
		switch (Step(2))
		{
		case 0:
			if (Step(1) < 100)
			{
				if (Step(1) % 30 == 0)
				{
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE039_BOSS_APPEAR3, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				}
				m_fCenterPosY += 0.02f;
				vPillarScale()[1] += 0.01f;
				Vector4& reference = vPillarScale();
				float value = (vPillarScale()[2] = 1.0f);
				reference[0] = value;
				SetApearScale(vPillarScale()[0], vPillarScale()[1], vPillarScale()[2]);
				if (Step(1) % 20 == 0 || Step(1) == 0)
				{
					SpawnAppearEffectCircle(this, Constants::VEC3_GET(-2.6f, 0.0f, 0.0f), 0.8f, 0.7f);
					SpawnAppearEffectCircle(this, Constants::VEC3_GET(2.6f, 0.0f, 0.0f), 0.8f, 0.7f);
					SpawnAppearEffectCircle(this, Constants::VEC3_GET(0.0f, 0.0f, -2.6f), 0.8f, 0.7f);
					SpawnAppearEffectCircle(this, Constants::VEC3_GET(0.0f, 0.0f, 2.6f), 0.8f, 0.7f);
				}
				Step(1)++;
			}
			else
			{
				m_fCenterPosY = 0.0f;
				vPillarScale()[1] = 1.0f;
				SetApearScale(1.0f, 1.0f, 1.0f);
				Step(1) = 0;
				Step(2)++;
			}
			break;
		case 1:
			if (++Step(1) < 30)
			{
				float rate = 1.0f - (float)(int)Step(1) / 30.0f;
				SetFlash(5, rate);
				SetFlash(6, rate);
				SetFlash(7, rate);
				SetFlash(8, rate);
				SetFlash(1, rate);
				SetFlash(2, rate);
				SetFlash(3, rate);
				SetFlash(4, rate);
				break;
			}
			InitFogData();
			cTaskBoss04::SET_STEP_WAIT(this, 2, 10);
			pModel(5u)->SetFogIdx(2);
			pModel(6u)->SetFogIdx(2);
			pModel(7u)->SetFogIdx(2);
			pModel(8u)->SetFogIdx(2);
			pModel(1u)->SetFogIdx(2);
			pModel(2u)->SetFogIdx(2);
			pModel(3u)->SetFogIdx(2);
			pModel(4u)->SetFogIdx(2);
			SetFlash(5, 0.0f);
			SetFlash(6, 0.0f);
			SetFlash(7, 0.0f);
			SetFlash(8, 0.0f);
			SetFlash(1, 0.0f);
			SetFlash(2, 0.0f);
			SetFlash(3, 0.0f);
			SetFlash(4, 0.0f);
			SetMortion(1u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
			SetMortion(2u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
			SetMortion(3u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
			SetMortion(4u, "ANM_CHAR_BOSS_4_A03", 0u, 2u);
			break;
		}
		break;
	case 2:
		if (Step(2) == 0)
		{
			Step(2)++;
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE052_BOSS_CORE, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		}
		if (Step(1)++ == 120)
		{
			SetFrame(3u, m_fLightMot);
			cTaskBoss04::SET_STEP_WAIT(this, 3, 60);
		}
		break;
	case 3:
		if (++Step(1) > 60)
		{
			cTaskBoss04::SET_STEP_WAIT(this, 4, 10);
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
		pModel(5u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(6u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(7u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(8u)->SetUpdateRateMatClr(0u, 1.0f);
		SetMortion(1u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		SetMortion(2u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		SetMortion(3u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		SetMortion(4u, "ANM_CHAR_BOSS_4_A04", 0u, 2u);
		break;
	case 5:
		if (--nTimer() <= 0)
		{
			Step() = ReturnStep();
		}
		break;
	}
}

bool cTaskBoss04::CenterMove()
{
	m_fCenterPosY += m_fCenterSpeed;
	if (m_fCenterPosY > 0.0f)
	{
		m_fCenterSpeed = 0.0f;
		m_fCenterPosY = 0.0f;
		return true;
	}
	if (m_fCenterPosY <= 0.0f && m_fCenterPosY <= -2.0f)
	{
		m_fCenterSpeed = 0.0f;
		m_fCenterPosY = -2.0f;
		return true;
	}
	return false;
}

int cTaskBoss04::NextMode()
{
	return s_MdeTbl[(nAction() + 1) % 18]->m_nMode;
}

void cTaskBoss04::SetNextMode()
{
	if (s_MdeTbl[nAction()]->m_nMode == NextMode())
	{
		StepInit();
	}
	int nWait = s_MdeTbl[nAction()]->m_nWait;
	nAction() = (nAction() + 1) % 18;
	cTaskBoss04::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait);
}

int cTaskBoss04::AjustAccelRotate(float minrot)
{
	uint result = 0u;
	m_fAccelRotSpeed += m_fAddAccelRotSpeed;
	if (m_fAccelRotSpeed >= m_fMaxAccelRotSpeed)
	{
		m_fAccelRotSpeed = m_fMaxAccelRotSpeed;
		result = 1u;
	}
	else if (m_fAccelRotSpeed <= minrot)
	{
		m_fAccelRotSpeed = minrot;
		result = 2u;
	}
	if (!CustomKingdomUtil::IF(s_MdeTbl[nAction()]->m_nWork[0]))
	{
		vRotate()[1] += m_fAccelRotSpeed;
	}
	else
	{
		vRotate()[1] -= m_fAccelRotSpeed;
	}
	return (int)result;
}

int cTaskBoss04::AjustAccelRotate()
{
	return AjustAccelRotate(m_fRotSpeed);
}

}
