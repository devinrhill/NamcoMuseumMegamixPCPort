#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskBoss05.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskFade.hpp"
#include "kingdom/EFadeKind.hpp"
#include "kingdom/game/g07/cParamBoss05.hpp"
#include "kingdom/game/g07/cParamDamage.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBoss05Last.hpp"
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

Vector4 cTaskBoss05::s_vPos[9] = {};
int cTaskBoss05::g_HitDamage[6] = {};
const Constants::MO_BOSS_MODE_INFO* cTaskBoss05::s_MdeTbl[12] = {[]() { Constants::MO_BOSS_MODE_INFO* _o1 = new Constants::MO_BOSS_MODE_INFO(); _o1->m_nMode = 3; _o1->m_nWait = 10; return _o1; }(), []() { Constants::MO_BOSS_MODE_INFO* _o2 = new Constants::MO_BOSS_MODE_INFO(); _o2->m_nMode = 7; _o2->m_nWait = 20; return _o2; }(), []() { Constants::MO_BOSS_MODE_INFO* _o3 = new Constants::MO_BOSS_MODE_INFO(); _o3->m_nMode = 4; _o3->m_nWait = 20; return _o3; }(), []() { Constants::MO_BOSS_MODE_INFO* _o4 = new Constants::MO_BOSS_MODE_INFO(); _o4->m_nMode = 3; _o4->m_nWait = 10; return _o4; }(), []() { Constants::MO_BOSS_MODE_INFO* _o5 = new Constants::MO_BOSS_MODE_INFO(); _o5->m_nMode = 7; _o5->m_nWait = 20; return _o5; }(), []() { Constants::MO_BOSS_MODE_INFO* _o6 = new Constants::MO_BOSS_MODE_INFO(); _o6->m_nMode = 4; _o6->m_nWait = 20; return _o6; }(), []() { Constants::MO_BOSS_MODE_INFO* _o7 = new Constants::MO_BOSS_MODE_INFO(); _o7->m_nMode = 5; _o7->m_nWait = 10; return _o7; }(), []() { Constants::MO_BOSS_MODE_INFO* _o8 = new Constants::MO_BOSS_MODE_INFO(); _o8->m_nMode = 5; _o8->m_nWait = 10; return _o8; }(), []() { Constants::MO_BOSS_MODE_INFO* _o9 = new Constants::MO_BOSS_MODE_INFO(); _o9->m_nMode = 5; _o9->m_nWait = 10; return _o9; }(), []() { Constants::MO_BOSS_MODE_INFO* _o10 = new Constants::MO_BOSS_MODE_INFO(); _o10->m_nMode = 4; _o10->m_nWait = 30; return _o10; }(), []() { Constants::MO_BOSS_MODE_INFO* _o11 = new Constants::MO_BOSS_MODE_INFO(); _o11->m_nMode = 8; _o11->m_nWait = 5; return _o11; }(), []() { Constants::MO_BOSS_MODE_INFO* _o12 = new Constants::MO_BOSS_MODE_INFO(); _o12->m_nMode = 9; _o12->m_nWait = 30; return _o12; }()};
float cTaskBoss05::fRotatePow = 0.0f;
float cTaskBoss05::fMovePoints = 0.0f;
float cTaskBoss05::fMovePoints2 = 0.0f;

float cTaskBoss05::DAMAGE_BASE_HP(cTaskCharBoss* boss)
{
	return (int)((float)boss->nMaxHp() * 0.6f);
}

float cTaskBoss05::DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss)
{
	return 60.0f / cTaskBoss05::DAMAGE_BASE_HP(boss);
}

float cTaskBoss05::DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss)
{
	return 30.0f / cTaskBoss05::DAMAGE_BASE_HP(boss);
}

void cTaskBoss05::SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time)
{
	boss->nTimer() = time;
	if (boss->nTimer() != 0)
	{
		boss->SetMode(11);
		boss->ReturnMode((ushort)mode);
	}
	else
	{
		boss->SetMode(mode);
	}
}

void cTaskBoss05::SET_MODE_PIYO(cTaskCharBoss* boss, int mode)
{
	boss->SetMode(6);
	boss->ReturnMode((ushort)mode);
}

cTaskBoss05::cTaskBoss05()
{
	Init();
}

float& cTaskBoss05::fRushAccel()
{
	return m_fRushAccel;
}

float& cTaskBoss05::fRushSpeed()
{
	return m_fRushSpeed;
}

float& cTaskBoss05::fRotSpeed()
{
	return m_fRushSpeed;
}

float& cTaskBoss05::fNowAccel()
{
	return m_fNowAccel;
}

float& cTaskBoss05::fNowMaxSpeed()
{
	return m_fNowMaxSpeed;
}

float& cTaskBoss05::fCenterPosY()
{
	return m_fCenterPosY;
}

Constants::MO_RING_INFO*& cTaskBoss05::pRingInfo()
{
	return m_RingInfo;
}

bool cTaskBoss05::IsLastForm()
{
	return m_IsLastForm;
}

void cTaskBoss05::SetApearvPos(Vector4 v)
{
	uint num = 0u;
	while (true)
	{
		switch (num)
		{
		case 0u:
		case 6u:
		case 7u:
		case 8u:
		case 9u:
		case 10u:
			::Constants::jetGeoVecCopy(pModel(num)->vScale(), v);
			break;
		case 1u:
		case 2u:
		case 3u:
		case 4u:
		case 5u:
			break;
		default:
			return;
		}
		num++;
	}
}

void cTaskBoss05::SetApearScale(float x, float y, float z)
{
	Vector4 v = Vector4(x, y, z, 1.0f);
	uint num = 0u;
	while (true)
	{
		switch (num)
		{
		case 1u:
		case 2u:
		case 3u:
		case 4u:
		case 5u:
			::Constants::jetGeoVecCopy(pModel(num)->vScale(), v);
			break;
		case 0u:
		case 6u:
		case 7u:
		case 8u:
		case 9u:
		case 10u:
			break;
		default:
			return;
		}
		num++;
	}
}

void cTaskBoss05::HeadRotate(Vector4 v)
{
	::Constants::jetGeoVecCopy(pModel()->vRotate(), v);
	::Constants::jetGeoVecCopy(pModel(6u)->vRotate(), v);
	::Constants::jetGeoVecCopy(pModel(7u)->vRotate(), v);
	::Constants::jetGeoVecCopy(pModel(8u)->vRotate(), v);
	::Constants::jetGeoVecCopy(pModel(9u)->vRotate(), v);
	::Constants::jetGeoVecCopy(pModel(10u)->vRotate(), v);
}

