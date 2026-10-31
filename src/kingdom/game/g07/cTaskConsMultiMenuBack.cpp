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

// DOL 0x801cadf8 [audited: anim table and priority compared word for word with the DOL data, step logic vs decomp]
int cTaskConsMultiMenuBack::nGetBackAnmNo(int nAnmState)
{
	int obj[4] = {91, 92, 93, 94};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "AnmState Over");
	return obj[nAnmState];
}

// DOL 0x801cae34 [audited: anim table and priority compared word for word with the DOL data, step logic vs decomp]
void cTaskConsMultiMenuBack::SetData()
{
	nDrawPriority(2147483672u);
	nHitPat() = 19;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetBackAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

}
