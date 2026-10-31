#include "kingdom/game/g07/cTaskConsMultiMenuStart.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuStart::cTaskConsMultiMenuStart(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuMulti(pConsMan)
{
}

cTaskConsMultiMenuStart::~cTaskConsMultiMenuStart()
{
}

// DOL 0x801ced60 [audited: anim tables and priorities compared word for word with the DOL data, step logic vs decomp]
void cTaskConsMultiMenuStart::SetData()
{
	nDrawPriority(2147483654u);
	nHitPat() = 23;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x801cee44 [audited: anim tables and priorities compared word for word with the DOL data, step logic vs decomp]
int cTaskConsMultiMenuStart::nGetAnmNo(int nAnmState)
{
	int obj[4] = {118, 119, 120, 121};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	return obj[nAnmState];
}

}
