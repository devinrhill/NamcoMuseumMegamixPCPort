#include "kingdom/game/g07/cTaskConsSingleClear.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

cTaskConsSingleClear::cTaskConsSingleClear()
{
	Init();
}

cTaskConsSingleClear::~cTaskConsSingleClear()
{
	Free();
}

void cTaskConsSingleClear::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

cTaskAnita* cTaskConsSingleClear::Construct()
{
	return new cTaskConsSingleClear();
}

void cTaskConsSingleClear::Init()
{
	m_nTimer = 0;
	int array[1] = {143};
	for (uint num = 0u; num < 1; num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num)->SetAnimNo(array[num]);
		Invisible(num);
	}
}

void cTaskConsSingleClear::Term()
{
}

bool cTaskConsSingleClear::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) != 3)
	{
		return cTaskConsoleBase::Execute(pParam);
	}
	switch (nStep())
	{
	case 0:
		Visible(0u);
		StepChange(1);
		break;
	case 1:
		if (IsAnmEnd(0u))
		{
			SetAnm(0u, 144);
			StepChange(2);
			m_nTimer = 0;
		}
		break;
	case 2:
		if (KingdomGlobal::g_pMotosGame->nStep(1) == 3)
		{
			SetAnm(0u, 145);
			StepChange(3);
		}
		break;
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