// DOL 0x80049114 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskBoss05::Init()
{
	m_nPiyoFrame = 0;
	m_fNowAccel = 0.0f;
	m_fNowMaxSpeed = 0.0f;
	m_fRushAccel = 0.0f;
	m_fRushSpeed = 0.0f;
	m_fCenterPosY = 0.0f;
	m_fCenterSpeed = 0.0f;
	m_nFrameCnt = 0;
	m_nRingHP = 0;
	m_fPillarMot = 0.0f;
	m_nHitFlash = 0;
	m_IsDaiAnim = false;
	m_fBouHitPower = 0.0f;
	m_fDodaiHitPower = 0.0f;
	m_fDodaiHitSize = 0.0f;
	nAction() = 0;
	nMovePoint() = 0;
	m_fSlopeMaxRad = 0.0f;
	m_fSlopeAccel = 0.0f;
	m_fLightMot = 0.0f;
	m_fLightMotAdd = 0.5f;
	m_fLightAmb = 0.0f;
	m_fLightAmbAdd = 0.0f;
	fChargeDamageRate() = 0.0f;
	std::fill(std::begin(m_BouMotInfo), std::end(m_BouMotInfo), nullptr);
	for (int i = 0; i < (int)std::size(m_BouMotInfo); i++)
	{
		m_BouMotInfo[i] = new cTaskBoss05::MO_BOU_MOT_INFO();
	}
	m_RingInfo = new Constants::MO_RING_INFO();
	m_fScaleAcsel = 0.0f;
	m_fGoScale = 0.0f;
	m_fUpScale = 0.0f;
	m_fTopAlpha = 0.0f;
	::Constants::jetGeoVecUnit(m_vApearScale);
	nCoreFogColor() = 4278190335u;
	m_fPiyoDWSp = 0.001f;
	m_IsLastForm = false;
	m_nDeathCnt = 0;
	cTaskPanelMan* cTaskPanelMan2 = KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan();
	Constants::VEC3_SET(s_vPos[0], cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nWidth() / 2.0f, 0.0f, cTaskPanelMan2->fPanelSize() * (float)cTaskPanelMan2->nDepth() / 2.0f + 0.5f);
	Constants::VEC3_SET(s_vPos[1], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) - 3.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) + 3.5f));
	Constants::VEC3_SET(s_vPos[2], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) - 3.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) - 2.5f));
	Constants::VEC3_SET(s_vPos[3], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) + 3.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) - 2.5f));
	Constants::VEC3_SET(s_vPos[4], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) + 3.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) + 3.5f));
	Constants::VEC3_SET(s_vPos[5], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) - 4.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) + 4.5f));
	Constants::VEC3_SET(s_vPos[6], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) - 4.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) - 3.5f));
	Constants::VEC3_SET(s_vPos[7], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) + 4.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) - 3.5f));
	Constants::VEC3_SET(s_vPos[8], cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nWidth() / 2) + 4.5f), 0.0f, cTaskPanelMan2->fPanelSize() * ((float)(cTaskPanelMan2->nDepth() / 2) + 4.5f));
	KingdomGlobal::g_pMotosGame->pBoss() = this;
	m_BouMotInfo[0]->m_nModel = 0;
	m_BouMotInfo[1]->m_nModel = 2;
	m_BouMotInfo[2]->m_nModel = 5;
	std::fill(std::begin(m_pBossPtk), std::end(m_pBossPtk), nullptr);
	cParamBoss05* cParamBoss6 = new cParamBoss05("CSV_BOSS5_1_PARAM");
	m_fRushSpeed = cParamBoss6->GetValueF32(1, 4);
	m_fRushAccel = cParamBoss6->GetValueF32(1, 5);
	m_nPiyoFrame = cParamBoss6->GetValueS32(1, 6);
	m_nRingHP = cParamBoss6->GetValueS32(1, 7);
	m_fSlopeMaxRad = Constants::TSP_DEG2RAD(cParamBoss6->GetValueF32(1, 8));
	m_fBouHitPower = cParamBoss6->GetValueF32(1, 10);
	m_fDodaiHitPower = cParamBoss6->GetValueF32(1, 11);
	m_fDodaiHitSize = cParamBoss6->GetValueF32(1, 12);
	nHp() = cParamBoss6->GetValueS32(1, 1);
	nHp() *= 100;
	nBombMax() = cParamBoss6->GetValueS32(1, 2);
	nBombTime() = cParamBoss6->GetValueS32(1, 3);
	nScore() = cParamBoss6->GetValueS32(1, 9);
	nMaxHp() = nHp();
	cParamDamage* cParamDamage2 = new cParamDamage("CSV_HIT_DAMAGE");
	for (int j = 0; j < 6; j++)
	{
		g_HitDamage[j] = (int)(cParamDamage2->GetValueF32(j + 1, 5) * 100.0f);
	}
	fChargeDamageRate() = cParamDamage2->GetValueF32(7, 5);
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay() && KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP > 0)
	{
		nHp() = KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHP;
	}
	else
	{
		KingdomGlobal::pMotosGameSys->KeepData()[0]->m_nBossHPLast = (short)nMaxHp();
	}
	if (m_nRingHP == 0)
	{
		m_nRingHP = 1;
	}
	ExHitAlloc(3);
	float num = cTaskPanelMan2->nWidth();
	float num2 = cTaskPanelMan2->nDepth();
	Vector4 v;
	Constants::MOTOS_VEC4_SET(v, 0.0f - num, 0.0f, -0.5f);
	ExHitRegistRect(0, v, num * 2.0f, 1.0f);
	Constants::MOTOS_VEC4_SET(v, -0.5f, 0.0f, 0.0f - num2);
	ExHitRegistRect(1, v, 1.0f, num2 * 2.0f);
	ExHitRegistCircle(2, Constants::VEC3_GET(0.0f, 0.0f, 0.0f), m_fDodaiHitSize, 3);
	if ((float)nHp() < cTaskBoss05::DAMAGE_BASE_HP(this))
	{
		fCoreFogNear() = 30.0f - cTaskBoss05::DAMAGE_ADD_NEAR_FOG(this) * (cTaskBoss05::DAMAGE_BASE_HP(this) - (float)nHp());
		fCoreFogFar() = 60.0f - cTaskBoss05::DAMAGE_ADD_FAR_FOG(this) * (cTaskBoss05::DAMAGE_BASE_HP(this) - (float)nHp());
	}
	CreateBomb();
	OnFlag(4194304u);
	SetMode(1);
}

void cTaskBoss05::Term()
{
	SparkFree();
	cTaskCharBoss::Term();
}

void cTaskBoss05::SparkFree()
{
	if (m_pBossPtk[0] != nullptr)
	{
		m_pBossPtk[0]->Detach(true);
		m_pBossPtk[0] = nullptr;
	}
	if (m_pBossPtk[1] != nullptr)
	{
		m_pBossPtk[1]->Detach(true);
		m_pBossPtk[1] = nullptr;
	}
	SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE057_SPARK);
}

void cTaskBoss05::SetRingDamageFrame()
{
	uint idx = 5u;
	if (GetMode() == 6 || m_RingInfo->m_vRingPos[1] >= 2.1f)
	{
		pModel(idx)->SetFrameMatClr(0u, 140.0f); // DOL 0x80049ba4 [audited: per AUDIT_G03_G07 row]: FLOAT_8047a404 = 140 frames (the port had the C#'s 2.333 s)
		return;
	}
	float num = m_RingInfo->m_vRingPos[1] / 2.1f * 139.0f;
	pModel(idx)->SetFrameMatClr(0u, num);   // native frames (the C# passed Unity seconds)
}

