#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskEffect.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "kingdom/game/g07/MO_EFFECT_INFO.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

const string cTaskEffect::ptk_tbl[46] = {"", "PTK_01", "PTK_02", "PTK_03", "PTK_05", "PTK_06", "PTK_07", "PTK_08", "PTK_09", "PTK_10", "PTK_12", "PTK_13", "PTK_14", "PTK_15", "PTK_18", "PTK_19", "PTK_20", "PTK_21", "PTK_22", "PTK_23", "PTK_24", "PTK_26", "PTK_28", "PTK_29", "PTK_30", "PTK_31", "PTK_32", "PTK_33", "PTK_34", "PTK_36", "PTK_37", "PTK_38", "PTK_39", "PTK_10", "PTK_41", "PTK_42", "PTK_44", "PTK_45", "PTK_46", "PTK_47", "PTK_48", "PTK_49", "PTK_50", "PTK_51", "PTK_52", "PTK_53"};

cTaskEffect::cTaskEffect()
{
	Init();
}

cTaskEffect::~cTaskEffect()
{
	Term();
}

float cTaskEffect::GetFrameRate()
{
	return m_fFrameRate;
}

void cTaskEffect::SetFrameRate(float rate)
{
	m_fFrameRate = rate;
}

bool cTaskEffect::IsPause()
{
	return m_bPause;
}

void cTaskEffect::Pause()
{
	m_bPause = true;
}

void cTaskEffect::Resume()
{
	m_bPause = false;
}

void cTaskEffect::Init()
{
	m_nInfoIdx = 0u;
	for (int i = 0; i < (int)std::size(m_aEffInfo); i++)
	{
		m_aEffInfo[i] = new MO_EFFECT_INFO();
	}
	for (int j = 0; j < (int)std::size(m_ahPtk); j++)
	{
		m_ahPtk[j] = new cKingdomPtkHnd();
	}
}

void cTaskEffect::Term()
{
}

// DOL 0x801f6f34 [audited: described by the 43rd-session pass] (46 ptk CRCs at 0x8044f330 compared)
void cTaskEffect::SetData()
{
	if (KingdomGlobal::MotosSys->IsBossStage())
	{
		this->get_KingdomPtkMan()->Create(46u, 512u, 512u, 4096u);
	}
	else
	{
		this->get_KingdomPtkMan()->Create(46u, 256u, 512u, 2048u);
	}
	m_bPause = false;
	for (uint num = 0u; num < 46; num++)
	{
		if (!ptk_tbl[num].empty())
		{
			this->get_KingdomPtkMan()->RegistData(num, ptk_tbl[num]);
		}
	}
}

void cTaskEffect::AllDetach()
{
}

// DOL 0x801f7010 [audited: described by the 43rd-session pass] (compared 2026-10-30)
bool cTaskEffect::Execute(cJetTask* pParam)
{
	if (IsPause())
	{
		return true;
	}
	m_nowFramePlaySEExpl01 = false;
	m_nowFramePlaySEExplPanel = false;
	m_nowFramePlaySEExpl03 = false;
	m_nowFramePlaySEHitZako03 = false;
	auto& aEffInfo = m_aEffInfo;
	for (int i = 0; (long)i < 128L; i++)
	{
		if (!CustomKingdomUtil::IF(aEffInfo[i]->m_nFlag & 1) || (IsPause() && CustomKingdomUtil::IF(aEffInfo[i]->m_nFlag & 0x400)))
		{
			continue;
		}
		if (CustomKingdomUtil::IF(aEffInfo[i]->m_nDelay))
		{
			aEffInfo[i]->m_nDelay--;
		}
		else if (CustomKingdomUtil::IF(aEffInfo[i]->m_nFlag & 0x10))
		{
			if (aEffInfo[i]->m_Func != nullptr)
			{
				aEffInfo[i]->m_Func(aEffInfo[i]);
			}
		}
		else
		{
			aEffInfo[i]->m_nFlag = 0;
		}
	}
	if (!Constants::SYSTEM_IS_PAUSE()) // DOL 0x801f7010: game pause byte 0 and pad 0 not holding HOME; otherwise no call (the port also advanced while HOME was held)
	{
		this->get_KingdomPtkMan()->UpdateFrame(1.0f);
	}
	return true;
}

