#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskBoss02.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/g07/cParamBoss02.hpp"
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

Vector4 cTaskBoss02::s_vPos[7] = {};
int cTaskBoss02::g_HitDamage[6] = {};
const Constants::MO_BOSS_MODE_INFO* cTaskBoss02::s_MdeTbl[20] = {[]() { Constants::MO_BOSS_MODE_INFO* _o1 = new Constants::MO_BOSS_MODE_INFO(); _o1->m_nMode = 3; _o1->m_nMove = -1; _o1->m_nWait = 20; return _o1; }(), []() { Constants::MO_BOSS_MODE_INFO* _o2 = new Constants::MO_BOSS_MODE_INFO(); _o2->m_nMode = 4; _o2->m_nMove = 4; _o2->m_nWait = 0; return _o2; }(), []() { Constants::MO_BOSS_MODE_INFO* _o3 = new Constants::MO_BOSS_MODE_INFO(); _o3->m_nMode = 6; _o3->m_nMove = -1; _o3->m_nWait = 20; return _o3; }(), []() { Constants::MO_BOSS_MODE_INFO* _o4 = new Constants::MO_BOSS_MODE_INFO(); _o4->m_nMode = 6; _o4->m_nMove = -1; _o4->m_nWait = 10; return _o4; }(), []() { Constants::MO_BOSS_MODE_INFO* _o5 = new Constants::MO_BOSS_MODE_INFO(); _o5->m_nMode = 3; _o5->m_nMove = -1; _o5->m_nWait = 20; return _o5; }(), []() { Constants::MO_BOSS_MODE_INFO* _o6 = new Constants::MO_BOSS_MODE_INFO(); _o6->m_nMode = 4; _o6->m_nMove = 5; _o6->m_nWait = 0; return _o6; }(), []() { Constants::MO_BOSS_MODE_INFO* _o7 = new Constants::MO_BOSS_MODE_INFO(); _o7->m_nMode = 6; _o7->m_nMove = -1; _o7->m_nWait = 20; return _o7; }(), []() { Constants::MO_BOSS_MODE_INFO* _o8 = new Constants::MO_BOSS_MODE_INFO(); _o8->m_nMode = 6; _o8->m_nMove = -1; _o8->m_nWait = 10; return _o8; }(), []() { Constants::MO_BOSS_MODE_INFO* _o9 = new Constants::MO_BOSS_MODE_INFO(); _o9->m_nMode = 3; _o9->m_nMove = -1; _o9->m_nWait = 20; return _o9; }(), []() { Constants::MO_BOSS_MODE_INFO* _o10 = new Constants::MO_BOSS_MODE_INFO(); _o10->m_nMode = 4; _o10->m_nMove = 3; _o10->m_nWait = 0; return _o10; }(), []() { Constants::MO_BOSS_MODE_INFO* _o11 = new Constants::MO_BOSS_MODE_INFO(); _o11->m_nMode = 6; _o11->m_nMove = -1; _o11->m_nWait = 20; return _o11; }(), []() { Constants::MO_BOSS_MODE_INFO* _o12 = new Constants::MO_BOSS_MODE_INFO(); _o12->m_nMode = 6; _o12->m_nMove = -1; _o12->m_nWait = 10; return _o12; }(), []() { Constants::MO_BOSS_MODE_INFO* _o13 = new Constants::MO_BOSS_MODE_INFO(); _o13->m_nMode = 3; _o13->m_nMove = -1; _o13->m_nWait = 20; return _o13; }(), []() { Constants::MO_BOSS_MODE_INFO* _o14 = new Constants::MO_BOSS_MODE_INFO(); _o14->m_nMode = 4; _o14->m_nMove = 6; _o14->m_nWait = 0; return _o14; }(), []() { Constants::MO_BOSS_MODE_INFO* _o15 = new Constants::MO_BOSS_MODE_INFO(); _o15->m_nMode = 6; _o15->m_nMove = -1; _o15->m_nWait = 15; return _o15; }(), []() { Constants::MO_BOSS_MODE_INFO* _o16 = new Constants::MO_BOSS_MODE_INFO(); _o16->m_nMode = 6; _o16->m_nMove = -1; _o16->m_nWait = 10; return _o16; }(), []() { Constants::MO_BOSS_MODE_INFO* _o17 = new Constants::MO_BOSS_MODE_INFO(); _o17->m_nMode = 3; _o17->m_nMove = -1; _o17->m_nWait = 20; return _o17; }(), []() { Constants::MO_BOSS_MODE_INFO* _o18 = new Constants::MO_BOSS_MODE_INFO(); _o18->m_nMode = 4; _o18->m_nMove = 0; _o18->m_nWait = 0; return _o18; }(), []() { Constants::MO_BOSS_MODE_INFO* _o19 = new Constants::MO_BOSS_MODE_INFO(); _o19->m_nMode = 6; _o19->m_nMove = -1; _o19->m_nWait = 15; return _o19; }(), []() { Constants::MO_BOSS_MODE_INFO* _o20 = new Constants::MO_BOSS_MODE_INFO(); _o20->m_nMode = 6; _o20->m_nMove = -1; _o20->m_nWait = 15; return _o20; }()};

