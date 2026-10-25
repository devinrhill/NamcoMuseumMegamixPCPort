#include "kingdom/cTaskPushBack.hpp"

#include "cJetTask.hpp"

namespace kingdom
{

bool cTaskPushBack::get_IsClipping()
{
	return m_bClipping;
}

void cTaskPushBack::set_IsClipping(bool value)
{
	m_bClipping = value;
}

int cTaskPushBack::get_nDrawCamID()
{
	return m_nDrawCamID;
}

void cTaskPushBack::set_nDrawCamID(int value)
{
	m_nDrawCamID = value;
}

void cTaskPushBack::Init()
{
	m_bClipping = true;
	m_nDrawCamID = -1;
	m_nPushBackNum = 1u;
	SetName("cTaskPushBack");
}

void cTaskPushBack::Term()
{
}

cTaskPushBack::cTaskPushBack()
{
	Init();
}

cTaskPushBack::~cTaskPushBack()
{
	Term();
}

bool cTaskPushBack::PostExecute(cJetTask* pParam)
{
	return true;
}

void cTaskPushBack::Draw(int nStatus, cJetTask* pParam)
{
	DrawScnRootOpa();
	DrawScnRootXlu();
	PushBack();
}

void cTaskPushBack::DrawScnRootOpa()
{
}

void cTaskPushBack::DrawScnRootXlu()
{
}

Vector4 cTaskPushBack::vClipTrans()
{
	return Vector4::zero;
}

float cTaskPushBack::fClipFovy()
{
	return 0.0f;
}

bool cTaskPushBack::IsDrawShadowDepth()
{
	return false;
}

bool cTaskPushBack::PushBack()
{
	SubmitNative();
	return true;
}

}
