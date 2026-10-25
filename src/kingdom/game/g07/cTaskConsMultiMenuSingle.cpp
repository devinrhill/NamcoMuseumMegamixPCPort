#include "kingdom/game/g07/cTaskConsMultiMenuSingle.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/g07/cTaskConsMultiMenuMan.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuSingle::cTaskConsMultiMenuSingle(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuMulti(pConsMan)
{
}

cTaskConsMultiMenuSingle::~cTaskConsMultiMenuSingle()
{
}

void cTaskConsMultiMenuSingle::SetData()
{
	nDrawPriority(2147483671u);
	nHitPat() = 21;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	this->UnityUI = (static_cast<cTaskConsMultiMenuMan*>(pTaskConsMenuMan()))->UnityUI;
}

int cTaskConsMultiMenuSingle::nGetAnmNo(int nAnmState)
{
	int obj[4] = {95, 96, 97, 98};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	return obj[nAnmState];
}

}