// DOL 0x801f7114 [audited: described by the 43rd-session pass] (audited 44th: slope-matrix transform, slot search, flags 0x409, custom funcs for kinds 14 / 33, SE kinds 19 / 20 / 21 / 32 match)
cKingdomPtkHnd* cTaskEffect::SpawnEffect(int nKind, Vector4 pMPos, cJetTask* pParam, int delay, bool bDead, float loadFactor)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pMPos);
	::Constants::jetGeoVecTrans(v, KingdomGlobal::g_pMotosGame->pSlopeMatrix(), v);
	Vector4 _discard1;
	::Constants::jetGeoVecCopy(_discard1, v);
	MO_EFFECT_INFO* mO_EFFECT_INFO = nullptr;
	uint num = 0u;
	for (num = 0u; num < 128; num++)
	{
		m_nInfoIdx = (m_nInfoIdx + 1) % 128;
		mO_EFFECT_INFO = m_aEffInfo[m_nInfoIdx];
		if ((!CustomKingdomUtil::IF(mO_EFFECT_INFO->m_nFlag & 1) && mO_EFFECT_INFO->m_bDie) || (mO_EFFECT_INFO->m_phPtk != nullptr && !mO_EFFECT_INFO->m_phPtk->IsValid()))
		{
			break;
		}
	}
	if (num >= 128)
	{
		// TODO(unity): Debug.LogFormat("Effect All Used....\n");
		return nullptr;
	}
	CustomKingdomUtil::Assert(mO_EFFECT_INFO != nullptr, "SpawnEffect");
	mO_EFFECT_INFO->m_nID = (ushort)m_nInfoIdx;
	mO_EFFECT_INFO->m_nFlag |= 1033;
	mO_EFFECT_INFO->m_nKind = (ushort)nKind;
	mO_EFFECT_INFO->m_nDelay = (ushort)delay;
	mO_EFFECT_INFO->m_phPtk = m_ahPtk[m_nInfoIdx];
	int drawLayer = 9;
	switch (nKind)
	{
	case 14:
		CustomKingdomUtil::Assert(mO_EFFECT_INFO != nullptr, "error");
		if (static_cast<cTaskChar*>(pParam) != nullptr)
		{
			mO_EFFECT_INFO->m_Func = &cTaskEffect::CustumFuncCharge;
			mO_EFFECT_INFO->m_Param[0] = pParam;
			mO_EFFECT_INFO->m_nFlag |= 16;
		}
		drawLayer = 0;
		break;
	case 19:
		if (!m_nowFramePlaySEExpl01)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE006_SMOLE_BOM);
			m_nowFramePlaySEExpl01 = true;
		}
		break;
	case 20:
		drawLayer = 0;
		if (!m_nowFramePlaySEExplPanel)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE051_BLAST_BOM);
			m_nowFramePlaySEExplPanel = true;
		}
		break;
	case 21:
		if (!m_nowFramePlaySEExpl03)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE007_BOSS_DEATH);
			m_nowFramePlaySEExpl03 = true;
		}
		break;
	case 24:
		drawLayer = 0;
		break;
	case 27:
	case 28:
		drawLayer = 0;
		break;
	case 32:
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE046_BOSS_HIT);
		break;
	case 33:
		CustomKingdomUtil::Assert(mO_EFFECT_INFO != nullptr, "error");
		if (static_cast<cTaskChar*>(pParam) != nullptr)
		{
			mO_EFFECT_INFO->m_Func = &cTaskEffect::CustumFuncPiyori;
			mO_EFFECT_INFO->m_Param[0] = pParam;
			mO_EFFECT_INFO->m_nFlag |= 16;
		}
		break;
	case 37:
		drawLayer = 0;
		break;
	}
	mO_EFFECT_INFO->m_phPtk->Detach(true);
	if (bDead)
	{
		mO_EFFECT_INFO->m_phPtk->Generate((uint)nKind, true, true, true, delay, v, nullptr, nullptr, 0, -1, drawLayer, loadFactor);
		mO_EFFECT_INFO->m_bDie = true;
	}
	else
	{
		mO_EFFECT_INFO->m_phPtk->Generate((uint)nKind, false, true, true, delay, v, nullptr, nullptr, 0, -1, drawLayer, loadFactor);
		mO_EFFECT_INFO->m_bDie = false;
	}
	Vector4 v3 = Vector4::zero;
	::Constants::jetGeoVecUnit(v3);
	mO_EFFECT_INFO->m_phPtk->vSetRotate(v3);
	return mO_EFFECT_INFO->m_phPtk;
}