float cTaskBoss02::get_APPEAR_ADD_NEAR_FOG()
{
	return 0.0f;
}

float cTaskBoss02::get_APPEAR_ADD_FAR_FOG()
{
	return 0.0f;
}

float cTaskBoss02::DAMAGE_BASE_HP(cTaskCharBoss* boss)
{
	return (int)((float)boss->nMaxHp() * 0.6f);
}

float cTaskBoss02::DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss)
{
	return 60.0f / cTaskBoss02::DAMAGE_BASE_HP(boss);
}

float cTaskBoss02::DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss)
{
	return 30.0f / cTaskBoss02::DAMAGE_BASE_HP(boss);
}

void cTaskBoss02::SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time)
{
	boss->nTimer() = time;
	if (boss->nTimer() != 0)
	{
		boss->SetMode(7);
		boss->ReturnMode((ushort)mode);
	}
	else
	{
		boss->SetMode(mode);
	}
}

void cTaskBoss02::SET_MODE_PIYO(cTaskCharBoss* boss, int mode)
{
	boss->SetMode(5);
	boss->ReturnMode((ushort)mode);
}

cTaskBoss02::cTaskBoss02()
{
	Init();
}

Vector4& cTaskBoss02::vRandTarget()
{
	return m_vRandTarget;
}

float& cTaskBoss02::fCenterPosY()
{
	return m_fCenterPosY;
}

Constants::MO_RING_INFO* cTaskBoss02::pRingInfo(uint n)
{
	if ((long)n >= (long)::Constants::ARRAY_SIZE(m_RingInfo))
	{
		return nullptr;
	}
	return m_RingInfo[n];
}

Vector4& cTaskBoss02::vPillarScale()
{
	return m_vPillarScale;
}

byte& cTaskBoss02::nRingAlpha()
{
	return m_nRingAlpha;
}

cKingdomPtkHnd*& cTaskBoss02::pHover(int n)
{
	return m_pHover[n];
}

float& cTaskBoss02::fNowAccel()
{
	return m_fNowAccel;
}

float& cTaskBoss02::fNowMaxSpeed()
{
	return m_fNowMaxSpeed;
}

float& cTaskBoss02::fRandomAccel()
{
	return m_fRandomAccel;
}

float& cTaskBoss02::fRandomMaxSpeed()
{
	return m_fRandomMaxSpeed;
}