// DOL 0x80049e6c [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskBoss05::ActionBoth()
{
	SetRingDamageFrame();
	if (bTestFlag(4194304u) && bTestFlag(4194304u) && Step() != 8 && Step() != 9 && Step() == 255)
	{
		ReturnStep();
		(void)(8);
	}
	for (int i = 0; i < 3; i++)
	{
		if (m_BouMotInfo[i]->m_fRate == 0.0f)
		{
			continue;
		}
		if (m_BouMotInfo[i]->m_nDelay > 0)
		{
			m_BouMotInfo[i]->m_nDelay--;
		}
		else
		{
			m_BouMotInfo[i]->m_fMot += m_BouMotInfo[i]->m_fRate;
			if (m_BouMotInfo[i]->m_fMot <= 0.0f)
			{
				m_BouMotInfo[i]->m_fMot = 0.0f;
				m_BouMotInfo[i]->m_fRate = 0.0f;
			}
		}
		SetFrame((uint)m_BouMotInfo[i]->m_nModel, m_BouMotInfo[i]->m_fMot);
	}
	(void)(m_fCenterPosY);
	(void)(0.0f);
	GetMode();
	(void)(9);
	float frame = ((m_RingInfo->m_vRingPos[1] != 2.1f) ? (205.0f + 35.0f * (m_RingInfo->m_vRingPos[1] / 2.1f)) : 240.0f);
	SetFrame(8u, frame);
	(void)(m_IsDaiAnim);
	SlantFloor();
	if (GetMode() != 10 && (float)nHp() < cTaskBoss05::DAMAGE_BASE_HP(this))
	{
		fCoreFogNear() = 30.0f - cTaskBoss05::DAMAGE_ADD_NEAR_FOG(this) * (cTaskBoss05::DAMAGE_BASE_HP(this) - (float)nHp());
		fCoreFogFar() = 60.0f - cTaskBoss05::DAMAGE_ADD_FAR_FOG(this) * (cTaskBoss05::DAMAGE_BASE_HP(this) - (float)nHp());
	}
	return true;
}

// DOL 0x80049ba4 (compared 2026-10-30)
bool cTaskBoss05::ActionLocal()
{
	if (m_fCenterPosY < 0.0f && m_fCenterPosY + 2.95f <= m_RingInfo->m_vRingPos[1])
	{
		Constants::MO_RING_INFO* ringInfo = m_RingInfo;
		float fGoPosY = (m_RingInfo->m_vRingPos[1] = m_fCenterPosY + 2.95f);
		ringInfo->m_fGoPosY = fGoPosY;
		m_RingInfo->m_nHitCnt = 0;
		m_RingInfo->m_fUPSp = 0.0f;
	}
	if (m_RingInfo->m_vRingPos[1] < m_RingInfo->m_fGoPosY)
	{
		if ((m_RingInfo->m_vRingPos[1] += m_RingInfo->m_fUPSp) >= m_RingInfo->m_fGoPosY)
		{
			m_RingInfo->m_vRingPos[1] = m_RingInfo->m_fGoPosY;
			m_RingInfo->m_fUPSp = 0.0f;
		}
		else if (m_fCenterPosY < 0.0f && m_RingInfo->m_vRingPos[1] > 1.8999999f)
		{
			m_RingInfo->m_vRingPos[1] = (m_RingInfo->m_fGoPosY = 1.8999999f);
		}
		if ((m_RingInfo->m_fUPSp -= 0.05f) < 0.05f)
		{
			m_RingInfo->m_fUPSp = 0.05f;
		}
	}
	if (GetMode() != 1)
	{
		if (m_nHitFlash != 0 && GetMode() != 10)
		{
			m_nHitFlash--;
			int value = m_nHitFlash % 8 / 4;
			if (!CustomKingdomUtil::IF(m_nHitFlash))
			{
				pModel()->SetFogIdx(2);
				pModel(6u)->SetFogIdx(2);
				pModel(1u)->SetFogIdx(2);
				SetFlash(0, 0.0f);
				SetFlash(6, 0.0f);
				SetFlash(1, 0.0f);
			}
			else if (CustomKingdomUtil::IF(value))
			{
				pModel()->SetFogIdx(3);
				pModel(6u)->SetFogIdx(3);
				pModel(1u)->SetFogIdx(3);
				SetFlash(0, 1.0f);
				SetFlash(6, 1.0f);
				SetFlash(1, 1.0f);
			}
			else
			{
				pModel()->SetFogIdx(2);
				pModel(6u)->SetFogIdx(2);
				pModel(1u)->SetFogIdx(2);
				SetFlash(0, 0.0f);
				SetFlash(6, 0.0f);
				SetFlash(1, 0.0f);
			}
		}
		else
		{
			pModel()->SetFogIdx(2);
			pModel(6u)->SetFogIdx(2);
			pModel(1u)->SetFogIdx(2);
			SetFlash(0, 0.0f);
			SetFlash(6, 0.0f);
			SetFlash(1, 0.0f);
		}
	}
	if (m_fCenterPosY < -2.55f)
	{
		fHitSize() = fDefHitSize();
		ExHitParam(2)->m_nKind = 2;
		OffFlag(256u);
	}
	else
	{
		fHitSize() = fDefHitSize();
		ExHitParam(2)->m_nKind = 1;
		OnFlag(256u);
	}
	return ExecModeFunc() != 0;
}

bool cTaskBoss05::ActionOther()
{
	return true;
}

void cTaskBoss05::AdjustMaxSpeed()
{
	float num = ::Constants::jetGeoVecLength(vSpeed());
	if (num > fNowMaxSpeed())
	{
		num = fNowMaxSpeed() / num;
		Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
	}
}

