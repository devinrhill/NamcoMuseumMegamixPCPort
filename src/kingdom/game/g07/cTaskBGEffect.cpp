#include "kingdom/game/g07/cTaskBGEffect.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"

#include "cJetTask.hpp"
#include "kingdom/cTaskScnRoot.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g07
{

const string cTaskBGEffect::ptk_tbl[8] = {"BG_PTK01", "BG_PTK02", "BG_PTK03", "BG_PTK04", "BG_PTK05", "BG_PTK06", "BG_PTK07", "BG_PTK08"};

cTaskBGEffect::cTaskBGEffect()
{
	Init();
}

cTaskBGEffect::~cTaskBGEffect()
{
	Term();
}

void cTaskBGEffect::DrawScnRootOpa(cTaskScnRoot* pTaskScnRoot)
{
	DrawScnRootXlu(pTaskScnRoot);
}

void cTaskBGEffect::DrawScnRootXlu(cTaskScnRoot* pTaskScnRoot)
{
}

void cTaskBGEffect::Init()
{
	this->get_KingdomPtkMan()->Create(8u, 16u, 32u, 512u);
	for (uint num = 0u; num < 8; num++)
	{
		if (!ptk_tbl[num].empty())
		{
			this->get_KingdomPtkMan()->RegistData(num, ptk_tbl[num]);
		}
	}
}

void cTaskBGEffect::Term()
{
}

bool cTaskBGEffect::Execute(cJetTask* pParam)
{
	if (!Constants::SYSTEM_IS_PAUSE() && KingdomGlobal::g_pMotosGame->nStep(0) != 1)
	{
		this->get_KingdomPtkMan()->UpdateFrame();
	}
	else
	{
		this->get_KingdomPtkMan()->UpdateFrame(0.0f);
	}
	return true;
}

}
