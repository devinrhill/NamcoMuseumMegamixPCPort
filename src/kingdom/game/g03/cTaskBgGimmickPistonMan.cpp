#include "kingdom/game/g03/cTaskBgGimmickPistonMan.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include <cmath>

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

cTaskBgGimmickPistonMan::cTaskBgGimmickPistonMan()
{
	SetName("cTaskBgGimmickPistonMan");
	Init();
}

void cTaskBgGimmickPistonMan::Init()
{
	m_nSyncSndNo = -1;
}

void cTaskBgGimmickPistonMan::Term()
{
	for (cTaskBgGimmickPistonMan::AreaGimmick* item : m_vecGimmick)
	{
		if (item->pTaskBgGimmick != nullptr)
		{
			item->pTaskBgGimmick->Term();
		}
	}
}

bool cTaskBgGimmickPistonMan::Execute(cJetTask* pParam)
{
	if (Constants::PACN_APP()->pTaskCharPacman() == nullptr)
	{
		return true;
	}
	bool flag = false;
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	for (int i = 0; i < (int)m_vecGimmick.size(); i++)
	{
		cTaskBgGimmick* pTaskBgGimmick = m_vecGimmick[i]->pTaskBgGimmick;
		Vector4 v2;
		::Constants::jetGeoVecCopy(v2, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
		if (::Constants::jetGeoVecDistance(pTaskBgGimmick->Model->vTrans(), v2) <= m_vecGimmick[i]->length)
		{
			::Constants::jetGeoVecCopy(v, pTaskBgGimmick->Model->vTrans());
			flag = true;
			if (m_nSyncSndNo == -1)
			{
				m_nSyncSndNo = i;
			}
			break;
		}
	}
	if (flag)
	{
		cTaskBgGimmick* pTaskBgGimmick2 = m_vecGimmick[m_nSyncSndNo]->pTaskBgGimmick;
		float frameAnmChr = pTaskBgGimmick2->Model->GetFrameAnmChr(0u);
		float frameMaxAnmChr = pTaskBgGimmick2->Model->GetFrameMaxAnmChr(0u);
		if (std::fmod(frameAnmChr, frameMaxAnmChr) < Time::deltaTime * 2.0f && !m_isFirstSnd)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE115_PISTON);
			m_isFirstSnd = true;
		}
		else if (std::fmod(frameAnmChr, frameMaxAnmChr) > frameMaxAnmChr / 2.0f && m_isFirstSnd)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE115_PISTON);
			m_isFirstSnd = false;
		}
	}
	else
	{
		SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE115_PISTON, 16.666666f);
		m_nSyncSndNo = -1;
		m_isFirstSnd = false;
	}
	return cJetTask::Execute(pParam);
}

void cTaskBgGimmickPistonMan::Draw(int nState, cJetTask* pParam)
{
	cJetTask::Draw(nState, pParam);
}

void cTaskBgGimmickPistonMan::RegistGimmick(cTaskBgGimmick* gim)
{
	cTaskBgGimmickPistonMan::AreaGimmick* areaGimmick = new cTaskBgGimmickPistonMan::AreaGimmick();
	Vector4 v = Vector4();
	AABB& reference = gim->pFaceList()->ReadOnlyAABB();
	v[0] = (reference.This.max().x - reference.This.min().x) * 0.5f;
	v[1] = (reference.This.max().y - reference.This.min().y) * 0.5f;
	v[2] = (reference.This.max().z - reference.This.min().z) * 0.5f;
	v[3] = 1.0f;
	areaGimmick->length = ::Constants::jetGeoVecLength(v);
	areaGimmick->length *= 2.0f;
	areaGimmick->pTaskBgGimmick = gim;
	m_vecGimmick.push_back(areaGimmick);
}

}
