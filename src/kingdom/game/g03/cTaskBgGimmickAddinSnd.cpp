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

cTaskBgGimmickAddinSnd::cTaskBgGimmickAddinSnd(ResNode* const& node0, ResNode* const& node1, const ESEKind& sndName)
	: cTaskBgGimmick(node0, node1)
{
	m_pSndName = sndName;
	SetName("cTaskBgGimmickAddinSnd");
	Init();
}

void cTaskBgGimmickAddinSnd::Init()
{
	Vector4 v = Vector4();
	AABB& reference = pFaceList()->ReadOnlyAABB();
	v[0] = (reference.This.max().x - reference.This.min().x) * 0.5f;
	v[1] = (reference.This.max().y - reference.This.min().y) * 0.5f;
	v[2] = (reference.This.max().z - reference.This.min().z) * 0.5f;
	v[3] = 1.0f;
	m_fLength = ::Constants::jetGeoVecLength(v);
	// WII (8014e128): m_fLength is always the half diagonal of the model's bounding volume; the port
	// forced 5.0 for the pendulum. Only the port's sound-position object is kept here.
	if (m_pSndName == ESEKind::PACN_ROLL_SND_SE114_PENDULUM)
	{
		// TODO(unity): m_pendulumSEPosObj = new GameObject();
		// TODO(unity): m_pendulumSEPosObj.name = "PendulumSEPosObj";
	}
}

void cTaskBgGimmickAddinSnd::Term()
{
	if (nullptr != m_pendulumSEPosObj)
	{
		ReleaseObjectManager::Regist(m_pendulumSEPosObj);
	}
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickAddinSnd::Execute(cJetTask* pParam)
{
	// WII (8014e230): the gimmick's looping SE plays while pac-man is within m_fLength of it and fades
	// out over 500 frames when he leaves; the 3D sound source follows the gimmick. The port dropped all
	// of this (m_SndHnd was declared but never used).
	Vector4 vPos;
	::Constants::jetGeoVecCopy(vPos, this->Model->vTrans());
	if (::Constants::jetGeoVecDistance(Constants::PACN_APP()->pTaskCharPacman()->vTrans(), vPos) < m_fLength)
	{
		if (nullptr == m_SndHnd)
		{
			m_SndHnd = SoundManager::get_SE()->PlaySE(m_pSndName);
		}
	}
	else if (nullptr != m_SndHnd)
	{
		SoundManager::get_SE()->Stop(m_SndHnd, 500.0f / 60.0f);
		m_SndHnd = nullptr;
	}
	// TODO(wii): move the 3D sound actor (created in Init, +0x520) to vPos every frame (80026620).
	return cTaskBgGimmick::Execute(pParam);
}

void cTaskBgGimmickAddinSnd::Draw(int nState, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nState, pParam);
}

}
