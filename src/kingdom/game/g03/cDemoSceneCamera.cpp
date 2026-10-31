#include "kingdom/game/g03/cDemoSceneCamera.hpp"
#include "kingdom/NativeScene.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

namespace kingdom::game::g03
{

cDemoSceneCamera::cDemoSceneCamera()
{
}

cDemoSceneCamera::~cDemoSceneCamera()
{
	if (Active() == this)
	{
		Active() = nullptr;
	}
	delete m_pAnm;
	delete m_pBrres;
}

cDemoSceneCamera*& cDemoSceneCamera::Active()
{
	static cDemoSceneCamera* s_pActive = nullptr;
	return s_pActive;
}

bool cDemoSceneCamera::Bind(const std::string& entry)
{
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(entry, size);
	if (d == nullptr)
	{
		return false;
	}
	wii::Brres* b = new wii::Brres();
	wii::ScnAnim* a = new wii::ScnAnim();
	if (!b->Load(d, size) || !a->Load(*b))
	{
		delete a;
		delete b;
		return false;
	}
	delete m_pAnm;
	delete m_pBrres;
	m_pBrres = b;
	m_pAnm = a;
	return true;
}

void cDemoSceneCamera::SetAnm()
{
	m_bSet = m_pAnm != nullptr;
	m_fFrame = 0.0f;
}

void cDemoSceneCamera::UpdateFrame()
{
	if (!m_bSet)
	{
		return;
	}
	m_fFrame += 1.0f;
	const float end = (float)m_pAnm->numFrame;
	if (m_pAnm->loops)
	{
		if (end > 0.0f && m_fFrame >= end)
		{
			m_fFrame -= end;
		}
	}
	else if (m_fFrame > end)
	{
		m_fFrame = end;   // UNVERIFIED: nw4r PlayPolicy_Onetime keeps the frame running; sampling past the end holds the last key either way
	}
}

bool cDemoSceneCamera::Apply() const
{
	if (!m_bSet)
	{
		return false;
	}
	wii::CameraResult cam;
	if (!m_pAnm->SampleCamera("", m_fFrame, cam))
	{
		return false;
	}
	float target[3];
	cam.LookAt(target);
	kingdom::NativeScene::SetCamera(cam.pos, target, cam.fovy, cam.nearZ, cam.farZ);
	return true;
}

}
