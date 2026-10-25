#include "kingdom/game/g03/cTaskBgGimmickMerryGoRound.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskBgGimmickMerryGoRound::cTaskBgGimmickMerryGoRound(ResNode* const& node0, ResNode* const& node1)
	: cTaskBgGimmick(node0, node1)
{
	SetName("cTaskBgGimmickMerryGoRound");
	Init();
}

void cTaskBgGimmickMerryGoRound::Init()
{
	Vector4 v = Vector4();
	AABB& reference = pFaceList()->ReadOnlyAABB();
	v[0] = (reference.This.max().x - reference.This.min().x) * 0.5f;
	v[1] = (reference.This.max().y - reference.This.min().y) * 0.5f;
	v[2] = (reference.This.max().z - reference.This.min().z) * 0.5f;
	v[3] = 1.0f;
	m_fLength = ::Constants::jetGeoVecLength(v);
	m_fLength *= 0.75f;
}

void cTaskBgGimmickMerryGoRound::Term()
{
	cTaskBgGimmick::Term();
}

bool cTaskBgGimmickMerryGoRound::Execute(cJetTask* pParam)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, this->Model->vTrans());
	::Constants::jetGeoVecDistance(Constants::PACN_APP()->pTaskCharPacman()->vTrans(), v);
	return cTaskBgGimmick::Execute(pParam);
}

void cTaskBgGimmickMerryGoRound::Draw(int nState, cJetTask* pParam)
{
	cTaskBgGimmick::Draw(nState, pParam);
}

}
