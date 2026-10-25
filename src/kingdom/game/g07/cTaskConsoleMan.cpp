#include "kingdom/game/g07/cTaskConsoleMan.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cTaskConsBossLife.hpp"
#include "kingdom/game/g07/cTaskConsFade.hpp"
#include "kingdom/game/g07/cTaskConsItemMultiPlay.hpp"
#include "kingdom/game/g07/cTaskConsItemSinglePlay.hpp"
#include "kingdom/game/g07/cTaskConsMultiSetup.hpp"
#include "kingdom/game/g07/cTaskConsReady.hpp"
#include "kingdom/game/g07/cTaskConsSingleSetup.hpp"
#include "kingdom/game/g07/cTaskConsSlopeEffect.hpp"

namespace kingdom::game::g07
{

cTaskConsoleMan::cTaskConsoleMan()
{
	Init();
}

void cTaskConsoleMan::Free()
{
	Term();
}

cTaskAnita* cTaskConsoleMan::pGetConsole(uint set)
{
	CustomKingdomUtil::Assert(set < 9, "muripo!");
	return m_pAnita[set];
}

uint& cTaskConsoleMan::nPriority()
{
	return m_nPriority;
}

void cTaskConsoleMan::Init()
{
	m_nPriority = 2147483648u;
}

void cTaskConsoleMan::Create()
{
	for (uint num = 0u; num < 9; num++)
	{
		if (s_console_task_tbl[num] != nullptr)
		{
			cTaskAnita* cTaskAnita2 = s_console_task_tbl[num]();
			m_pAnita[num] = cTaskAnita2;
			ConnectChild(cTaskAnita2);
		}
		else
		{
			m_pAnita[num] = nullptr;
		}
	}
}

void cTaskConsoleMan::Term()
{
	auto& pAnita = m_pAnita;
	for (cTaskAnita* cTaskAnita2 : pAnita)
	{
		if (cTaskAnita2 != nullptr)
		{
			cTaskConsSingleSetup* cTaskConsSingleSetup2 = dynamic_cast<cTaskConsSingleSetup*>(cTaskAnita2);
			if ((cTaskConsSingleSetup2 != nullptr))
			{
				cTaskConsSingleSetup2->Term();
			}
			else if (cTaskConsMultiSetup* cTaskConsMultiSetup2 = dynamic_cast<cTaskConsMultiSetup*>(cTaskAnita2))
			{
				cTaskConsMultiSetup2->Term();
			}
		}
	}
}

bool cTaskConsoleMan::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskConsoleMan::ItemSetup()
{
}

}