void cTaskBoss02::SetApearScale(float x, float y, float z)
{
	Vector4 v = Vector4(x, y, z, 1.0f);
	::Constants::jetGeoVecCopy(pModel(1u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(2u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(3u)->vScale(), v);
	::Constants::jetGeoVecCopy(pModel(4u)->vScale(), v);
}

// DOL 0x8003ca50 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskBoss02::Init()
{
	m_fRotSpeed = 0.0f;
	m_nRandomFrame = 0;
	m_nDownFrame = 0;
	m_nPiyoFrame = 0;
	m_fNowAccel = 0.0f;
	m_fNowMaxSpeed = 0.0f;
	m_fCenterPosY = 0.0f;
	m_nFrameCnt = 0;
	m_nRestTime = 0;
	m_nRingHP = 0;
	m_nHitFlash = 0;
	m_fRandomAccel = 0.0f;
	m_fRandomMaxSpeed = 0.0f;
	m_fLightMot = 0.0f;
	m_fLightMotAdd = 0.5f;
	m_fLightAmb = 0.0f;
	m_fLightAmbAdd = 0.0f;
	nAction() = 0;
	nMovePoint() = 0;
	m_fCenterSpeed = 0.0f;
	m_nMoveSEPitch = 0;
	m_nRoteSEPitch = 0;
	fChargeDamageRate() = 0.0f;
	std::fill(std::begin(m_fPillarMot), std::end(m_fPillarMot), 5.0f); // DOL 0x8003ca50: +0xaf0 / +0xaf4 = FLOAT_8047a128 (5.0)
	LampForceDell();
	std::fill(std::begin(m_pHover), std::end(m_pHover), nullptr);
	std::fill(std::begin(m_RingInfo), std::end(m_RingInfo), nullptr);
	for (int i = 0; i < (int)std::size(m_RingInfo); i++)
	{
		m_RingInfo[i] = new Constants::MO_RING_INFO();
	}
	m_fCameraSin = 0.0f;
	m_fCameraCos = 0.0f;
	fFogNear() = -30.0f;
	fFogFar() = 30.0f;
	SetMode(1);
	ExHitAlloc(3);
	ExHitRegistCircle(0, Constants::VEC3_GET(2.5f, 0.0f, 0.0f), 0.6f, 7);
	ExHitRegistCircle(1, Constants::VEC3_GET(-2.5f, 0.0f, 0.0f), 0.6f, 7);
	ExHitRegistRect(2, Constants::VEC3_GET(-2.5f, 0.0f, -0.3f), 5.0f, 0.6f);
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	Constants::VEC3_SET(s_vPos[0], cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f + 0.5f);
	Constants::VEC3_SET(s_vPos[1], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() / 2 - 3), 0.0f, s_vPos[0][2]);
	Constants::VEC3_SET(s_vPos[2], cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nWidth() / 2 + 3), 0.0f, s_vPos[0][2]);
	Constants::VEC3_SET(s_vPos[3], s_vPos[1][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 - 4));
	Constants::VEC3_SET(s_vPos[4], s_vPos[1][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 + 5));
	Constants::VEC3_SET(s_vPos[5], s_vPos[2][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 - 4));
	Constants::VEC3_SET(s_vPos[6], s_vPos[2][0], 0.0f, cTaskPanelMan2->fPanelSize() * (float)(cTaskPanelMan2->nDepth() / 2 + 5));
	cParamBoss02* cParamBoss6 = new cParamBoss02("CSV_BOSS2_PARAM");
	m_fRotSpeed = cParamBoss6->GetValueF32(1, 4);
	m_nRandomFrame = cParamBoss6->GetValueS32(1, 7);
	m_nDownFrame = cParamBoss6->GetValueS32(1, 8);
	m_nPiyoFrame = cParamBoss6->GetValueS32(1, 9);
	m_nRingHP = cParamBoss6->GetValueS32(1, 10);
	m_fRandomAccel = cParamBoss6->GetValueF32(1, 5);
	m_fRandomMaxSpeed = cParamBoss6->GetValueF32(1, 6);
	nHp() = cParamBoss6->GetValueS32(1, 1);
	nHp() *= 100;
	nBombMax() = cParamBoss6->GetValueS32(1, 2);
	nBombTime() = cParamBoss6->GetValueS32(1, 3);
	nScore() = cParamBoss6->GetValueS32(1, 11);
	nMaxHp() = nHp();
	cParamDamage* cParamDamage2 = new cParamDamage("CSV_HIT_DAMAGE");
	for (int j = 0; j < 6; j++)
	{
		g_HitDamage[j] = (int)(cParamDamage2->GetValueF32(j + 1, 2) * 100.0f);
	}
	fChargeDamageRate() = cParamDamage2->GetValueF32(7, 2);
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay() && KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP > 0)
	{
		nHp() = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP;
	}
	if ((float)nHp() < cTaskBoss02::DAMAGE_BASE_HP(this))
	{
		fCoreFogNear() = 30.0f - cTaskBoss02::DAMAGE_ADD_NEAR_FOG(this) * (cTaskBoss02::DAMAGE_BASE_HP(this) - (float)nHp());
		fCoreFogFar() = 60.0f - cTaskBoss02::DAMAGE_ADD_FAR_FOG(this) * (cTaskBoss02::DAMAGE_BASE_HP(this) - (float)nHp());
	}
	if (m_nRingHP == 0)
	{
		m_nRingHP = 1;
	}
	CreateBomb();
	nCoreFogColor() = 4278190335u;
	m_nRingAlpha = 0;
	Constants::VEC3_SET(m_vPillarScale, 0.0f, 0.0f, 0.0f);
	OnFlag(4194304u);
}