// DOL 0x8004a1e0 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskBoss05::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	if (GetMode() != 0)
	{
		if (1 == GetMode())
		{
			if (2 == nMes)
			{
				::Constants::jetGeoVecUnit(vSpeed());
				Step() = 0;
				Step(1) = 0;
				Step(2) = 0;
				Step(3) = 0;
				nMovePoint() = 0;
				::Constants::jetGeoVecCopy(vMTrans(), s_vPos[nMovePoint()]);
				OnFlag(32u);
				for (uint num = 0u; num < 11; num++)
				{
					::Constants::jetGeoVecUnit(pModel(num)->vRotate());
				}
				return true;
			}
			if (3 == nMes)
			{
				SparkFree();
				InitFogData();
				pModel(2u)->SetUpdateRateAnmChr(0u, 1.0f);
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
				nMovePoint() = 0;
				fHitSize() = 0.0f;
				m_fNowAccel = fAccelSpeed();
				m_fNowMaxSpeed = fMaxSpeed();
				OffFlag(32u);
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()]))
				{
					::Constants::jetGeoVecUnit(vSpeed());
					nAction() = 0;
					cTaskBoss05::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, s_MdeTbl[nAction()]->m_nWait);
				}
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				nMovePoint() = SearchPlayer();
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
					int nWait = s_MdeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 12;
					cTaskBoss05::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait);
				}
				else if (IsAllRingUP())
				{
					cTaskBoss05::SET_MODE_PIYO(this, GetMode());
				}
				CenterMove();
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
				m_nFrameCnt = 0;
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
					int nWait2 = s_MdeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 12;
					if (s_MdeTbl[nAction()]->m_nMode == 5)
					{
						switch (nMovePoint())
						{
						case 5:
							nMovePoint() = 6;
							break;
						case 6:
							nMovePoint() = 7;
							break;
						case 7:
							nMovePoint() = 8;
							break;
						default:
							nMovePoint() = 5;
							break;
						}
					}
					cTaskBoss05::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait2);
				}
				else if (IsAllRingUP())
				{
					cTaskBoss05::SET_MODE_PIYO(this, GetMode());
				}
				CenterMove();
				return true;
			}
		}
		else if (5 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				::Constants::jetGeoVecUnit(vSpeed());
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
				m_fNowAccel = fRushAccel();
				m_fNowMaxSpeed = fRushSpeed();
				if (Constants::motosCharMove(this, s_vPos[nMovePoint()], 0.05f, m_fNowAccel))
				{
					int nWait3 = s_MdeTbl[nAction()]->m_nWait;
					nAction() = (nAction() + 1) % 12;
					int nMode = s_MdeTbl[nAction()]->m_nMode;
					if (nMode == 5)
					{
						switch (nMovePoint())
						{
						case 5:
							nMovePoint() = 6;
							break;
						case 6:
							nMovePoint() = 7;
							break;
						case 7:
							nMovePoint() = 8;
							break;
						default:
							nMovePoint() = 5;
							break;
						}
					}
					else
					{
						nMovePoint() = 0;
					}
					cTaskBoss05::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait3);
				}
				else if (IsAllRingUP())
				{
					cTaskBoss05::SET_MODE_PIYO(this, GetMode());
				}
				CenterMove();
				return true;
			}
		}
		else if (7 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				m_nFrameCnt = 0;
				nTimer() = 120;
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				switch (Step())
				{
				case 0:
					switch (nMovePoint())
					{
					case 0:
						SetBouAnime(4, 1.0f);
						break;
					case 1:
						SetBouAnime(2, 1.0f);
						SetBouAnime(0, 1.0f);
						break;
					case 2:
						SetBouAnime(3, 1.0f);
						SetBouAnime(0, 1.0f);
						break;
					case 3:
						SetBouAnime(3, 1.0f);
						SetBouAnime(1, 1.0f);
						break;
					case 4:
						SetBouAnime(2, 1.0f);
						SetBouAnime(1, 1.0f);
						break;
					}
					if (m_fCenterPosY > -2.95f)
					{
						SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE026_BOSS_FALL1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					}
					Step()++;
					break;
				case 1:
					m_fCenterSpeed = -0.084285714f;
					if (CenterMove())
					{
						Step()++;
					}
					break;
				case 2:
					switch (nMovePoint())
					{
					case 0:
						SpawnBomb(9, 3.3f, 0.3f, fDefHitSize());
						SpawnBomb(10, 3.3f, 0.3f, fDefHitSize());
						SpawnBomb(5, 3.3f, 0.3f, fDefHitSize());
						SpawnBomb(6, 3.3f, 0.3f, fDefHitSize());
						break;
					case 1:
						SpawnBomb(6, 3.3f, 0.3f, fDefHitSize());
						break;
					case 2:
						SpawnBomb(5, 3.3f, 0.3f, fDefHitSize());
						break;
					case 3:
						SpawnBomb(9, 3.3f, 0.3f, fDefHitSize());
						break;
					case 4:
						SpawnBomb(10, 3.3f, 0.3f, fDefHitSize());
						break;
					}
					Step()++;
					break;
				case 3:
					if (--nTimer() <= 0)
					{
						Step()++;
					}
					break;
				case 4:
					{
						int nWait4 = s_MdeTbl[nAction()]->m_nWait;
						nAction() = (nAction() + 1) % 12;
						if (s_MdeTbl[nAction()]->m_nMode == 9)
						{
							m_fCenterSpeed = 0.09833334f;
						}
						else
						{
							m_fCenterSpeed = 0.084285714f;
						}
						switch (nAction())
						{
						case 2:
							nMovePoint() = 0;
							break;
						case 5:
							nMovePoint() += 4;
							break;
						}
						cTaskBoss05::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait4);
						break;
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
				switch (Step())
				{
				case 0:
					SetBouAnime(4, 1.0f);
					if (m_fCenterPosY > -2.95f)
					{
						SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE026_BOSS_FALL1, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					}
					Step()++;
					break;
				case 1:
					m_fCenterSpeed = -0.084285714f;
					if (CenterMove())
					{
						Step(3) = SearchPlayer();
						Step()++;
					}
					break;
				case 2:
					switch (Step(3))
					{
					case 2:
						SpawnBomb(5, 3.3f, 0.3f, fDefHitSize());
						break;
					case 1:
						SpawnBomb(6, 3.3f, 0.3f, fDefHitSize());
						break;
					case 3:
						SpawnBomb(9, 3.3f, 0.3f, fDefHitSize());
						break;
					case 4:
						SpawnBomb(10, 3.3f, 0.3f, fDefHitSize());
						break;
					}
					if (++Step(2) >= 5)
					{
						Step(1) = 0;
						Step() = 4;
					}
					else
					{
						nTimer() = 30;
						Step(1) = 0;
						Step() = 3;
					}
					break;
				case 3:
					if (--nTimer() <= 0)
					{
						Step(1) = 0;
						Step() = 2;
					}
					break;
				case 4:
					m_fCenterSpeed = 0.084285714f;
					if (CenterMove())
					{
						int nWait5 = s_MdeTbl[nAction()]->m_nWait;
						nAction() = (nAction() + 1) % 12;
						cTaskBoss05::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait5);
					}
					break;
				}
				return true;
			}
		}
		else if (9 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				m_fTopAlpha = 0.0f;
				Vector4 v = Vector4();
				pModel(6u)->GetLocalNodeTrans(v, "effect_middle");
				::Constants::jetGeoVecAdd(v, v, pModel(6u)->vTrans());
				v[1] += fCenterPosY();
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE057_SPARK);
				m_pBossPtk[0] = Constants::SpawnEffect(38, v, nullptr, 0, 0.5f);
				pModel(6u)->GetLocalNodeTrans(v, "effect_middle");
				::Constants::jetGeoVecAdd(v, v, pModel(6u)->vTrans());
				v[1] += fCenterPosY();
				m_pBossPtk[1] = Constants::SpawnEffect(39, v, nullptr, 0, 0.5f);
				return true;
			}
			if (3 == nMes)
			{
				m_fTopAlpha = 0.0f;
				SparkFree();
				return true;
			}
			if (1 == nMes)
			{
				if (IsAllRingUP())
				{
					nAction() = (nAction() + 1) % 12;
					cTaskBoss05::SET_MODE_PIYO(this, s_MdeTbl[nAction()]->m_nMode);
					SparkFree();
				}
				else
				{
					switch (Step())
					{
					case 0:
					_switch23_0:
						SetFrame(9u, 0.0f);
						goto _switch23_1;
					case 1:
					_switch23_1:
						{
							uint num2 = 0u;
							if (++Step(1) < 30)
							{
								m_fTopAlpha += (float)num2 / 30.0f;
								if (m_fTopAlpha > (float)num2)
								{
									m_fTopAlpha = num2;
								}
							}
							else
							{
								m_fTopAlpha = num2;
								Step(1) = 0;
								Step()++;
							}
							SetBouAnime(6, 1.0f);
							SetBouAnime(7, 2.0f, 30);
							break;
						}
					case 2:
					_switch23_2:
						{
							Vector4 _out;
							Constants::VEC3_SET(_out, vMTrans()[0], 10.0f, vMTrans()[2]);
							KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->MeteoSpawn(KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans(), 0.35f, _out, 0.0f, 100);
							if (++Step(2) >= 5)
							{
								Step(1) = 0;
								Step() = 4;
							}
							else
							{
								nTimer() = 40 + kingdom::MslRandRange(0, 40);
								Step(1) = 0;
								Step() = 3;
							}
							SetBouAnime(6, 1.0f);
							SetBouAnime(7, 2.0f, 30);
							break;
						}
					case 3:
					_switch23_3:
						if (--nTimer() <= 0)
						{
							Step(1) = 0;
							Step() = 2;
						}
						SetBouAnime(6, 1.0f);
						SetBouAnime(7, 2.0f, 30);
						break;
					case 4:
					_switch23_4:
						if (++Step(1) < 60)
						{
							uint num2 = 0u;
							m_fTopAlpha -= (float)num2 / 40.0f;
							if (m_fTopAlpha < 0.0f)
							{
								m_fTopAlpha = 0.0f;
							}
						}
						else
						{
							int nWait6 = s_MdeTbl[nAction()]->m_nWait;
							nAction() = (nAction() + 1) % 12;
							cTaskBoss05::SET_MODE_WAIT(this, s_MdeTbl[nAction()]->m_nMode, nWait6);
						}
						break;
					}
					if (Step() < 4)
					{
						KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.065f, 10);
					}
					else
					{
						KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.02f, 10);
					}
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
				nTimer() = m_nPiyoFrame;
				m_nFrameCnt = 0;
				m_fLightMotAdd = 0.0f;
				m_fLightAmb = 0.0f;
				m_fLightAmbAdd = 0.0f - m_fLightAmb / 150.0f;
				return true;
			}
			if (3 == nMes)
			{
				SetFrame(10u, m_fLightMot);
				m_fLightMotAdd = 0.5f;
				m_fLightAmb = 0.0f;
				return true;
			}
			if (1 == nMes)
			{
				if (m_fLightMotAdd < 0.0f)
				{
					m_fLightMotAdd = 0.0f;
				}
				else if (m_fLightMotAdd > 0.5f)
				{
					m_fLightMotAdd = 0.5f;
				}
				m_fLightAmb += m_fLightAmbAdd;
				if (m_fLightAmb < 0.0f)
				{
					m_fLightAmb = 0.0f;
				}
				else if (m_fLightAmb > 0.0f)
				{
					m_fLightAmb = 255.0f;
				}
				switch (Step())
				{
				case 0:
					if (Step(1) % 2 == 0)
					{
						m_fCenterPosY -= 0.075f;
						if (m_fCenterPosY <= -0.15f)
						{
							Step(1)++;
						}
					}
					else
					{
						m_fCenterPosY += 0.075f;
						if (m_fCenterPosY >= 0.0f)
						{
							Step(1)++;
						}
					}
					if (Step(1) % 5 == 0)
					{
						SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE053_FAINT, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					}
					if (Step(1) == 21)
					{
						Step()++;
					}
					break;
				case 1:
					m_fCenterPosY -= m_fPiyoDWSp;
					m_fPiyoDWSp += m_fPiyoDWSp * 0.2f;
					if (m_fCenterPosY <= -2.95f)
					{
						m_fCenterPosY = -2.95f;
						KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.15f, 30);
						SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE047_BOSS_FALL, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
						Step()++;
					}
					break;
				case 2:
					if (--nTimer() <= 0)
					{
						Step()++;
					}
					break;
				case 3:
					m_fCenterSpeed = 0.073750004f;
					if (CenterMove())
					{
						SetMode(ReturnMode());
					}
					break;
				}
				return true;
			}
		}
		else if (10 == GetMode())
		{
			if (2 == nMes)
			{
				StepInit();
				nTimer() = 90; // DOL 0x8004a1e0 mode 10: 0x5a
				::Constants::jetGeoVecUnit(vSpeed());
				BombAllExplode();
				Step() = 0;
				KingdomGlobal::g_pMotosGame->nScore() += nScore();
				m_fCenterPosY = -2.95f;
				fFogNear() = 0.0f;
				fFogFar() = 0.0f;
				fCoreFogNear() = -30.0f;
				fCoreFogFar() = 30.0f;
				SetMortionReverse(3u, "ANM_CHAR_BOSS_5_A07", 0u, 2u);
				SetMortionReverse(4u, "ANM_CHAR_BOSS_5_A05", 0u, 2u);
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				ModeFoamChange();
				return true;
			}
		}
		else if (11 == GetMode())
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
					cTaskBoss05::SET_MODE_PIYO(this, ReturnMode());
				}
				CenterMove();
				return true;
			}
		}
	}
	return true;
}

