#include "kingdom/game/menu/cTaskConsRemote.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsRemote::~cTaskConsRemote()
{
}

cTaskConsRemote::cTaskConsRemote(int nId, string Key)
	: cTaskConsDataBase(nId, Key)
{
}

int cTaskConsRemote::nGetDataBaseAnmNo(uint nState)
{
	int obj[7] = {105, 106, 169, 107, 164, 108, 109};
	CustomKingdomUtil::Assert(0 <= nState && nState < 7, "error");
	return obj[nState];
}

bool cTaskConsRemote::IsDpdHit(float x, float y)
{
	return false;
}

}
