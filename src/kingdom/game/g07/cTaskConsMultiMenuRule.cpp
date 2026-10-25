#include "kingdom/game/g07/cTaskConsMultiMenuRule.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuMan.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuRule::cTaskConsMultiMenuRule(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuRule(pConsMan)
{
}

cTaskConsMultiMenuRule::~cTaskConsMultiMenuRule()
{
}

void cTaskConsMultiMenuRule::SetData()
{
	nDrawPriority(2147483670u);
	nHitPat() = 6;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetRuleAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	this->UnityUI = (static_cast<cTaskConsMultiMenuMan*>(pTaskConsMenuMan()))->UnityUI;
}

int cTaskConsMultiMenuRule::nGetRuleAnmNo(int nAnmState)
{
	int obj[4] = {79, 80, 81, 82};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	return obj[nAnmState];
}

}