bool cTaskBoss05::IsAllRingUP()
{
	if (!(m_RingInfo->m_vRingPos[1] >= 2.95f))
	{
		return false;
	}
	return true;
}

bool cTaskBoss05::CenterMove()
{
	m_fCenterPosY += m_fCenterSpeed;
	if (m_fCenterPosY > 0.0f)
	{
		m_fCenterSpeed = 0.0f;
		m_fCenterPosY = 0.0f;
		return true;
	}
	if (m_fCenterPosY <= 0.0f && m_fCenterPosY <= -2.95f)
	{
		m_fCenterSpeed = 0.0f;
		m_fCenterPosY = -2.95f;
		return true;
	}
	return false;
}

void cTaskBoss05::SetBouAnime(int kind, float rate, int delay, bool force)
{
	switch (kind)
	{
	case 2:
		if (!force && CustomKingdomUtil::IF(m_BouMotInfo[0]->m_fRate))
		{
			return;
		}
		m_BouMotInfo[0]->m_nModel = 0;
		m_BouMotInfo[0]->m_fMot = 0.0f;
		m_BouMotInfo[0]->m_fRate = rate;
		m_BouMotInfo[0]->m_nDelay = delay;
		break;
	case 3:
		if (!force && CustomKingdomUtil::IF(m_BouMotInfo[0]->m_fRate))
		{
			return;
		}
		m_BouMotInfo[0]->m_nModel = 0;
		m_BouMotInfo[0]->m_fMot = 0.0f;
		m_BouMotInfo[0]->m_fRate = 0.0f - rate;
		m_BouMotInfo[0]->m_nDelay = delay;
		break;
	case 0:
		if (!force && CustomKingdomUtil::IF(m_BouMotInfo[1]->m_fRate))
		{
			return;
		}
		m_BouMotInfo[1]->m_nModel = 2;
		m_BouMotInfo[1]->m_fMot = 0.0f;
		m_BouMotInfo[1]->m_fRate = rate;
		m_BouMotInfo[1]->m_nDelay = delay;
		break;
	case 1:
		if (!force && CustomKingdomUtil::IF(m_BouMotInfo[1]->m_fRate))
		{
			return;
		}
		m_BouMotInfo[1]->m_nModel = 2;
		m_BouMotInfo[1]->m_fMot = 0.0f;
		m_BouMotInfo[1]->m_fRate = 0.0f - rate;
		m_BouMotInfo[1]->m_nDelay = delay;
		break;
	case 4:
		if ((!force && CustomKingdomUtil::IF(m_BouMotInfo[0]->m_fRate)) || (!force && CustomKingdomUtil::IF(m_BouMotInfo[1]->m_fRate)))
		{
			return;
		}
		m_BouMotInfo[0]->m_nModel = 1;
		m_BouMotInfo[0]->m_fMot = 0.0f;
		m_BouMotInfo[0]->m_fRate = rate;
		m_BouMotInfo[0]->m_nDelay = delay;
		m_BouMotInfo[1]->m_nModel = 3;
		m_BouMotInfo[1]->m_fMot = 0.0f;
		m_BouMotInfo[1]->m_fRate = rate;
		m_BouMotInfo[1]->m_nDelay = delay;
		break;
	case 5:
		if (!force && CustomKingdomUtil::IF(m_BouMotInfo[2]->m_fRate))
		{
			return;
		}
		m_BouMotInfo[2]->m_nModel = 5;
		m_BouMotInfo[2]->m_fMot = 0.0f;
		m_BouMotInfo[2]->m_fRate = rate;
		m_BouMotInfo[2]->m_nDelay = delay;
		break;
	case 6:
		if ((!force && CustomKingdomUtil::IF(m_BouMotInfo[0]->m_fRate)) || (!force && CustomKingdomUtil::IF(m_BouMotInfo[1]->m_fRate)))
		{
			return;
		}
		m_BouMotInfo[0]->m_nModel = 1;
		m_BouMotInfo[0]->m_fMot = 0.0f;
		m_BouMotInfo[0]->m_fRate = 0.0f - rate;
		m_BouMotInfo[0]->m_nDelay = delay;
		m_BouMotInfo[1]->m_nModel = 3;
		m_BouMotInfo[1]->m_fMot = 0.0f;
		m_BouMotInfo[1]->m_fRate = 0.0f - rate;
		m_BouMotInfo[1]->m_nDelay = delay;
		break;
	case 7:
		if (!force && CustomKingdomUtil::IF(m_BouMotInfo[2]->m_fRate))
		{
			return;
		}
		m_BouMotInfo[2]->m_nModel = 5;
		m_BouMotInfo[2]->m_fMot = 0.0f;
		m_BouMotInfo[2]->m_fRate = 0.0f - rate;
		m_BouMotInfo[2]->m_nDelay = delay;
		break;
	default:
		{
			for (int i = 0; i < 3; i++)
			{
				m_BouMotInfo[i]->m_fMot = 0.0f;
				m_BouMotInfo[i]->m_fRate = 0.0f;
				m_BouMotInfo[i]->m_nDelay = 0;
			}
			break;
		}
	}
	for (int j = 0; j < 3; j++)
	{
		SetFrame((uint)m_BouMotInfo[j]->m_nModel, m_BouMotInfo[j]->m_fMot);
	}
}

