#include "kingdom/MslRand.hpp"
#include "kingdom/game/g07/cTaskCamera.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskScnRoot.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cParamCamera.hpp"
#include <Constants.hpp>
#include "kingdom/NativeScene.hpp"

namespace kingdom::game::g07
{

cTaskCamera::cTaskCamera()
{
	Init();
}

cTaskCamera::cTaskCamera(cTaskScnRoot* pTaskScnRoot, string key)
{
	Init();
	Setup(pTaskScnRoot, key);
}

cTaskCamera::~cTaskCamera()
{
	Term();
}

Vector4& cTaskCamera::vPosition()
{
	return m_vPosition;
}

Vector4& cTaskCamera::vPositionTmp()
{
	return m_vPositionTmp;
}

Vector4& cTaskCamera::vTarget()
{
	return m_vTarget;
}

Vector4& cTaskCamera::vTargetTmp()
{
	return m_vTargetTmp;
}

float& cTaskCamera::fFovy()
{
	return m_fFovy;
}

float& cTaskCamera::fNear()
{
	return m_fNear;
}

float& cTaskCamera::fFar()
{
	return m_fFar;
}

float& cTaskCamera::fAspect()
{
	return m_fAspect;
}

void cTaskCamera::SaveVector()
{
	::Constants::jetGeoVecCopy(m_vPositionTmp, m_vPosition);
	::Constants::jetGeoVecCopy(m_vTargetTmp, m_vTarget);
}

void cTaskCamera::LoadVector()
{
	::Constants::jetGeoVecCopy(m_vPosition, m_vPositionTmp);
	::Constants::jetGeoVecCopy(m_vTarget, m_vTargetTmp);
}

void cTaskCamera::SetQuake(float range, int time)
{
	m_fRange = range;
	m_nTimer = time;
}

void cTaskCamera::Init()
{
	::Constants::jetGeoVecUnit(m_vPosition);
	::Constants::jetGeoVecUnit(m_vTarget);
	::Constants::jetGeoVecUnit(m_vSpeed);
	m_fFovy = 0.0f;
	m_fNear = 0.0f;
	m_fFar = 0.0f;
	m_fAspect = 0.0f;
	m_fRange = 0.0f;
	m_nTimer = 0;
}

void cTaskCamera::Term()
{
}

// DOL 0x801e23fc [audited: described by the 43rd-session pass] (cParamCamera::LoadValue inlined; also scene root +0x1d8 = 0: see UpdateMatrix)
void cTaskCamera::Setup(cTaskScnRoot* pTaskScnRoot, string key)
{
	cParamCamera* cParamCamera2 = new cParamCamera(key);
	CustomKingdomUtil::Assert(cParamCamera2->IsValid(), "error");
	::Constants::jetGeoVecCopy(m_vPosition, cParamCamera2->vGetPosition());
	::Constants::jetGeoVecCopy(m_vTarget, cParamCamera2->vGetTarget());
	m_fFovy = cParamCamera2->fGetFovy();
	m_fNear = cParamCamera2->fGetNear();
	m_fFar = cParamCamera2->fGetFar();
	m_fAspect = cParamCamera2->fGetAspect();
}

// DOL 0x801e27d4 [audited: constant getter vs disassembly]
bool cTaskCamera::Execute(cJetTask* pParam)
{
	return true;
}

// DOL 0x801e27dc [audited: described by the 43rd-session pass] (audited 44th: quake sign = rand() & 1; position / target / fov / near / far -> NativeScene camera)
void cTaskCamera::Draw(int nStatus, cJetTask* pParam)
{
	UpdateMatrix();
}

void cTaskCamera::UpdateMatrix()
{
	if (nullptr == m_mainCam)
	{
		// TODO(unity): m_mainCam = Camera.main;
	}
	if (nullptr == m_gazePoint)
	{
		// TODO(unity): m_gazePoint = GameObject.Find("GazePoint");
	}
	Vector4 v;
	::Constants::jetGeoVecCopy(v, m_vPosition);
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, m_vTarget);
	if (m_nTimer != 0 && !Constants::SYSTEM_IS_PAUSE())
	{
		// DOL 0x801e27dc: rand() & 1 picks the sign (the C#'s Random.Range(0, 1) was always 0, so it never alternated); y += +-range * 0.5
		v2[1] += (((kingdom::MslRand() & 1) == 0) ? m_fRange : -m_fRange) * 0.5f;
		m_nTimer--;
	}
	// native: the 3D pass camera (kingdom::NativeScene). Unity: position = v, LookAt(v2), fieldOfView = RAD2DEG(fovy), near, far
	{
		const float eye[3] = {v[0], v[1], v[2]}, target[3] = {v2[0], v2[1], v2[2]};
		kingdom::NativeScene::SetCamera(eye, target, ::Constants::JET_RAD2DEG(m_fFovy), m_fNear, m_fFar);
		// DOL 0x801e29e0 [audited: described by the 43rd-session pass]: camera +0xb0 = the CSV aspect (1.242 on the disc), and Setup 0x801e23fc cleared the scene root's flag +0x1d8, so
		// cTaskScnRoot::Draw 0x8009bc4c keeps it instead of the viewport's 4:3 (the 3D view is about 7 % wider than at 4:3)
		kingdom::NativeScene::SetFixedAspect(m_fAspect);
	}
	// TODO(unity): m_gazePoint.transform.position = m_vTarget;
}

void cTaskCamera::SetPan(Vector4 vPos, float fSpeed)
{
	float num = (m_vPosition[0] - vPos[0]) / 100.0f;
	float num2 = (m_vPosition[1] - vPos[1]) / 100.0f;
	float num3 = (m_vPosition[2] - vPos[2]) / 100.0f;
	if (!CustomKingdomUtil::IF(fSpeed))
	{
		fSpeed = 0.0005f;
	}
	if (num > fSpeed)
	{
		num = fSpeed;
	}
	else if (num < 0.0f - fSpeed)
	{
		num = 0.0f - fSpeed;
	}
	else
	{
		m_vSpeed[0] /= 1.3f;
	}
	if (num3 > fSpeed)
	{
		num3 = fSpeed;
	}
	else if (num3 < 0.0f - fSpeed)
	{
		num3 = 0.0f - fSpeed;
	}
	else
	{
		m_vSpeed[2] /= 1.3f;
	}
	m_vSpeed[0] += num;
	m_vSpeed[2] += num3;
	if (m_vSpeed[0] > 0.01f)
	{
		m_vSpeed[0] = 0.01f;
	}
	if (m_vSpeed[0] < -0.01f)
	{
		m_vSpeed[0] = -0.01f;
	}
	if (m_vSpeed[2] > 0.01f)
	{
		m_vSpeed[2] = 0.01f;
	}
	if (m_vSpeed[2] < -0.01f)
	{
		m_vSpeed[2] = -0.01f;
	}
	m_vPosition[0] -= m_vSpeed[0];
	m_vPosition[2] -= m_vSpeed[2];
	m_vTarget[0] -= m_vSpeed[0];
	m_vTarget[2] -= m_vSpeed[2];
	m_vPosition[1] -= num2;
}

}
