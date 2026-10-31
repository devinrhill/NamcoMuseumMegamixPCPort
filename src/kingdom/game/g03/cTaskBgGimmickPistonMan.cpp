#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
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

// DOL 0x801523c4 [audited: per AUDIT_G03_G07 row]: the sound actor (+0x1c0, 0x800ba17c) and the synced gimmick index (+0x1d0) = -1
cTaskBgGimmickPistonMan::cTaskBgGimmickPistonMan()
{
	SetName("cTaskBgGimmickPistonMan");
	Init();
}

void cTaskBgGimmickPistonMan::Init()
{
	m_pSndActor = new kingdom::cKingdomSndActor();
	m_nSyncSndNo = -1;
}

// DOL dtor 0x80152510: deletes the actor and the gimmick list storage (the gimmicks themselves belong to their own owners)
void cTaskBgGimmickPistonMan::Term()
{
	if (m_pSndActor != nullptr)
	{
		delete m_pSndActor;
		m_pSndActor = nullptr;
	}
	for (cTaskBgGimmickPistonMan::AreaGimmick* item : m_vecGimmick)
	{
		delete item;
	}
	m_vecGimmick.clear();
}

// DOL 0x80152618 [audited: per AUDIT_G03_G07 row]: the first gimmick whose position is within its length of Pac-Man is the one the piston sound follows (+0x1d0 keeps the first
// one found); the cue "m03se_115_Piston" starts when it is not playing and that gimmick's chr clip is at frame 0; out of range everything stops (1000 ms)
bool cTaskBgGimmickPistonMan::Execute(cJetTask* pParam)
{
	bool flag = false;
	Vector4 v = Vector4::zero;
	for (uint i = 0u; i < m_vecGimmick.size(); i++)
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
				m_nSyncSndNo = (int)i;
			}
			break;
		}
	}
	if (flag)
	{
		float frameAnmChr = m_vecGimmick[m_nSyncSndNo]->pTaskBgGimmick->Model->GetFrameAnmChr(0u);   // DOL 0x8001c2e0 [audited: class row done in the 43rd session; function not individually re-read]
		if (!SoundManager::get_SE()->IsPlaying(m_SndHnd) && frameAnmChr == 0.0f)
		{
			if (cPacnRollLoopSndMan::CanPlay()) m_SndHnd = cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlayActor(ESEKind::PACN_ROLL_SND_SE115_PISTON, m_pSndActor, Vector3(v.x, v.y, v.z)));
		}
	}
	else
	{
		SoundManager::get_SE()->Stop(m_SndHnd, 1.0f);
		m_nSyncSndNo = -1;
	}
	m_pSndActor->SetPosition(v);
	return cJetTask::Execute(pParam);
}

// DOL 0x801527c4 [audited: per AUDIT_G03_G07 row] (blr)
void cTaskBgGimmickPistonMan::Draw(int nState, cJetTask* pParam)
{
	cJetTask::Draw(nState, pParam);
}

// DOL 0x801527c8 [audited: per AUDIT_G03_G07 row]: length = 2 x the half diagonal of the gimmick's scene model volume (c3e0 = 2.0)
void cTaskBgGimmickPistonMan::RegistGimmick(cTaskBgGimmick* gim)
{
	cTaskBgGimmickPistonMan::AreaGimmick* areaGimmick = new cTaskBgGimmickPistonMan::AreaGimmick();
	areaGimmick->length = gim->ModelHalfDiagonal() * 2.0f;
	areaGimmick->pTaskBgGimmick = gim;
	m_vecGimmick.push_back(areaGimmick);
}

}