ushort cTaskBoss05::SearchPlayer()
{
	Vector4 _out;
	Constants::VEC3_SUB(_out, KingdomGlobal::g_pMotosGame->pPlayers(0u)->vMTrans(), vMTrans());
	if (_out[0] < 0.0f)
	{
		if (!(_out[2] < 0.0f))
		{
			return 1;
		}
		return 2;
	}
	if (!(_out[2] < 0.0f))
	{
		return 4;
	}
	return 3;
}

float cTaskBoss05::GetHitPower(int nPart, int nOtherID)
{
	if (KingdomGlobal::g_pMotosGame->pChars((uint)nOtherID)->bTestFlag(16u))
	{
		return 10.0f;
	}
	switch (nPart)
	{
	case 1:
	case 2:
		return m_fBouHitPower;
	case 3:
		return m_fDodaiHitPower;
	default:
		return fHitPower();
	}
}

// DOL 0x8004c0fc [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskBoss05::RefAttacked(int nParts, int nGrobalID)
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
	case 0:
		if (!cTaskChar2->bTestFlag(384u) && !KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(16448u) && (!KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(131072u) || KingdomGlobal::g_pMotosGame->pPlayers(0u)->bTestFlag(384u)))
		{
			m_nHitFlash = 15;
			nHp() -= HitDamage(cTaskChar2, g_HitDamage);
			if (nHp() <= 0 && GetMode() != 10)
			{
				SetMode(10);
			}
			if (flag)
			{
				SpawnBossColEffect_Core(cTaskChar2);
			}
		}
		else if (flag)
		{
			SpawnBossColEffect(cTaskChar2);
		}
		break;
	case 3:
		{
			Constants::MO_RING_INFO* ringInfo = m_RingInfo;
			SetMortion(1u, "ANM_CHAR_BOSS_5_A04");
			if (ringInfo->m_vRingPos[1] < 2.95f)
			{
				float num = 2.95f / (float)m_nRingHP;
				if (cTaskChar2->bTestFlagOLD(4096u))
				{
					num *= 2.0f;
				}
				ringInfo->m_fGoPosY += num;
				if (ringInfo->m_fGoPosY > 2.95f)
				{
					ringInfo->m_fGoPosY = 2.95f;
				}
				ringInfo->m_fUPSp = num * 0.3f;
				SpawnBossColEffect(cTaskChar2, 32);
			}
			else if (flag)
			{
				SpawnBossColEffect(cTaskChar2, -1, 7);
			}
			if (!m_IsDaiAnim)
			{
				m_IsDaiAnim = true;
				SetFrame(7u, 0.0f);
			}
			break;
		}
	default:
		if (flag)
		{
			SpawnBossColEffect(cTaskChar2);
		}
		break;
	}
}

void cTaskBoss05::SlantFloor()
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

void cTaskBoss05::SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step)
{
	boss->Step() = step;
}

void cTaskBoss05::SET_STEP(cTaskCharBoss* boss, ushort step)
{
	cTaskBoss05::SET_STEP_NO_CLEAR(boss, step);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss05::SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time)
{
	boss->Step() = 255;
	boss->ReturnStep(step);
	boss->nTimer() = time;
}

void cTaskBoss05::SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time)
{
	cTaskBoss05::SET_STEP_WAIT_NO_CLEAR(boss, step, time);
	boss->Step(1) = (boss->Step(2) = (boss->Step(3) = 0));
}