ESEKind cTaskEffect::nGetColSE(cTaskChar* pCharA, cTaskChar* pCharB)
{
	if (pCharA == nullptr || pCharA->bTestFlag(24u))
	{
		return ESEKind::NONE;
	}
	if (pCharA->bTestFlag(2u))
	{
		if (pCharA->bTestFlag(4096u) && !pCharA->bTestFlag(1048576u))
		{
			return ESEKind::MOTOS_SND_SE004_HIT_BOSS01;
		}
		if (pCharB != nullptr && pCharB->bTestFlag(2u))
		{
			return ESEKind::MOTOS_SND_SE004_HIT_BOSS01;
		}
	}
	switch (pCharA->nKind())
	{
	case 1u:
	case 2u:
	case 3u:
	case 25u:
		if (!m_nowFramePlaySEHitZako03)
		{
			m_nowFramePlaySEHitZako03 = true;
			return ESEKind::MOTOS_SND_SE003_HIT_ZAKO03;
		}
		return ESEKind::NONE;
	case 4u:
	case 10u:
		return ESEKind::MOTOS_SND_SE002_HIT_ZAKO02;
	case 11u:
	case 12u:
	case 13u:
	case 24u:
		return ESEKind::MOTOS_SND_SE001_HIT_ZAKO01;
	case 15u:
		return ESEKind::MOTOS_SND_SE005_HIT_WALL;
	case 5u:
	case 6u:
	case 7u:
	case 8u:
	case 9u:
		return ESEKind::MOTOS_SND_SE054_HIT_ZAKO04;
	case 14u:
		return ESEKind::MOTOS_SND_SE056_HIT_BEACON;
	case 22u:
	case 23u:
		return ESEKind::MOTOS_SND_SE057_HIT_BALL;
	default:
		return ESEKind::NONE;
	}
}

// DOL 0x801f7574 [audited: described by the 43rd-session pass] (audited 44th: matches; the per-frame sound debounce flags m_nowFramePlaySE* are C# stand-ins for the RSAR player limits, E1)
void cTaskEffect::SpawnColEffect(cTaskChar* pChar0, cTaskChar* pChar1)
{
	if (pChar0 == nullptr || pChar1 == nullptr || pChar0->bTestFlag(16u) || pChar1->bTestFlag(16u))   // DOL 0x801f7574: no kind 25 / 25 exclusion (C#)
	{
		return;
	}
	Vector4 v;
	::Constants::jetGeoVecCopy(v, ColPos(pChar0, pChar1));
	int nKind = 1;
	if (pChar0->bTestFlag(8u) || pChar1->bTestFlag(8u))
	{
		nKind = 3;
	}
	else if (pChar0->bTestFlag(4096u) || pChar1->bTestFlag(4096u))
	{
		nKind = (((!pChar0->bTestFlag(4096u) || pChar0->bTestFlag(1048576u)) && (!pChar1->bTestFlag(4096u) || pChar1->bTestFlag(1048576u))) ? 2 : 4);
	}
	SpawnEffect(nKind, v, nullptr);
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		int nKind2 = 5 + kingdom::MslRandRange(0, 4);
		if (!pChar0->bTestFlag(2u))
		{
			SpawnEffect(nKind2, v, nullptr);
			SpawnEffect(nKind2, v, nullptr);
			SpawnEffect(nKind2, v, nullptr);
		}
		if (!pChar1->bTestFlag(2u))
		{
			SpawnEffect(nKind2, v, nullptr);
			SpawnEffect(nKind2, v, nullptr);
			SpawnEffect(nKind2, v, nullptr);
		}
	}
	else
	{
		int nKind3 = 5 + kingdom::MslRandRange(0, 4);
		if (pChar0->bTestFlag(2u))
		{
			SpawnEffect(pChar0->nChipColor(), v, nullptr);
		}
		else
		{
			SpawnEffect(nKind3, v, nullptr);
			SpawnEffect(nKind3, v, nullptr);
		}
		if (pChar1->bTestFlag(2u))
		{
			SpawnEffect(pChar1->nChipColor(), v, nullptr);
		}
		else
		{
			SpawnEffect(nKind3, v, nullptr);
			SpawnEffect(nKind3, v, nullptr);
		}
	}
	ESEKind eSEKind = nGetColSE(pChar0, pChar1);
	ESEKind eSEKind2 = nGetColSE(pChar1, pChar0);
	if (eSEKind != ESEKind::NONE)
	{
		SoundManager::get_SE()->PlaySE(eSEKind);
	}
	if (eSEKind2 != ESEKind::NONE && eSEKind2 != eSEKind)
	{
		SoundManager::get_SE()->PlaySE(eSEKind2);
	}
}

