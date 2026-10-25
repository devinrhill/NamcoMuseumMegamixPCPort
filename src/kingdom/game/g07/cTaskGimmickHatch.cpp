#include "kingdom/game/g07/cTaskGimmickHatch.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g07
{

uint cTaskGimmickHatch::get_HATCH_CLOSE_TIME()
{
	if (!CustomKingdomUtil::IF(nWork(2)))
	{
		return 180u;
	}
	return nWork(2);
}

uint cTaskGimmickHatch::get_HATCH_2OPEN_TIME()
{
	return 40u;
}

uint cTaskGimmickHatch::get_HATCH_OPEN_TIME()
{
	if (!CustomKingdomUtil::IF(nWork(3)))
	{
		return 180u;
	}
	return nWork(3);
}

uint cTaskGimmickHatch::get_HATCH_2CLOSE_TIME()
{
	return 40u;
}

uint cTaskGimmickHatch::get_HATCHI_TIME_TOTAL()
{
	return get_HATCH_CLOSE_TIME() + get_HATCH_2OPEN_TIME() + get_HATCH_OPEN_TIME() + get_HATCH_2CLOSE_TIME();
}

uint cTaskGimmickHatch::get_HATCH_CLOSE_MODE_START()
{
	return 0u;
}

uint cTaskGimmickHatch::get_HATCH_CLOSE_MODE_END()
{
	return get_HATCH_CLOSE_TIME();
}

uint cTaskGimmickHatch::get_HATCH_2OPEN_MODE_START()
{
	return get_HATCH_CLOSE_MODE_END();
}

uint cTaskGimmickHatch::get_HATCH_2OPEN_MODE_END()
{
	return get_HATCH_2OPEN_MODE_START() + get_HATCH_2OPEN_TIME();
}

uint cTaskGimmickHatch::get_HATCH_OPEN_MODE_START()
{
	return get_HATCH_2OPEN_MODE_END();
}

uint cTaskGimmickHatch::get_HATCH_OPEN_MODE_END()
{
	return get_HATCH_OPEN_MODE_START() + get_HATCH_OPEN_TIME();
}

uint cTaskGimmickHatch::get_HATCH_2CLOSE_MODE_START()
{
	return get_HATCH_OPEN_MODE_END();
}

uint cTaskGimmickHatch::get_HATCH_2CLOSE_MODE_END()
{
	return get_HATCH_2CLOSE_MODE_START() + get_HATCH_2CLOSE_TIME();
}

cTaskGimmickBase* cTaskGimmickHatch::Construct()
{
	return new cTaskGimmickHatch();
}

cTaskGimmickHatch::cTaskGimmickHatch()
{
	Init();
}

void cTaskGimmickHatch::Init()
{
	m_nMode = 2u;
	m_fHatchComa = 0.0f;
}

void cTaskGimmickHatch::Term()
{
	cTaskGimmickBase::Term();
}

bool cTaskGimmickHatch::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_GIMMICK_HATCH");
	string array[3] = {"TSA_GIMMICK_HATCH", "CLA_GIMMICK_HATCH", "CHA_GIMMICK_HATCH"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->AttachShadow();
	cKingdomModel2->SetLightSetIdx(1);
	cKingdomModel2->SetFogIdx(1);
	cKingdomModel2->SetUpdateRateAnmChr(0u, 1.0f);
	return true;
}

bool cTaskGimmickHatch::Execute(cJetTask* pParam)
{
	Exec();
	pModel()->SetFrameAnmAll(0u, m_fHatchComa);   // native frames (the C# passed Unity seconds)
	return true;
}

bool cTaskGimmickHatch::Exec()
{
	if (!CustomKingdomUtil::IF(m_nFlag & 8))
	{
		return true;
	}
	uint num = Constants::get_GAME_COUNTER();
	if (nWork(1) != 0)
	{
		num += get_HATCH_OPEN_MODE_START();
	}
	uint num2 = num % get_HATCHI_TIME_TOTAL();
	if (get_HATCH_CLOSE_MODE_START() <= num2 && num2 < get_HATCH_CLOSE_MODE_END())
	{
		m_fHatchComa = 0.0f;
		m_nComaNum = 0u;
		m_nMode = 2u;
	}
	else if (get_HATCH_2OPEN_MODE_START() <= num2 && num2 < get_HATCH_2OPEN_MODE_END())
	{
		if (num2 == get_HATCH_2OPEN_MODE_START())
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE027_HATCH_SE);
		}
		if (!Constants::MOTOS_IS_PAUSE())
		{
			m_fHatchComa++;
		}
	}
	else if (get_HATCH_OPEN_MODE_START() <= num2 && num2 < get_HATCH_OPEN_MODE_END())
	{
		m_fHatchComa = 40.0f;
	}
	else
	{
		if (num2 == get_HATCH_OPEN_MODE_END())
		{
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE027_HATCH_SE);
		}
		if (!Constants::MOTOS_IS_PAUSE())
		{
			m_fHatchComa--;
		}
	}
	m_nComa = (int)m_fHatchComa;
	return true;
}

bool cTaskGimmickHatch::IsStepOnCntW(Vector4 pWTrans)
{
	float num = KingdomGlobal::g_pMotosGame->pMap()->fPanelSize() * 2.0f;
	if (vWTrans()[0] - num > pWTrans[0] || vWTrans()[0] + num < pWTrans[0] || vWTrans()[2] - num > pWTrans[2] || vWTrans()[2] + num < pWTrans[2])
	{
		return false;
	}
	return true;
}

bool cTaskGimmickHatch::IsFall(Vector4 pMTrans)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, pMTrans);
	float num = KingdomGlobal::g_pMotosGame->pMap()->fPanelSize();
	if (!(vWTrans()[0] - num > pMTrans[0]) && !(vWTrans()[0] + num < pMTrans[0]) && !(vWTrans()[2] - num > pMTrans[2]) && !(vWTrans()[2] + num < pMTrans[2]))
	{
		float num2 = 0.0f;
		if (m_nComa > 9 && (num2 = (float)(m_nComa - 9) / 16.0f) > 1.0f)
		{
			num2 = 1.0f;
		}
		float num3 = 1.0f * num2;
		float num4 = Constants::tspVector3Distance(vMTrans(), v);
		if (0.3f < num3 && num4 < num3)
		{
			return true;
		}
	}
	return false;
}

}
