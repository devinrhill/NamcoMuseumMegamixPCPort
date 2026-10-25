#include "kingdom/game/g07/cTaskConsSlopeEffect.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

cTaskConsSlopeEffect::cTaskConsSlopeEffect()
{
	Init();
}

cTaskConsSlopeEffect::~cTaskConsSlopeEffect()
{
	Free();
}

void cTaskConsSlopeEffect::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

cTaskAnita* cTaskConsSlopeEffect::Construct()
{
	return new cTaskConsSlopeEffect();
}

void cTaskConsSlopeEffect::Init()
{
	m_nTimer = 0u;
	m_nSoundTimer = 0u;
	m_nSlopePat = 2u;
	m_nSlopePatOld = m_nSlopePat;
	m_bGo = false;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(0u)->SetAnimNo(275);
	pAnita(0u)->SetState(3u, false);
}

void cTaskConsSlopeEffect::Term()
{
}

bool cTaskConsSlopeEffect::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) != 8)
	{
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE037_SLOPE_SE);
		Invisible(0u);
		return true;
	}
	float num = KingdomGlobal::g_pMotosGame->fConsSlopeRad();
	float num2 = KingdomGlobal::g_pMotosGame->fConsSlopeRadOLD();
	KingdomGlobal::g_pMotosGame->fSlopeRad();
	KingdomGlobal::g_pMotosGame->fSlopeRadOLD();
	if (num == num2 && KingdomGlobal::g_pMotosGame->nSlopeState() == 0)
	{
		m_nSlopePat = 2u;
	}
	else if (KingdomGlobal::g_pMotosGame->nSlopeState() == 1 || KingdomGlobal::g_pMotosGame->nSlopeState() == 3)
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, KingdomGlobal::g_pMotosGame->vSlopePower());
		if (v[0] < 0.0f)
		{
			m_nSlopePat = 0u;
		}
		else if (v[0] > 0.0f)
		{
			m_nSlopePat = 1u;
		}
		else
		{
			m_nSlopePat = 2u;
		}
	}
	else if (KingdomGlobal::g_pMotosGame->nSlopeState() == 2)
	{
		if (m_vSlopeVecOld[0] > KingdomGlobal::g_pMotosGame->vSlopePower()[0])
		{
			m_nSlopePat = 0u;
		}
		else if (m_vSlopeVecOld[0] < KingdomGlobal::g_pMotosGame->vSlopePower()[0])
		{
			m_nSlopePat = 1u;
		}
		else
		{
			m_nSlopePat = 2u;
		}
	}
	::Constants::jetGeoVecCopy(m_vSlopeVecOld, KingdomGlobal::g_pMotosGame->vSlopePower());
	if (m_nSlopePat == 0)
	{
		SetAnmDiffCurrent(0u, 275);
		pAnita(0u)->SetState(3u);
	}
	else if (m_nSlopePat == 1)
	{
		SetAnmDiffCurrent(0u, 276);
		pAnita(0u)->SetState(3u);
	}
	else
	{
		pAnita(0u)->SetState(3u, false);
	}
	if (CustomKingdomUtil::IF(m_nSoundTimer) && !CustomKingdomUtil::IF(--m_nSoundTimer))
	{
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE037_SLOPE_SE);
	}
	if (m_nSlopePat != m_nSlopePatOld && m_nSlopePat != 2)
	{
		m_nSlopePatOld = m_nSlopePat;
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE037_SLOPE_SE);
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE037_SLOPE_SE, true);
		m_nSoundTimer = 90u;
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
