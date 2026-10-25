#include "kingdom/cTaskPBPtkMan.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkMan.hpp"

namespace kingdom
{

cKingdomPtkMan* cTaskPBPtkMan::get_KingdomPtkMan()
{
	return m_particleManager;
}

cTaskPBPtkMan::cTaskPBPtkMan()
{
	m_particleManager = new cKingdomPtkMan();
	m_bDebugDraw = false;
	this->set_IsClipping(false);
	SetName("cTaskPBPtkMan");
}

cTaskPBPtkMan::~cTaskPBPtkMan()
{
}

void cTaskPBPtkMan::Term()
{
	if (m_particleManager != nullptr)
	{
		m_particleManager->Term();
		m_particleManager = nullptr;
	}
}

bool cTaskPBPtkMan::Execute(cJetTask* pParam)
{
	m_particleManager->UpdateFrame();
	return true;
}

void cTaskPBPtkMan::PostDraw(int nStatus, cJetTask* pParam)
{
	if (m_bDebugDraw)
	{
		m_particleManager->DebugDraw(0, 348, 0u);
	}
}

void cTaskPBPtkMan::DrawScnRootXlu()
{
	if (nullptr == m_camera)
	{
		// TODO(unity): m_camera = Camera.main;
	}
	if (nullptr == m_gazePoint)
	{
		// TODO(unity): m_gazePoint = GameObject.Find("GazePoint");
	}
	Vector4 vPos = Vector4(); // TODO(unity): vPos = m_camera.transform.position
	Vector4 vTarget = Vector4(); // TODO(unity): vTarget = m_gazePoint.transform.position
	m_particleManager->Draw(m_camera, vPos, vTarget);
}

bool cTaskPBPtkMan::IsDebugDraw()
{
	return m_bDebugDraw;
}

}