void cTaskBoss05::ModeAppear()
{
	cTaskFade* cTaskFade2 = KingdomGlobal::g_pMotosGame->pTaskFade();
	if (KingdomGlobal::g_pMotosGame->nStep(0) != 2 || !bTestFlag(4194304u))
	{
		if (bTestFlag(4194304u))
		{
			for (int i = 0; i < 3; i++)
			{
				m_BouMotInfo[i]->m_fMot = 0.0f;
				m_BouMotInfo[i]->m_fRate = 0.0f;
				m_BouMotInfo[i]->m_nDelay = 0;
				SetFrame((uint)m_BouMotInfo[i]->m_nModel, 0.0f);
			}
		}
		fFogNear() = 0.0f;
		fFogFar() = 0.0f;
		m_fCenterPosY = 0.0f;
		SetApearScale(1.0f, 1.0f, 1.0f);
		OffFlag(4194304u);
		SetMortion(3u, "ANM_CHAR_BOSS_5_A08");
		SetMortion(4u, "ANM_CHAR_BOSS_5_A06");
		for (uint num = 0u; num < 11; num++)
		{
			pModel(num)->SetFogIdx(2);
			SetFlash((int)num, 0.0f);
		}
		pModel(5u)->SetUpdateRateMatClr(0u, 1.0f);
		KingdomGlobal::g_pMotosGame->pCamera()->LoadVector();
		::Constants::jetGeoVecCopy(vMTrans(), vMBeginTrans());
		vMTrans()[1] = 0.0f;
		cTaskBoss05::SET_MODE_WAIT(this, 2, 10);
		for (uint num2 = 0u; num2 < 11; num2++)
		{
			::Constants::jetGeoVecUnit(pModel(num2)->vRotate());
		}
		// TODO(unity): new GameObject().AddComponent<CoroutineObject>().ExecCoroutine(WaitOneFramePlayBGM());
		return;
	}
	switch (Step())
	{
	case 0:
		{
			Constants::VEC3_SET(m_vApearScale, 1.0f, 1.0f, 1.0f);
			SetApearScale(m_vApearScale[0], m_vApearScale[1], m_vApearScale[2]);
			fRotatePow = 0.0f;
			for (uint num13 = 0u; num13 < 11; num13++)
			{
				::Constants::jetGeoVecUnit(pModel(num13)->vRotate());
			}
			pModel(4u)->vScale()[0] = 0.4f;
			pModel(3u)->vScale()[2] = 0.4f;
			::Constants::jetGeoVecCopy(vMBeginTrans(), vMTrans());
			vMTrans()[1] = 60.0f;
			switch (Step(1))
			{
			case 0:
				if (KingdomGlobal::g_pMotosGame->nStep(0) == 2)
				{
					Step(1)++;
					// TODO(unity): new GameObject().AddComponent<CoroutineObject>().ExecCoroutine(WaitOneFramePlayBGM());
				}
				break;
			case 1:
				cTaskBoss05::SET_STEP(this, 7);
				break;
			}
			break;
		}
	case 6:
		{
			Constants::VEC3_SET(m_vApearScale, 1.0f, 1.0f, 1.0f);
			float num11 = 3.0f;
			Vector4 v4;
			::Constants::jetGeoVecCopy(v4, pModel()->vTrans());
			v4[1] += fCenterPosY();
			::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vTarget(), v4);
			float num12 = 10.0f;
			v4[0] += Mathf::Sin(fMovePoints) * num12;
			v4[1] = 30.0f;
			v4[2] += Mathf::Cos(fMovePoints) * num12;
			fMovePoints += 0.002f * num11;
			::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vPosition(), v4);
			vMTrans()[1] -= 0.05f * num11;
			if (vMTrans()[1] < 30.0f)
			{
				cTaskFade2->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE);
				cTaskBoss05::SET_STEP(this, 7);
			}
			break;
		}
	case 7:
		switch (Step(1))
		{
		case 0:
			Step(1)++;
			::Constants::jetGeoVecUnit(pModel(2u)->vScale());
			vMTrans()[1] = 20.0f;
			break;
		case 1:
			{
				Constants::VEC3_SET(m_vApearScale, 1.0f, 1.0f, 1.0f);
				float num9 = 1.0f;
				Vector4 v2;
				::Constants::jetGeoVecCopy(v2, pModel()->vTrans());
				v2[1] += fCenterPosY();
				::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vTarget(), v2);
				v2[0] += Mathf::Sin(fMovePoints2) * 10.0f;
				v2[1] = 30.0f;
				v2[2] += Mathf::Cos(fMovePoints2) * 10.0f;
				fMovePoints2 = 0.0f;
				::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vPosition(), v2);
				vMTrans()[1] -= 0.05f * num9;
				if (vMTrans()[1] < 6.0f)
				{
					vMTrans()[1] = 3.0f;
					Step(1) = 2;
				}
				break;
			}
		case 2:
			{
				Vector4 originPos = Vector4(vMTrans()[0], 0.5f, vMTrans()[2], 0.0f);
				::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vTarget(), originPos);
				originPos[2] += 10.0f;
				originPos[1] = 20.5f;
				::Constants::jetGeoVecCopy(KingdomGlobal::g_pMotosGame->pCamera()->vPosition(), originPos);
				vMTrans()[1] -= 0.05f;
				if (vMTrans()[1] < -2.3f)
				{
					::Constants::jetGeoVecCopy(m_vTempTrans, vMTrans());
					Step(1) = 3;
					Step(2) = 0;
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE020_BREAK, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					for (uint num4 = 0u; num4 < 11; num4++)
					{
						pModel(num4)->SetFogIdx(3);
					}
				}
				break;
			}
		case 3:
			{
				FogStep(2.0f); // DOL 0x8004cdcc [audited: described by the 43rd-session pass] (FLOAT_8047a3dc = 2)
				if (Step(2) < 30)
				{
					::Constants::jetGeoVecCopy(vMTrans(), m_vTempTrans);
					vMTrans()[1] += Constants::MOTOS_RANDRANGEF(-0.05f, 0.05f);
				}
				else
				{
					::Constants::jetGeoVecCopy(vMTrans(), m_vTempTrans);
				}
				if (Step(2)++ > 120)
				{
					for (uint num5 = 0u; num5 < 11; num5++)
					{
						pModel(num5)->SetFogIdx(3);
						SetFlash((int)num5, 1.0f);
					}
					InitFogData();
					Step(1) = 4;
					Step(2) = 0;
					Step(3) = 0;
				}
				break;
			}
		case 4:
			{
				pModel()->vRotate()[1] += fRotatePow;
				HeadRotate(pModel()->vRotate());
				fRotatePow += 0.0001f;
				FogStep(1.0f); // DOL 0x8004cfd4 [audited: described by the 43rd-session pass] (FLOAT_8047a3b8 = 1)
				if (!CustomKingdomUtil::IF(Step(2)))
				{
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE057_ENERGY, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					Step(2)++;
				}
				else if (Step(2) == 80)
				{
					Step(2) = 0;
					Step(3)++;
					if (Step(3) == 4)
					{
						Step(1)++;
						Step(2) = 0;
						Step(3) = 0;
						KingdomGlobal::g_pMotosGame->pCamera()->LoadVector();
						Vector4& reference = pModel(2u)->vScale();
						Vector4& reference2 = pModel(2u)->vScale();
						float num6 = (pModel(2u)->vScale()[2] = 1.0f);
						float value = (reference2[1] = num6);
						reference[0] = value;
					}
					InitFogData();
				}
				else
				{
					Step(2)++;
				}
				break;
			}
		case 5:
			{
				FogStep(2.0f); // DOL 0x8004d0dc [audited: call-site address checked in the disassembly]
				pModel()->vRotate()[1] += fRotatePow;
				HeadRotate(pModel()->vRotate());
				fRotatePow += 0.002f;
				if (pModel(4u)->vScale()[0] <= 1.0f)
				{
					pModel(4u)->vScale()[0] += 0.01f;
					pModel(3u)->vScale()[2] = pModel(4u)->vScale()[0];
				}
				vMTrans()[1] += 0.02f;
				if (vMTrans()[1] > 0.0f)
				{
					InitFogData();
					Step(1) = 6;
					Vector4 v3 = Vector4::zero;
					::Constants::jetGeoVecUnit(v3);
					HeadRotate(v3);
					for (uint num10 = 0u; num10 < 11; num10++)
					{
						pModel(num10)->SetFogIdx(3);
						SetFlash((int)num10, 1.0f);
					}
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE057_SPARK, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
					pModel(6u)->GetLocalNodeTrans(v3, "effect_middle");
					::Constants::jetGeoVecAdd(v3, v3, pModel(6u)->vTrans());
					v3[1] += fCenterPosY();
					m_pBossPtk[0] = Constants::SpawnEffect(38, v3, nullptr, 0, 0.5f);
					pModel(6u)->GetLocalNodeTrans(v3, "effect_middle");
					::Constants::jetGeoVecAdd(v3, v3, pModel(6u)->vTrans());
					v3[1] += fCenterPosY();
					m_pBossPtk[1] = Constants::SpawnEffect(39, v3, nullptr, 0, 0.5f);
					cTaskFade2->Fade(EFadeKind::eFADE_TYPE_IN_WHITE, 3000);
					SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE051_BLAST_BOM, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				}
				break;
			}
		case 6:
			{
				FogStep(2.0f); // DOL 0x8004d4cc [audited: call-site address checked in the disassembly]
				if (Step(2)++ > 180)
				{
					Vector4 v = Vector4::zero;
					::Constants::jetGeoVecUnit(v);
					HeadRotate(v);
					for (uint num3 = 0u; num3 < 11; num3++)
					{
						pModel(num3)->SetFogIdx(2);
						SetFlash((int)num3, 0.0f);
					}
					InitFogData();
					Step(1) = 0;
					Step(2) = 0;
					cTaskBoss05::SET_STEP(this, 9);
				}
				break;
			}
		}
		break;
	case 9:
		if (Step(1) != 0)
		{
			(void)(1);
			break;
		}
		OffFlag(4194304u);
		Step(1)++;
		bApearDemoEnd() = true;
		::Constants::jetGeoVecCopy(vMTrans(), vMBeginTrans());
		vMTrans()[1] = 0.0f;
		pModel(5u)->SetUpdateRateMatClr(0u, 1.0f);
		SetMortion(3u, "ANM_CHAR_BOSS_5_A08");
		SetMortion(4u, "ANM_CHAR_BOSS_5_A06");
		break;
	case 255:
		if (--nTimer() <= 0)
		{
			Step() = ReturnStep();
		}
		break;
	}
}

