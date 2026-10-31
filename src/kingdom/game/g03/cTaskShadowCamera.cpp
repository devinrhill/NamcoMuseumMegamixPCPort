#include "kingdom/game/g03/cTaskShadowCamera.hpp"

#include <cmath>

#include "cJetTask.hpp"
#include "GamePad.hpp"
#include "kingdom/NativeScene.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

cTaskShadowCamera::cTaskShadowCamera()
{
	SetName("cTaskConsoleMan");
	Init();
}

cTaskShadowCamera::~cTaskShadowCamera()
{
	Term();
}

// DOL 0x80188384 [audited: per AUDIT_G03_G07 row]: the light camera of the depth shadows (mode 0: straight above the target), the box by the round (+0x1b4 near, +0x1b8 far, +0x1bc factor)
void cTaskShadowCamera::Init()
{
	Constants::PACN_APP();
	kingdom::cKingdomShadow& sh = kingdom::NativeScene::Shadow();
	sh.SetMode(0);
	sh.SetTarget(0.0f, 0.0f, 0.0f);
	sh.SetAngles(1.5707964f, 0.0f, 0.0f);
	const uint round = KingdomGlobal::PacnRollSys->nRound();   // DOL 0x8012aeac [audited: described by the 43rd-session pass]: the sys's first word
	if (round == 4u)
	{
		sh.SetOrtho(40.0f, 40.0f, 20.0f, 90.0f);
		sh.SetRadius(60.0f);
		m_fNear = 20.0f; m_fFar = 92.0f; m_fFacter = 18.0f;
	}
	else if (round == 6u)
	{
		sh.SetOrtho(40.0f, 40.0f, 20.0f, 105.0f);
		sh.SetRadius(60.0f);
		m_fNear = 20.0f; m_fFar = 107.0f; m_fFacter = 18.0f;
	}
	else
	{
		sh.SetOrtho(40.0f, 40.0f, 20.0f, 60.0f);
		sh.SetRadius(40.0f);
		m_fNear = 20.0f; m_fFar = 62.0f; m_fFacter = 18.0f;
	}
}

void cTaskShadowCamera::Term()
{
}

// DOL 0x801884ec [audited: per AUDIT_G03_G07 row]: the light follows Pac-Man (ahead of him along the camera's view direction) and its box grows with the camera distance
bool cTaskShadowCamera::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* app = Constants::PACN_APP();
	cTaskCharPacman* pac = app->pTaskCharPacman();
	if (Constants::IS_PAC_PAUSE() || (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0 || pac == nullptr) return true;
	kingdom::cKingdomShadow& sh = kingdom::NativeScene::Shadow();
	const Vector4 cam = app->pTaskCamera()->vPos(), camTarget = app->pTaskCamera()->vTarget(), pp = pac->vTrans();
	if (!KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		float v[3] = {camTarget[0] - cam[0], camTarget[1] - cam[1], camTarget[2] - cam[2]};
		const float len = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		float num = 40.0f * len / m_fFacter;
		v[1] = 0.0f;
		float t[3];
		if (v[0] * v[0] + v[2] * v[2] < 1.17549435e-38f * 2.0f)
		{
			t[0] = pp[0]; t[1] = pp[1]; t[2] = pp[2];
		}
		else
		{
			const float l = std::sqrt(v[0] * v[0] + v[2] * v[2]);
			const float k = 0.28f * num / l;
			t[0] = pp[0] + v[0] * k; t[1] = pp[1]; t[2] = pp[2] + v[2] * k;
		}
		if (num < 15.0f) num = 15.0f;
		sh.SetOrtho(num, num, m_fNear, m_fFar);
		sh.SetTarget(t[0], t[1], t[2]);
	}
	else
	{
		const float d[3] = {pp[0] - cam[0], pp[1] - cam[1], pp[2] - cam[2]};
		const float len = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
		float num = len != 0.0f ? 40.0f * len / m_fFacter : 0.0f;
		if (num < 15.0f) num = 15.0f;
		sh.SetOrtho(num, num, m_fNear, m_fFar);
		sh.SetTarget(pp[0], pp[1], pp[2]);
	}
	return true;
}

void cTaskShadowCamera::Draw(int nStatus, cJetTask* pParam)
{
}

}
