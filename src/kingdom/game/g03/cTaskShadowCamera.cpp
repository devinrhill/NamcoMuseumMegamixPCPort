#include "kingdom/game/g03/cTaskShadowCamera.hpp"

#include "cJetTask.hpp"
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

void cTaskShadowCamera::Init()
{
	Constants::PACN_APP();
}

void cTaskShadowCamera::Term()
{
}

bool cTaskShadowCamera::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (Constants::IS_PAC_PAUSE() || cTaskCharPacman2 == nullptr)
	{
		return true;
	}
	if (!KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		Vector4 v = Vector4::zero;
		::Constants::jetGeoVecUnit(v);
		::Constants::jetGeoVecSub(v, cAppG03PacnRollGame2->pTaskCamera()->vTarget(), cAppG03PacnRollGame2->pTaskCamera()->vPos());
		float num = 40.0f * ::Constants::jetGeoVecLength(v) / m_fFacter;
		v[1] = 0.0f;
		if (Constants::IsVecNormalize(v))
		{
			::Constants::jetGeoVecNormalize(v, v);
			::Constants::jetGeoVecScale(v, v, num * 0.28f);
			::Constants::jetGeoVecAdd(v, cTaskCharPacman2->vTrans(), v);
		}
		else
		{
			::Constants::jetGeoVecCopy(v, cTaskCharPacman2->vTrans());
		}
		if (num < 15.0f)
		{
			num = 15.0f;
		}
	}
	else
	{
		Vector4 v2;
		::Constants::jetGeoVecSub(v2, cTaskCharPacman2->vTrans(), cAppG03PacnRollGame2->pTaskCamera()->vPos());
		float num2 = ::Constants::jetGeoVecLength(v2);
		float num3 = 0.0f;
		if (num2 != 0.0f)
		{
			num3 = 40.0f * num2 / m_fFacter;
		}
		if (num3 < 15.0f)
		{
			num3 = 15.0f;
		}
	}
	return true;
}

void cTaskShadowCamera::Draw(int nStatus, cJetTask* pParam)
{
}

}