void cTaskBoss02::Term()
{
	cTaskCharBoss::Term();
}

bool cTaskBoss02::ActionBoth()
{
	return true;
}

void cTaskBoss02::SetRingDamageFrame(int nRing)
{
	uint array[2] = {2u, 1u};
	uint obj[2] = {4u, 3u};
	uint idx = array[nRing];
	uint idx2 = obj[nRing];
	if (GetMode() == 5 || IsRingUP(nRing))
	{
		pModel(idx)->SetFrameMatClr(1u, 140.0f); // DOL 0x8003d2c4 [audited: per AUDIT_G03_G07 row]: FLOAT_8047a174 = 140 frames (the port had the C#'s 2.333 seconds)
		pModel(idx2)->SetFrameMatClr(0u, 140.0f);
	}
	else
	{
		float num = m_RingInfo[nRing]->m_vRingPos[1] / 2.0f * 139.0f;
		pModel(idx)->SetFrameMatClr(1u, num);   // native frames (the C# passed Unity seconds)
		pModel(idx2)->SetFrameMatClr(0u, num);   // native frames (the C# passed Unity seconds)
	}
}

// DOL 0x8003d2c4 (compared 2026-10-30)
bool cTaskBoss02::ActionLocal()
{
	for (int i = 0; i < 2; i++)
	{
		if (pHover(i) != nullptr)
		{
			Vector4 v = Vector4();
			pModel(1u)->GetLocalNodeTrans(v, "effect");
			::Constants::jetGeoVecRotY(v, v, vRotate()[1] + ::Constants::JET_DEG2RAD(180.0f) * (float)i);
			::Constants::jetGeoVecAdd(v, v, vMTrans());
			pHover(i)->vSetTrans(v);
		}
	}
	for (int j = 0; j < 2; j++)
	{
		SetRingDamageFrame(j);
		Constants::MO_RING_INFO* mO_RING_INFO = m_RingInfo[j];
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
		IsRingUP(j);
	}
	if (m_fCenterPosY != -2.0f)
	{
		fHitSize() = fDefHitSize();
		OnFlag(256u);
		ExHitParam(2)->m_nFlag = 4;
	}
	else
	{
		fHitSize() = fDefHitSize();
		OffFlag(256u);
		ExHitParam(2)->m_nFlag = 1;
	}
	if (CustomKingdomUtil::IF(m_nHitFlash))
	{
		m_nHitFlash--;
		int value = m_nHitFlash % 8 / 4;
		if (!CustomKingdomUtil::IF(m_nHitFlash))
		{
			pModel()->SetFogIdx(2);
			pModel(5u)->SetFogIdx(2);
			SetMortion(0u, "ANM_CHAR_BOSS_2_A00");
			SetFlash(0, 0.0f);
			SetFlash(5, 0.0f);
		}
		else if (CustomKingdomUtil::IF(value))
		{
			pModel()->SetFogIdx(3);
			pModel(5u)->SetFogIdx(3);
			SetFlash(0, 1.0f);
			SetFlash(5, 1.0f);
		}
		else
		{
			pModel()->SetFogIdx(2);
			pModel(5u)->SetFogIdx(2);
			SetFlash(0, 0.0f);
			SetFlash(5, 0.0f);
		}
	}
	return ExecModeFunc() != 0;
}

bool cTaskBoss02::ActionOther()
{
	return true;
}

// DOL 0x8003d86c [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskBoss02::AdjustMaxSpeed()
{
	float num = ::Constants::jetGeoVecLength(vSpeed());
	if (num > fNowMaxSpeed())
	{
		num = fNowMaxSpeed() / num;
		Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
	}
}

void cTaskBoss02::DrawStartDirection()
{
}

void cTaskBoss02::DrawEndDirection()
{
}

void cTaskBoss02::DrawInGame()
{
}

