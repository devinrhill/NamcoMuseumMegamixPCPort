#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
#include "kingdom/game/g03/cTaskBgGimmickTruck.hpp"

#include "cJetTask.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g03
{

// DOL 0x80153cb8 [audited: per AUDIT_G03_G07 row]
cTaskBgGimmickTruck::cTaskBgGimmickTruck(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickTruck");
	Init();
}

// DOL 0x80153e14 [audited: per AUDIT_G03_G07 row] (vtable +8): the sound actor (+0x520), step (+0x524) 0, state bit 0x40
void cTaskBgGimmickTruck::Init()
{
	m_pSndActor = new kingdom::cKingdomSndActor();
	m_nStep = 0u;
	OnState(64u);
}

// DOL 0x80153e58 [audited: per AUDIT_G03_G07 row]: deletes the actor (its cues stay where they were last placed)
void cTaskBgGimmickTruck::Term()
{
	if (m_pSndActor != nullptr)
	{
		delete m_pSndActor;
		m_pSndActor = nullptr;
	}
	cTaskBgGimmick::Term();
}

// DOL 0x80153eb0 [audited: per AUDIT_G03_G07 row]: first frame plays the cue "m03se_040_Train" (0x8037a598, fade 300) on the actor, then the actor follows the gimmick matrix (+0x2b0)
bool cTaskBgGimmickTruck::Execute(cJetTask* pParam)
{
	if (m_nStep == 0)
	{
		if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(m_pSndActor->Play("m03se_040_Train"));
		m_nStep = 1u;
	}
	Vector4 v = Vector4::zero;
	::Constants::jetGeoMtxGetTrans(v, pMtx());   // DOL 0x8013ecf8 [audited: class row done in the 43rd session; function not individually re-read]
	m_pSndActor->SetPosition(v);
	return cTaskBgGimmick::Execute(pParam);
}

void cTaskBgGimmickTruck::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

}
