#include "kingdom/game/g07/cTaskConsMultiMenuMiniArrow.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuMiniArrow::cTaskConsMultiMenuMiniArrow(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuArrow(pConsMan)
{
}

cTaskConsMultiMenuMiniArrow::~cTaskConsMultiMenuMiniArrow()
{
}

void cTaskConsMultiMenuMiniArrow::SetData(uint nType)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 6, "ArrowType Over");
	nDrawPriority(2147483655u);
	nArrowType() = nType;
	bActive() = true;
	nHitPat() = 17;
	int pat = 4;
	uint array[6] = {5u, 2u, 4u, 1u, 3u, 0u};
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetArrowAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	pAnita(0u)->GetLayerPos(pAnita(0u)->vTrans(), array[nArrowType()], pat);
}

int cTaskConsMultiMenuMiniArrow::nGetArrowAnmNo(int nAnmState)
{
	int obj[6][4] = {{69, 70, 71, 72}, {73, 74, 75, 76}, {69, 70, 71, 72}, {73, 74, 75, 76}, {69, 70, 71, 72}, {73, 74, 75, 76}};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	CustomKingdomUtil::Assert(0 <= nArrowType() && nArrowType() < 6, "ArrowType Over");
	return obj[nArrowType()][nAnmState];
}

}