void cTaskBoss02::SpawnHover()
{
	Vector4 _out = Vector4();
	for (int i = 0; i < 2; i++)
	{
		if (pHover(i) == nullptr)
		{
			Constants::VEC3_SUB(_out, _out, vMTrans());
			Constants::tspVector3RotY(_out, _out, Constants::TSP_DEG2RAD(180 * i));
			Constants::VEC3_ADD(_out, _out, vMTrans());
			pHover(i) = Constants::SpawnEffect(27, pModel((uint)(1 + i))->vTrans(), nullptr, 0, false);
		}
	}
}

void cTaskBoss02::MoveSE()
{
	if (++m_nMoveSEPitch >= 25)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		m_nMoveSEPitch = 0;
	}
}

void cTaskBoss02::MoveSEForce()
{
	SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
	m_nMoveSEPitch = 0;
}

void cTaskBoss02::RoteSE()
{
	if (++m_nRoteSEPitch >= 110)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		m_nRoteSEPitch = 0;
	}
}

// DOL 0x8003d8dc [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskBoss02::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
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
					cTaskBoss02::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, s_MdeTbl[nAction()]->m_nWait);
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
					::Constants::jetGeoVecCopy(vRandTarget(), vSetRandTarget());
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
				if (Constants::motosCharMove(this, vRandTarget(), 0.05f, m_fNowAccel))
				{
					::Constants::jetGeoVecCopy(vRandTarget(), vSetRandTarget());
				}
				vRotate()[1] += m_fRotSpeed;
				if (IsAllRingUP())
				{
					m_nRestTime = nTimer();
					cTaskBoss02::SET_MODE_PIYO(this, 3);
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
					if ((value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(0.0f), m_fRotSpeed)) != -1.0f || (value = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(180.0f), m_fRotSpeed)) != -1.0f)
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
					cTaskBoss02::SET_MODE_PIYO(this, 4);
				}
				MoveSE();
				return true;
			}
		}
		else if (6 == GetMode())
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
				for (int i = 0; i < 2; i++)
				{
					if (!IsRingUP(i))
					{
						LampDell(i);
					}
				}
				switch (Step(1))
				{
				case 0:
					{
						float num = m_fRotSpeed * 1.5f;
						float value2;
						if ((value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(0.0f), num)) != -1.0f || (value2 = IsAdjustRotate(vRotate()[1], Constants::TSP_DEG2RAD(180.0f), num)) != -1.0f)
						{
							vRotate()[1] = value2;
							Step(1)++;
							break;
						}
						vRotate()[1] += num;
						if (IsAllRingUP())
						{
							cTaskBoss02::SET_MODE_PIYO(this, 6);
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
						SpawnBomb(9, 4.0f, 0.3f, fDefHitSize());
						break;
					case 5:
						SpawnBomb(6, 4.0f, 0.3f, fDefHitSize());
						break;
					case 3:
						SpawnBomb(10, 4.0f, 0.3f, fDefHitSize());
						break;
					case 6:
						SpawnBomb(5, 4.0f, 0.3f, fDefHitSize());
						break;
					case 0:
						SpawnBomb(5, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(6, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(9, 4.0f, 0.3f, fDefHitSize());
						SpawnBomb(10, 4.0f, 0.3f, fDefHitSize());
						break;
					}
					if (NextMode() == 6)
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
		else if (5 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				m_fPiyoDWSp = 0.001f;
				m_fLightMotAdd = 0.0f;
				m_fLightAmbAdd = 0.0f - m_fLightAmb / 150.0f;
				m_nFrameCnt = 0;
				return true;
			}
			if (3 == nMes)
			{
				LampForceDell();
				SetFrame(3u, m_fLightMot);
				m_fLightMotAdd = 0.5f;
				return true;
			}
			if (1 == nMes)
			{
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
						Step(1)++;
					}
					if (m_nPiyoFrame - Step(3) < 90)
					{
						LampBlink();
					}
					break;
				case 4:
					LampDell();
					m_fCenterPosY += 0.05f;
					if (++m_nFrameCnt == 40)
					{
						m_fCenterPosY = 0.0f;
						SetMode(ReturnMode());
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
					cTaskBoss02::SET_MODE_PIYO(this, ReturnMode());
				}
				return true;
			}
		}
		else if (8 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE024_BOSS_ROT1);
				SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE023_BOSS_MOVE1);
				fCoreFogNear() = -30.0f;
				fCoreFogFar() = 30.0f;
				for (int j = 0; j < 2; j++)
				{
					if (pHover(j) != nullptr)
					{
						Vector4 v = Vector4();
						pHover(j)->vSetTrans(v);
						pHover(j)->vSetScale(v);
						pHover(j)->vSetRotate(v);
						pHover(j)->Detach(true);
						pHover(j) = nullptr;
					}
				}
				m_fCenterPosY = -2.0f;
				BombAllExplode();
				SetMortionReverse(1u, "ANM_CHAR_BOSS_2_A03", 0u, 2u);
				SetMortionReverse(2u, "ANM_CHAR_BOSS_2_A03", 0u, 2u);
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
				_switch24_0:
					SpawnExplode2Random(this, Constants::VEC3_GET(-3.0f, 0.0f, 0.0f), 1.0f, 2.0f, 0.5f, 210);
					SpawnExplode2Random(this, Constants::VEC3_GET(2.5f, 0.0f, 0.0f), 1.0f, 2.0f, 0.5f, 210);
					Step()++;
					goto _switch24_1;
				case 1:
				_switch24_1:
					{
						bool flag = true;
						if (Step(1) < 120) // DOL 0x8003d8dc mode 8: 0x78 / 0x5a / 0x50 / 0x28 / 0x14 (the port had half the counts)
						{
							Step(1)++;
							flag = false;
						}
						if (Step(2) < 90)
						{
							Step(2)++;
							flag = false;
						}
						if (flag)
						{
							Step(1) = 0;
							Step(2) = 0;
							Step()++;
						}
						ExplodeSE();
						break;
					}
				case 2:
				_switch24_2:
					if (++Step(1) > 80)
					{
						Step(1) = 0;
						Step()++;
					}
					ExplodeSE();
					break;
				case 3:
				_switch24_3:
					SpawnExplode2(this, Constants::VEC3_GET(-3.0f, 0.0f, 0.0f), 1.0f, 3.0f, 0.5f);
					SpawnExplode2(this, Constants::VEC3_GET(2.5f, 0.0f, 0.0f), 1.0f, 3.0f, 0.5f);
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE047_BOSS_FALL, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					Step()++;
					Step(1) = 0;
					break;
				case 4:
				_switch24_4:
					switch (Step(2))
					{
					case 0:
					_switch25_0:
						ExHitParam(0)->m_nKind = 2;
						ExHitParam(1)->m_nKind = 2;
						Step(1) = 0;
						Step(2)++;
						cTaskFade2->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE, 2000);
						goto _switch25_1;
					case 1:
					_switch25_1:
						if (++Step(1) > 40)
						{
							Constants::SpawnEffect(21, vMTrans());
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.2f, 20);
							Step(1) = 0;
							Step(2)++;
						}
						break;
					case 2:
					_switch25_2:
						if (++Step(1) > 20)
						{
							Constants::SpawnEffect(21, vMTrans());
							KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.2f, 20);
							Step(1) = 0;
							Step(2)++;
						}
						break;
					case 3:
					_switch25_3:
						if (!cTaskFade2->IsEnd())
						{
							// DOL: the counter restarts after each burst while the fade runs (the port fired once)
							if (++Step(1) > 40)
							{
								Constants::SpawnEffect(21, vMTrans());
								KingdomGlobal::g_pMotosGame->nScore() += nScore();
								KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.42f, 30);
								ExHitParam(2)->m_nKind = 2;
								Step(1) = 0;
								OnFlag(33u);
							}
						}
						else
						{
							Step(1) = 0;
							Step(2)++;
						}
						break;
					case 4:
					_switch25_4:
						if (Step(1)++ > 60)
						{
							OnFlag(33u);
							cTaskFade2->Fade(EFadeKind::eFADE_TYPE_IN_WHITE, 2000);
							Step(2)++;
							for (uint num2 = 0u; pModel(num2) != nullptr; num2++)
							{
								::Constants::jetGeoVecUnit(pModel(num2)->vScale());
							}
						}
						break;
					case 5:
					_switch25_5:
						if (cTaskFade2->IsEnd())
						{
							Step()++;
						}
						break;
					}
					break;
				case 5:
				_switch24_5:
					if (Step(1)++ == 100)
					{
						ChangeTaiki();
					}
					break;
				}
				if (ExHitParam(0)->m_nKind != 2)
				{
					vMTrans()[1] = (float)((double)kingdom::MslRandRange(0, 10) * 0.01);
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

// DOL 0x8003f5ec [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
Vector4 cTaskBoss02::vSetRandTarget()
{
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	float num = cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth();
	float num2 = cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth();
	::Constants::jetGeoVecUnit(m_vSTarget);
	float n = (float)kingdom::MslRandRange(0, 100) * 0.1f + 1.0f;
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

bool cTaskBoss02::IsRingUP(int n)
{
	if (!(m_RingInfo[n]->m_vRingPos[1] >= 2.0f))
	{
		return false;
	}
	return true;
}

bool cTaskBoss02::IsAllRingUP()
{
	for (int i = 0; i < 2; i++)
	{
		if (!IsRingUP(i))
		{
			return false;
		}
	}
	return true;
}

void cTaskBoss02::LampBurn(int n)
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

void cTaskBoss02::LampBurn()
{
	for (int i = 0; i < 2; i++)
	{
		LampBurn(i);
	}
}

void cTaskBoss02::LampBlink(int n)
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

void cTaskBoss02::LampBlink()
{
	for (int i = 0; i < 2; i++)
	{
		LampBlink(i);
	}
}

void cTaskBoss02::LampDell(int n)
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

void cTaskBoss02::LampDell()
{
	for (int i = 0; i < 2; i++)
	{
		LampDell(i);
	}
}

void cTaskBoss02::LampForceDell(int n)
{
	m_fPillarMot[n] = 5.0f;
}

void cTaskBoss02::LampForceDell()
{
	for (int i = 0; i < 2; i++)
	{
		LampForceDell(i);
	}
}

// DOL 0x8003f80c [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskBoss02::RefAttacked(int nParts, int nGrobalID)
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
	switch (nParts)
	{
	case 1:
	case 2:
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
			return;
		}
	case 3:
		if (flag)
		{
			SpawnBossColEffect(cTaskChar2);
		}
		return;
	}
	if (!bTestFlag(256u) && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u) && (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(131072u) || KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(384u)))
	{
		if (flag)
		{
			m_nHitFlash = 15;
			nHp() -= HitDamage(cTaskChar2, g_HitDamage);
			if (nHp() <= 0)
			{
				SetMode(8);
			}
			else
			{
				SetMortion(0u, "ANM_CHAR_BOSS_2_A01");
			}
			SpawnBossColEffect_Core(cTaskChar2);
		}
	}
	else if (flag)
	{
		SpawnBossColEffect(cTaskChar2);
	}
}

