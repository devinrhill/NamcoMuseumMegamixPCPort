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

cTaskBgGimmickTruck::cTaskBgGimmickTruck(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickTruck");
	Init();
}

void cTaskBgGimmickTruck::Init()
{
	m_nStep = 0u;
	OnState(64u);
	ResNode* resNode = (new ResMdl(this->Model->pScnMdl()))->GetResNode("root");
	m_moveRoot = resNode->This;
	std::vector<SkinnedMeshRenderer*> componentsInChildren; // TODO(unity): componentsInChildren = base.Model.pScnMdl().GetComponentsInChildren<SkinnedMeshRenderer>()
	for (SkinnedMeshRenderer* obj : componentsInChildren)
	{
		Bounds localBounds = Bounds(); // TODO(unity): localBounds = obj.localBounds
		localBounds.Expand(1000.0f); // the converter dropped the argument (C#: Expand(1000f))
		// TODO(unity): obj.localBounds = localBounds;
	}
}

void cTaskBgGimmickTruck::Term()
{
	if (nullptr != m_SndHnd)
	{
		SoundManager::get_SE()->Stop(m_SndHnd);
	}
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickTruck::Execute(cJetTask* pParam)
{
	if (m_nStep == 0)
	{
		// TODO(unity): m_SndHnd = SoundManager.SE.PlaySE(ESEKind.PACN_ROLL_SND_SE040_TRAIN, bLoop: true, 5f, 1f, SoundSe.EMixerType.eNORMAL, SoundSeObject.ESeType.eNORMAL, 1f, SoundSe.ESameKindCheck.eLATE_IMMEDIATE, SoundSeObject.ESeGroup.eNORMAL, m_moveRoot.gameObject, 1f, 7.5f, checkStalkDist: false);
		m_nStep = 1u;
	}
	Vector4 v = Vector4::zero;
	::Constants::jetGeoMtxGetTrans(v, pMtx());
	return cTaskBgGimmick::Execute(pParam);
}

void cTaskBgGimmickTruck::Draw(int nStatus, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nStatus, pParam);
}

}
