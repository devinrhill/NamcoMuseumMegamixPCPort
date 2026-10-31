#include "kingdom/game/g07/cTaskConsMultiMenuCurtain.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuCurtain::cTaskConsMultiMenuCurtain(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuCurtain(pConsMan)
{
}

cTaskConsMultiMenuCurtain::~cTaskConsMultiMenuCurtain()
{
}

// DOL 0x801cb3a4 [audited: anim table and priority compared word for word with the DOL data, step logic vs decomp]
void cTaskConsMultiMenuCurtain::SetData()
{
	nDrawPriority(2147483649u);
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetCurtainAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x801cb480 [audited: anim table and priority compared word for word with the DOL data, step logic vs decomp]
int cTaskConsMultiMenuCurtain::nGetCurtainAnmNo(int nAnmState)
{
	int obj[2] = {111, 112};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 2, "Array Over");
	return obj[nAnmState];
}

}
