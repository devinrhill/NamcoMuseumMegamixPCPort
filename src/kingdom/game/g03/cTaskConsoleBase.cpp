#include "platform/devtools.hpp"
#include "kingdom/game/g03/cTaskConsoleBase.hpp"

#include <cmath>
#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskConsoleMan.hpp"
#include "kingdom/game/g03/cTaskCamera.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

// DOL 0x80175428 [audited: described by the 43rd-session pass] (priority 0x80000000 via Init)
cTaskConsoleBase::cTaskConsoleBase()
{
	SetName("cTaskConsoleBase");
	Init();
}

cTaskConsoleBase::~cTaskConsoleBase()
{
	Term();
}

uint cTaskConsoleBase::nGetState()
{
	return m_nState;
}

uint cTaskConsoleBase::nGetStep()
{
	return m_nStep;
}

void cTaskConsoleBase::Init()
{
	::Constants::jetGeoVecUnit(m_vTrans);
	m_nState = 0u;
	m_nStep = 0u;
	m_nStateNew = 0u;
	m_nStepNew = 0u;
	nDrawPriority(2147483648u);
}

void cTaskConsoleBase::Term()
{
}

// DOL 0x8018ea34 [audited: described by the 43rd-session pass] (anita UpdateFrame unless paused / HOME held)
bool cTaskConsoleBase::Execute(cJetTask* pParam)
{
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	return cTaskAnita::Execute(pParam);
}

// DOL 0x8018ea98 [audited: described by the 43rd-session pass] (visible manager: ortho projection, anita draw; then the state / step latch)
void cTaskConsoleBase::Draw(int nStatus, cJetTask* pParam)
{
	if (IsManagerVisible() && !devtools::G03HideHud())   // devtools: the Pac-N-Roll editor's "Hide the HUD" (not 1:1)
	{
		cTaskAnita::Draw(nStatus, pParam);
	}
	UpdateState();
}

bool cTaskConsoleBase::Regist(uint anita, string key)
{
	Regist(anita, key, 13);
	return true;
}

bool cTaskConsoleBase::Regist(uint anita, string key, int pat, int lay)
{
	pAnita(anita)->RegistData(ResourceDatabase::GetPath(key));
	pAnita(anita)->SetAnimNo(0);
	::Constants::jetGeoVecZero(pAnita(anita)->vTrans());
	Invisible(anita);
	return SetPosition(anita, pat, lay);
}

bool cTaskConsoleBase::RegistNotSetPos(uint anita, string key)
{
	pAnita(anita)->RegistData(ResourceDatabase::GetPath(key));
	pAnita(anita)->SetAnimNo(0);
	::Constants::jetGeoVecZero(pAnita(anita)->vTrans());
	Invisible(anita);
	return true;
}

bool cTaskConsoleBase::SetPosition(uint anita, int pat, int lay)
{
	CustomKingdomUtil::Assert(lay >= 0, "idx error");
	Vector4 pos = Vector4();
	if (!pAnita(anita)->GetLayerPos(pos, (uint)lay, pat))
	{
		return false;
	}
	pAnita(anita)->vTrans()[0] = pos[0];
	pAnita(anita)->vTrans()[1] = pos[1];
	return true;
}

bool cTaskConsoleBase::SetAnim(uint anita, int ano)
{
	return pAnita(anita)->SetAnimNo(ano);
}

bool cTaskConsoleBase::ChangeAnim(uint anita, int ano)
{
	return pAnita(anita)->SetAnimNo(ano, false);
}

bool cTaskConsoleBase::IsAnimEnd(uint anita)
{
	return pAnita(anita)->IsState(65536u);
}

bool cTaskConsoleBase::IsVisible(uint anita)
{
	return pAnita(anita)->IsState(1u);
}

void cTaskConsoleBase::Visible(uint anita)
{
	pAnita(anita)->SetState(1u);
}

void cTaskConsoleBase::Invisible(uint anita)
{
	pAnita(anita)->SetState(1u, false);
}

