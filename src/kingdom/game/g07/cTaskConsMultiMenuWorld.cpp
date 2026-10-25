#include "kingdom/game/g07/cTaskConsMultiMenuWorld.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuWorld::cTaskConsMultiMenuWorld(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuWorld(pConsMan)
{
}

cTaskConsMultiMenuWorld::~cTaskConsMultiMenuWorld()
{
}

void cTaskConsMultiMenuWorld::SetData(uint nWorld)
{
	nDrawPriority(2147483656u);
	nWorldNo() = nWorld;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetWorldAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsMultiMenuWorld::nGetWorldAnmNo(int nAnmState)
{
	int obj[10][3] = {{36, 37, 38}, {39, 40, 41}, {42, 43, 44}, {45, 46, 47}, {48, 49, 50}, {51, 52, 53}, {54, 55, 56}, {57, 58, 59}, {60, 61, 62}, {63, 64, 65}};
	CustomKingdomUtil::Assert(0 <= nWorldNo() && nWorldNo() < 10, "WorldOver");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "AnmState Over");
	return obj[nWorldNo()][nAnmState];
}

}
