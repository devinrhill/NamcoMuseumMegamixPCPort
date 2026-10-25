#include "kingdom/game/menu/cTaskConsGuest.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsGuest::~cTaskConsGuest()
{
}

cTaskConsGuest::cTaskConsGuest(int nId, string Key)
	: cTaskConsDataBase(nId, Key)
{
}

int cTaskConsGuest::nGetDataBaseAnmNo(uint nState)
{
	int obj[7] = {100, 101, 168, 102, 163, 103, 104};
	CustomKingdomUtil::Assert(0 <= nState && nState < 7, "error");
	return obj[nState];
}

bool cTaskConsGuest::IsDpdHit(float x, float y)
{
	return false;
}

}