// DOL 0x8004d740 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
void cTaskBoss05::ModeFoamChange()
{
	cTaskFade* cTaskFade2 = KingdomGlobal::g_pMotosGame->pTaskFade();
	switch (Step())
	{
	case 0:
		vMTrans()[1] = (float)((double)kingdom::MslRandRange(0, 10) * 0.01);
		if (++Step(1) >= 60) // DOL 0x8004d740: 0x3b < count (the port had half the counts / times of this sequence)
		{
			fFogNear() = 0.0f;
			fFogFar() = 0.0f;
			cTaskBoss05::SET_STEP(this, 1);
		}
		break;
	case 1:
		{
			vMTrans()[1] = (float)((double)kingdom::MslRandRange(0, 10) * 0.01);
			if (Step(1) == 0)
			{
				KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.15f, 150);
			}
			bool flag = true;
			if (++Step(1) < 90)
			{
				fFogNear() -= 1.0f / 3.0f;
				fFogFar() -= -1.0f / 3.0f;
				flag = false;
			}
			if (++Step(2) < 90)
			{
				flag = false;
			}
			if (flag)
			{
				vMTrans()[1] = 0.0f;
				cTaskBoss05::SET_STEP_WAIT(this, 2, 60);
			}
			break;
		}
	case 2:
		{
			KingdomGlobal::g_pMotosGame->pCamera()->SetQuake(0.25f, 60);
			Vector4 _out;
			Constants::VEC3_SET(_out, 0.0f, 0.0f, vMTrans()[2]);
			Vector4 _out2;
			Constants::VEC3_SET(_out2, 0.0f, 0.0f, 0.25f);
			SpawnExplode(_out, Constants::VEC3_GET(0.0f, 0.0f, 0.0f), _out2, 16.0f, 1.0f, 1.0f);
			SpawnExplode(_out, Constants::VEC3_GET(0.0f, 0.0f, 0.0f), _out2, 16.0f, 1.0f, 1.0f, 20);
			Constants::VEC3_SET(_out, vMTrans()[0], 0.0f, 0.0f);
			Constants::VEC3_SET(_out2, 0.25f, 0.0f, 0.0f);
			SpawnExplode(_out, Constants::VEC3_GET(0.0f, 0.0f, 0.0f), _out2, 1.0f, 16.0f, 1.0f);
			SpawnExplode(_out, Constants::VEC3_GET(0.0f, 0.0f, 0.0f), _out2, 1.0f, 16.0f, 1.0f, 20);
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE047_BOSS_FALL, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
			cTaskFade2->Fade(EFadeKind::eFADE_TYPE_OUT_WHITE, 2000);
			cTaskBoss05::SET_STEP_WAIT(this, 3, 60);
			break;
		}
	case 3:
		if (cTaskFade2->IsEnd())
		{
			ExHitParam(0)->m_nKind = 2;
			ExHitParam(1)->m_nKind = 2;
			ExHitParam(2)->m_nKind = 2;
			cTaskCharEnemyBoss05Last* cTaskCharEnemyBoss05Last2 = new cTaskCharEnemyBoss05Last();
			cTaskCharEnemyBoss05Last2->Create(0u);
			::Constants::jetGeoVecCopy(cTaskCharEnemyBoss05Last2->vMTrans(), vMTrans());
			ConnectChild(cTaskCharEnemyBoss05Last2);
			KingdomGlobal::g_pMotosGame->pBoss() = cTaskCharEnemyBoss05Last2;
			cTaskCharEnemyBoss05Last2->InitParam(Constants::get_g_MoEnemyParam()[26]);
			cTaskCharEnemyBoss05Last2->OffFlag(16385u);
			KingdomGlobal::MotosSys->nLastBossType() = 1u;
			cTaskFade2->Fade(EFadeKind::eFADE_TYPE_IN_WHITE, 2000);
			ChangeTaiki();
		}
		else
		{
			ExplodeSE(3u, 10u);
		}
		break;
	case 255:
		if (--nTimer() <= 0)
		{
			Step() = ReturnStep();
		}
		else if (ReturnStep() == 3)
		{
			ExplodeSE(3u, 10u);
		}
		break;
	}
}

void cTaskBoss05::ModeDeath()
{
}

IEnumerator* cTaskBoss05::WaitOneFramePlayBGM()
{
	// TODO(unity): coroutine; original C# body:
	// 	yield return null;
	// 	yield return new WaitWhile(() => InGamePadManager.Disable);
	// 	if (!SceneChanger.IsExec)
	// 	{
	// 		SoundManager.BGM.Play(EBGMKind.MOTOS_SND_BGM06_BOSS2, 0.5f, bLoop: true, requestStopActive: false, checkSameAudioPlaying: true, 0.1f);
	// 	}
	return nullptr;
}

}
