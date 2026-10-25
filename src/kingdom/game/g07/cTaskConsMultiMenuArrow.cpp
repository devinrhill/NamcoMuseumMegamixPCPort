#include "kingdom/game/g07/cTaskConsMultiMenuArrow.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuArrow::cTaskConsMultiMenuArrow(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuArrow(pConsMan)
{
}

cTaskConsMultiMenuArrow::~cTaskConsMultiMenuArrow()
{
}

void cTaskConsMultiMenuArrow::SetData(uint nType, bool bActive)
{
	CustomKingdomUtil::Assert(0 <= nType && nType < 2, "ArrowType Over");
	nDrawPriority(2147483669u);
	nArrowType() = nType;
	cTaskConsMenuArrow::bActive() = bActive;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetArrowAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	int array[2] = {14, 15};
	nHitPat() = array[nArrowType()];
}

int cTaskConsMultiMenuArrow::nGetArrowAnmNo(int nAnmState)
{
	int obj[2][4] = {{83, 84, 85, 86}, {87, 88, 89, 90}};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	CustomKingdomUtil::Assert(0 <= nArrowType() && nArrowType() < 2, "ArrowType Over");
	return obj[nArrowType()][nAnmState];
}

}
