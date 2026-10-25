#include "kingdom/game/g07/cTaskConsPacIconSingleSetting.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

float cTaskConsPacIconSingleSetting::get_PAC_ICON_POS_X_RIVISE()
{
	return 35.0f;
}

float cTaskConsPacIconSingleSetting::get_PAC_ICON_POS_Y_RIVISE()
{
	return -53.0f;
}

float cTaskConsPacIconSingleSetting::get_PAC_ICON_POS_X_FIRST()
{
	return 91.0f;
}

float cTaskConsPacIconSingleSetting::get_PAC_ICON_POS_Y_FIRST()
{
	return -83.0f;
}

cTaskConsPacIconSingleSetting::cTaskConsPacIconSingleSetting()
{
	Init();
}

cTaskConsPacIconSingleSetting::~cTaskConsPacIconSingleSetting()
{
	Free();
}

void cTaskConsPacIconSingleSetting::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

cTaskAnita* cTaskConsPacIconSingleSetting::Construct()
{
	return new cTaskConsPacIconSingleSetting();
}

uint& cTaskConsPacIconSingleSetting::nPlayerID()
{
	return m_nPlayerID;
}

void cTaskConsPacIconSingleSetting::Init()
{
	for (uint num = 0u; num < 15; num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num)->SetAnimNo(130);
		pAnita(num)->SetState(3u);
		Invisible(num);
	}
	::Constants::jetGeoVecUnit(vTrans());
	X() = get_PAC_ICON_POS_X_FIRST();
	Y() = get_PAC_ICON_POS_Y_FIRST();
	m_nPlayerID = 0u;
}

void cTaskConsPacIconSingleSetting::Term()
{
}

bool cTaskConsPacIconSingleSetting::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 1)
	{
		uint num = KingdomGlobal::g_pMotosGame->nPlayerParam();
		for (uint num2 = 0u; num2 < 3; num2++)
		{
			for (uint num3 = 0u; num3 < 5; num3++)
			{
				uint num4 = num3 + num2 * 5;
				uint num5 = num2 + 1;
				Visible(num4);
				if ((long)KingdomGlobal::g_pMotosGame->nStep(1) == (long)num2)
				{
					if (KingdomGlobal::pMotosGameSys->KeepData()[num]->m_nUseItem[num5] > num3)
					{
						SetAnm(num4, 135);
					}
					else
					{
						SetAnm(num4, 131);
					}
				}
				else if (KingdomGlobal::pMotosGameSys->KeepData()[num]->m_nUseItem[num5] > num3)
				{
					SetAnm(num4, 218);
				}
				else
				{
					SetAnm(num4, 215);
				}
				::Constants::jetGeoVecCopy(pAnita(num4)->vTrans(), vTrans());
				pAnita(num4)->vTrans()[0] += get_PAC_ICON_POS_X_RIVISE() * (float)num3;
				pAnita(num4)->vTrans()[1] += get_PAC_ICON_POS_Y_RIVISE() * (float)num2;
			}
		}
	}
	else if (KingdomGlobal::g_pMotosGame->nStep(0) == 7)
	{
		for (uint num6 = 0u; num6 < 15; num6++)
		{
			if (IsStateChange())
			{
				int animNo = pAnita(num6)->GetAnimNo();
				if (animNo == 134 || animNo == 135)
				{
					pAnita(num6)->SetAnimNo(137);
				}
				else
				{
					pAnita(num6)->SetAnimNo(133);
				}
			}
			if ((pAnita(num6)->GetState() & 1) != 0 && (pAnita(num6)->GetState() & 0x10000) != 0)
			{
				pAnita(num6)->SetState(3u, false);
			}
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