Vector4 cTaskEffect::ColPos(cTaskChar* pChar0, cTaskChar* pChar1, bool Ad)
{
	::Constants::jetGeoVecUnit(m_vColPos);
	Constants::VEC3_SUB(m_vColPos, pChar1->vMTrans(), pChar0->vMTrans());
	if (CustomKingdomUtil::IF(Constants::tspVector3Length(m_vColPos)))
	{
		::Constants::jetGeoVecNormalize(m_vColPos, m_vColPos);
		if (!Ad)
		{
			Constants::VEC3_MUL(m_vColPos, m_vColPos, pChar0->fHitSize());
			Constants::VEC3_ADD(m_vColPos, pChar0->vMTrans(), m_vColPos);
		}
		else
		{
			Constants::tspVector3Scale(m_vColPos, m_vColPos, -1.0f);
			Constants::VEC3_MUL(m_vColPos, m_vColPos, pChar1->fHitSize());
			Constants::VEC3_ADD(m_vColPos, pChar1->vMTrans(), m_vColPos);
		}
	}
	else
	{
		::Constants::jetGeoVecCopy(m_vColPos, pChar0->vMTrans());
	}
	return m_vColPos;
}

void cTaskEffect::CustumFuncCharge(MO_EFFECT_INFO* pInfo)
{
	cKingdomPtkHnd* phPtk = pInfo->m_phPtk;
	cTaskCharPlayer* cTaskCharPlayer2 = static_cast<cTaskCharPlayer*>(pInfo->m_Param[0]);
	if (phPtk != nullptr && cTaskCharPlayer2 != nullptr)
	{
		Vector4 v = Vector4::zero;
		::Constants::jetGeoVecUnit(v);
		Constants::VEC3_MUL(v, v, 1.0f);
		Constants::VEC3_ADD(v, cTaskCharPlayer2->vMTrans(), v);
		v[1] += 0.5f;
		::Constants::jetGeoVecTrans(v, KingdomGlobal::g_pMotosGame->pSlopeMatrix(), v);
		phPtk->vSetTrans(v);
		cTaskCharPlayer2->ChargeCnt2Level(cTaskCharPlayer2->nChargeCnt());
		if (!cTaskCharPlayer2->bTestFlag(8192u))
		{
			pInfo->m_nFlag = 0;
			phPtk->Detach(true);
		}
	}
	if (CustomKingdomUtil::IF(pInfo->m_nFlag & 4) && CustomKingdomUtil::IF(pInfo->m_nTimer) && !CustomKingdomUtil::IF(--pInfo->m_nTimer))
	{
		pInfo->m_nFlag = 0;
		phPtk->Detach(true);
	}
}

void cTaskEffect::CustumFuncPiyori(MO_EFFECT_INFO* pInfo)
{
	cKingdomPtkHnd* phPtk = pInfo->m_phPtk;
	cTaskChar* cTaskChar2 = static_cast<cTaskChar*>(pInfo->m_Param[0]);
	Vector4 v = Vector4();
	if (cTaskChar2 != nullptr)
	{
		::Constants::jetGeoVecCopy(v, cTaskChar2->vMTrans());
	}
	if (phPtk != nullptr && cTaskChar2 != nullptr)
	{
		if (KingdomGlobal::g_pMotosGame->pMap() != nullptr)
		{
			KingdomGlobal::g_pMotosGame->pMap()->Map2World(v, v);
		}
		v[1] += 1.2f;
		::Constants::jetGeoVecTrans(v, KingdomGlobal::g_pMotosGame->pSlopeMatrix(), v);
		phPtk->vSetTrans(v);
		cTaskChar2->nPiyoTimer();
		(void)(30);
		if (!cTaskChar2->bTestFlag(2097152u))
		{
			pInfo->m_nFlag = 0;
			phPtk->Detach(true);
		}
	}
	else
	{
		pInfo->m_nFlag = 0;
		phPtk->Detach(true);
	}
}

}
