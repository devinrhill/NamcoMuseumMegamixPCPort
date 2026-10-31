#include "kingdom/game/g07/cTaskConsFade.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cG07MotosSys.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

cTaskConsFade::cTaskConsFade()
{
	Init();
}

cTaskConsFade::~cTaskConsFade()
{
	Free();
}

void cTaskConsFade::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

cTaskAnita* cTaskConsFade::Construct()
{
	return new cTaskConsFade();
}

// DOL 0x800fd184 [audited: per AUDIT_G03_G07 row] (ctor 0x801f6c78)
void cTaskConsFade::Init()
{
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
	pAnita(0u)->SetAnimNo(269);
	pAnita(0u)->SetState(3u);
}

void cTaskConsFade::Term()
{
}

// DOL 0x800fd22c [audited: per AUDIT_G03_G07 row]
bool cTaskConsFade::Execute(cJetTask* pParam)
{
	return BaseExecute(pParam);
}

// DOL 0x800fd254 [audited: per AUDIT_G03_G07 row]: anim 269 / 272 (step 1), 274 (boss), 283 (step 5), 271
void cTaskConsFade::Draw(int nStatus, cJetTask* pParam)
{
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 1)
	{
		if (KingdomGlobal::MotosSys->IsBossStage())
		{
			SetAnmDiffCurrent(0u, 272);
		}
		else
		{
			SetAnmDiffCurrent(0u, 269);
		}
	}
	else if (KingdomGlobal::MotosSys->IsBossStage())
	{
		SetAnmDiffCurrent(0u, 274);
	}
	else if (KingdomGlobal::g_pMotosGame->nStep(0) == 5)
	{
		SetAnmDiffCurrent(0u, 283);
	}
	else
	{
		SetAnmDiffCurrent(0u, 271);
	}
	BaseDraw(nStatus, pParam);
}

}