void cTaskBoss02::SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step)
{
	boss->Step() = step;
}

void cTaskBoss02::SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time)
{
	boss->Step() = 5;
	boss->ReturnStep(step);
	boss->nTimer() = time;
}

void cTaskBoss02::SET_STEP(cTaskCharBoss* boss, ushort step)
{
	cTaskBoss02::SET_STEP_NO_CLEAR(boss, step);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss02::SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time)
{
	cTaskBoss02::SET_STEP_WAIT_NO_CLEAR(boss, step, time);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

// DOL 0x8003faec [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskBoss02::ModeAppear()
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
		SetMaterialAlpha(0u, std::numeric_limits<byte>::max());
		SetMaterialAlpha(5u, std::numeric_limits<byte>::max());
		SetMaterialAlpha(6u, std::numeric_limits<byte>::max());
		SetMortion(1u, "ANM_CHAR_BOSS_2_A04", 0u, 2u);
		SetMortion(2u, "ANM_CHAR_BOSS_2_A04", 0u, 2u);
		pModel(1u)->SetUpdateRateMatClr(1u, 0.0f);
		pModel(2u)->SetUpdateRateMatClr(1u, 0.0f);
		pModel(3u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(4u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(1u)->SetFogIdx(2);
		pModel(2u)->SetFogIdx(2);
		pModel(3u)->SetFogIdx(2);
		pModel(4u)->SetFogIdx(2);
		SetFlash(1, 0.0f);
		SetFlash(2, 0.0f);
		SetFlash(3, 0.0f);
		SetFlash(4, 0.0f);
		InitFogData();
		OffFlag(4194304u);
		cTaskBoss02::SET_MODE_WAIT(this, 2, 10);
		return;
	}
	KingdomGlobal::g_pMotosGame->pCamera();
	(void)(20.0f);
	(void)(20.0f);
	float num = 0.02f;
	m_fCameraSin += 0.001f;
	switch (Step())
	{
	case 0:
		SetApearScale(0.0f, 0.0f, 0.0f);
		switch (Step(1))
		{
		case 0:
			m_fCameraSin -= 0.3f;
			num = 0.02f;
			Step(1)++;
			break;
		case 1:
			pModel(1u)->SetFogIdx(3);
			pModel(2u)->SetFogIdx(3);
			pModel(3u)->SetFogIdx(3);
			pModel(4u)->SetFogIdx(3);
			SetFlash(1, 1.0f);
			SetFlash(2, 1.0f);
			SetFlash(3, 1.0f);
			SetFlash(4, 1.0f);
			pModel(1u)->SetUpdateRateMatClr(0u, 0.0f);
			pModel(2u)->SetUpdateRateMatClr(0u, 0.0f);
			cTaskBoss02::SET_STEP_WAIT(this, 1, 10);
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
					SpawnAppearEffectCircle(this, Constants::VEC3_GET(-2.5f, 0.0f, 0.0f), 0.8f, 0.4f); // DOL 0x8003faec: FLOAT_8047a1f4 / a1f8 = 0.8 / 0.4
					SpawnAppearEffectCircle(this, Constants::VEC3_GET(2.5f, 0.0f, 0.0f), 0.8f, 0.4f); // DOL 0x8003faec: FLOAT_8047a1f4 / a1f8 = 0.8 / 0.4
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
			if (++Step(1) < 60)
			{
				FogStep(5.0f); // DOL 0x80040268 [audited: described by the 43rd-session pass] (FLOAT_8047a128 = 5): the flash fog's end distance grows 5 per frame for 60 frames
				break;
			}
			pModel(1u)->SetFogIdx(2);
			pModel(2u)->SetFogIdx(2);
			pModel(3u)->SetFogIdx(2);
			pModel(4u)->SetFogIdx(2);
			SetFlash(1, 0.0f);
			SetFlash(2, 0.0f);
			SetFlash(3, 0.0f);
			SetFlash(4, 0.0f);
			InitFogData();
			cTaskBoss02::SET_STEP_WAIT(this, 2, 10);
			SetMortion(1u, "ANM_CHAR_BOSS_2_A03", 0u, 2u);
			SetMortion(2u, "ANM_CHAR_BOSS_2_A03", 0u, 2u);
			break;
		}
		break;
	case 2:
		SetMaterialAlpha(0u, std::numeric_limits<byte>::max());
		SetMaterialAlpha(5u, std::numeric_limits<byte>::max());
		SetMaterialAlpha(6u, std::numeric_limits<byte>::max());
		if (Step(2) == 0)
		{
			Step(2)++;
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE052_BOSS_CORE, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		}
		if (++Step(1) >= 120)
		{
			SetFrame(3u, m_fLightMot);
			cTaskBoss02::SET_STEP_WAIT(this, 3, 60);
		}
		break;
	case 3:
		if (++Step(1) > 60)
		{
			cTaskBoss02::SET_STEP_WAIT(this, 4, 10);
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
		pModel(1u)->SetUpdateRateMatClr(1u, 0.0f);
		pModel(2u)->SetUpdateRateMatClr(1u, 0.0f);
		pModel(3u)->SetUpdateRateMatClr(0u, 1.0f);
		pModel(4u)->SetUpdateRateMatClr(0u, 1.0f);
		SetMortion(1u, "ANM_CHAR_BOSS_2_A04", 0u, 2u);
		SetMortion(2u, "ANM_CHAR_BOSS_2_A04", 0u, 2u);
		break;
	case 5:
		if (--nTimer() <= 0)
		{
			Step() = ReturnStep();
		}
		break;
	}
}

bool cTaskBoss02::CenterMove()
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

int cTaskBoss02::NextMode()
{
	return s_MdeTbl[(nAction() + 1) % 20]->m_nMode;
}

void cTaskBoss02::SetNextMode()
{
	if (s_MdeTbl[nAction()]->m_nMode == NextMode())
	{
		StepInit();
	}
	int nWait = s_MdeTbl[nAction()]->m_nWait;
	nAction() = (nAction() + 1) % 20;
	cTaskBoss02::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait);
}

}
