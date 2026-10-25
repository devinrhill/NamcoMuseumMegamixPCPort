#include "kingdom/game/g07/cTaskConsMultiMenuBack.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuBack::cTaskConsMultiMenuBack(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBack(pConsMan)
{
}

cTaskConsMultiMenuBack::~cTaskConsMultiMenuBack()
{
}

int cTaskConsMultiMenuBack::nGetBackAnmNo(int nAnmState)
{
	int obj[4] = {91, 92, 93, 94};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "AnmState Over");
	return obj[nAnmState];
}

void cTaskConsMultiMenuBack::SetData()
{
	nDrawPriority(2147483672u);
	nHitPat() = 19;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetBackAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

}
