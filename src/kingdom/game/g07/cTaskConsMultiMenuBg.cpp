#include "kingdom/game/g07/cTaskConsMultiMenuBg.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuBg::cTaskConsMultiMenuBg(cTaskConsMenuManBase* const& pConsMan, const uint& nWorldNo)
	: cTaskConsMenuBg(pConsMan, nWorldNo)
{
}

cTaskConsMultiMenuBg::~cTaskConsMultiMenuBg()
{
}

// DOL 0x801caf7c [audited: anim table and priority compared word for word with the DOL data, step logic vs decomp]
void cTaskConsMultiMenuBg::SetData()
{
	nDrawPriority(65537u);
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_BG"));
	pAnita(0u)->SetAnimNo(nGetBgAnmNo());
}

// DOL 0x801cb03c [audited: anim table and priority compared word for word with the DOL data, step logic vs decomp]
int cTaskConsMultiMenuBg::nGetBgAnmNo()
{
	CustomKingdomUtil::Assert(nWorldNo() < 10, "error");
	static const int table[10] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
	return table[nWorldNo()];
}

}
