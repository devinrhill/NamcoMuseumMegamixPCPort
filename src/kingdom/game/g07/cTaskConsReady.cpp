#include "kingdom/game/g07/cTaskConsReady.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "platform/devtools_g07edit.hpp"

namespace kingdom::game::g07
{

cTaskConsReady::cTaskConsReady()
{
	Init();
}

cTaskConsReady::~cTaskConsReady()
{
	Free();
}

void cTaskConsReady::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

cTaskAnita* cTaskConsReady::Construct()
{
	return new cTaskConsReady();
}

void cTaskConsReady::Init()
{
	m_nTimer = 0u;
	m_bGo = false;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(0u)->SetAnimNo(138);
	pAnita(0u)->SetState(3u, false);
}

void cTaskConsReady::Term()
{
}

bool cTaskConsReady::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 7)
	{
		if (KingdomGlobal::g_pMotosGame->nStep(1) == 3)
		{
			if (!m_bGo)
			{
				m_bGo = true;
				pAnita(0u)->SetAnimNo(140);
			}
		}
		else
		{
			pAnita(0u)->SetState(3u);
			if (IsAnmNoSame(0u, 138) && IsAnmEnd(0u))
			{
				SetAnm(0u, 139);
			}
		}
	}
	else if (KingdomGlobal::g_pMotosGame->nStep(0) == 8)
	{
		m_nTimer++;
		if (m_nTimer == 60 && pAnita(0u)->GetAnimNo() != 142)
		{
			pAnita(0u)->SetAnimNo(142);
		}
	}
	if (devtools::G07SkipItemSet()) Invisible(0u); // devtools: no READY console with "Skip the item settings" (not 1:1)
	return cTaskConsoleBase::Execute(pParam);
}

}