void cTaskConsoleBase::SetState(uint num)
{
	if (m_nState != num)
	{
		SetStep(0u);
		m_nStateNew = num;
	}
}

void cTaskConsoleBase::SetStep(uint num)
{
	if (m_nStep != num && m_nState == m_nStateNew)
	{
		m_nStepNew = num;
	}
}

void cTaskConsoleBase::UpdateState()
{
	m_nState = m_nStateNew;
	m_nStep = m_nStepNew;
}

bool cTaskConsoleBase::IsManagerVisible()
{
	// DOL 0x8018ea98: Draw only draws while the HUD manager (app +0x340) says it is visible (cTaskConsoleMan::Invisible / Visible: the Golvis04 death demo
	// hides the whole HUD). The manager does not exist in the top menu (hub), where there are no HUD tasks anyway
	cTaskConsoleMan* man = Constants::PACN_APP()->pTaskConsole();
	return man == nullptr || man->IsVisible();
}

// DOL 0x8018a200: T = viewport * projection * camera matrix; clip = T * (position, w); the point is on screen when 0 <= x <= 640, 0 <= y <= 480 and its
// z / w <= 0 (in front of the camera, inside the far plane). The result is then moved to the HUD layout (x - 320, 240 - y). The game camera is
// perspective, fov 40 degrees, 4:3, near 0.1, far 32768 (cTaskCamera::Update), looking from vPos to vTarget with up (0, 1, 0)
bool cTaskConsoleBase::WorldToHud(const Vector4& world, Vector4& out)
{
	cTaskCamera* cam = Constants::PACN_APP()->pTaskCamera();
	if (cam == nullptr)
	{
		return false;
	}
	const float eye[3] = {cam->vPos()[0], cam->vPos()[1], cam->vPos()[2]};
	float zAxis[3] = {eye[0] - cam->vTarget()[0], eye[1] - cam->vTarget()[1], eye[2] - cam->vTarget()[2]};
	auto norm = [](float* v) {
		const float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		if (l > 0.0f) { v[0] /= l; v[1] /= l; v[2] /= l; }
	};
	norm(zAxis);
	float xAxis[3] = {1.0f * zAxis[2] - 0.0f * zAxis[1], 0.0f * zAxis[0] - 0.0f * zAxis[2], 0.0f * zAxis[1] - 1.0f * zAxis[0]};   // up (0, 1, 0) x z
	norm(xAxis);
	const float yAxis[3] = {zAxis[1] * xAxis[2] - zAxis[2] * xAxis[1], zAxis[2] * xAxis[0] - zAxis[0] * xAxis[2], zAxis[0] * xAxis[1] - zAxis[1] * xAxis[0]};
	const float d[3] = {world[0] - eye[0], world[1] - eye[1], world[2] - eye[2]};
	const float xv = xAxis[0] * d[0] + xAxis[1] * d[1] + xAxis[2] * d[2];
	const float yv = yAxis[0] * d[0] + yAxis[1] * d[1] + yAxis[2] * d[2];
	const float depth = -(zAxis[0] * d[0] + zAxis[1] * d[1] + zAxis[2] * d[2]);
	const float farZ = 32768.0f;
	if (depth <= 0.0f || depth > farZ)
	{
		return false;
	}
	const float f = 1.0f / std::tan(40.0f * 0.5f * 3.14159265f / 180.0f);
	const float ndcX = (f * 0.75f * xv) / depth;   // f / aspect, aspect = 640 / 480
	const float ndcY = (f * yv) / depth;
	const float sx = ndcX * 320.0f + 320.0f;
	const float sy = -ndcY * 240.0f + 240.0f;
	if (sx < 0.0f || sx > 640.0f || sy < 0.0f || sy > 480.0f)
	{
		return false;
	}
	out = Vector4(sx - 320.0f, 240.0f - sy, 0.0f, 1.0f);
	return true;
}

}
