#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
#include "kingdom/game/g03/cTaskBgGimmickAddinSnd.hpp"

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "ResNode.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include <Constants.hpp>
#include <ReleaseObjectManager.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

// DOL 0x8014dfbc [audited: per AUDIT_G03_G07 row] (sound kind +0x534; 0x8014e078 is the dtor)
cTaskBgGimmickAddinSnd::cTaskBgGimmickAddinSnd(ResNode* const& node0, ResNode* const& node1, const ESEKind& sndName)
	: cTaskBgGimmick(node0, node1)
{
	m_pSndName = sndName;
	SetName("cTaskBgGimmickAddinSnd");
	Init();
}

// DOL 0x8014e128 [audited: per AUDIT_G03_G07 row] (+0x530 = half diagonal of the scene model's bounding volume)
void cTaskBgGimmickAddinSnd::Init()
{
	m_pSndActor = new kingdom::cKingdomSndActor();   // +0x520 (0x800ba17c)
	m_fLength = ModelHalfDiagonal();   // the scene model's bounding volume, not the hit model's (ScnObj::GetBoundingVolume)
}

// DOL 0x8014e1d8 [audited: per AUDIT_G03_G07 row] (called from the dtor 0x8014e078): deletes the actor
void cTaskBgGimmickAddinSnd::Term()
{
	if (m_pSndActor != nullptr)
	{
		delete m_pSndActor;
		m_pSndActor = nullptr;
	}
	cTaskBgGimmick::Term();
}

// DOL 0x8014e230 [audited: per AUDIT_G03_G07 row]
bool cTaskBgGimmickAddinSnd::Execute(cJetTask* pParam)
{
	// the gimmick's looping SE plays while Pac-Man is within m_fLength of it and fades out when he leaves; the 3D actor follows the gimmick
	Vector4 vPos;
	::Constants::jetGeoVecCopy(vPos, this->Model->vTrans());
	if (::Constants::jetGeoVecDistance(Constants::PACN_APP()->pTaskCharPacman()->vTrans(), vPos) < m_fLength)
	{
		if (!SoundManager::get_SE()->IsPlaying(m_SndHnd))   // DOL: SoundSeBank::IsPlaying (0x80026424), so the cue restarts when it has ended
		{
			if (cPacnRollLoopSndMan::CanPlay()) m_SndHnd = cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlayActor(m_pSndName, m_pSndActor, Vector3(vPos.x, vPos.y, vPos.z)));
		}
	}
	else if (SoundManager::get_SE()->IsPlaying(m_SndHnd))
	{
		SoundManager::get_SE()->Stop(m_SndHnd, 0.5f);   // the DOL passes 500 (ms): the port had 500 / 60 s
	}
	m_pSndActor->SetPosition(vPos);   // DOL 0x80026620
	return cTaskBgGimmick::Execute(pParam);
}

void cTaskBgGimmickAddinSnd::Draw(int nState, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nState, pParam);
}

}
