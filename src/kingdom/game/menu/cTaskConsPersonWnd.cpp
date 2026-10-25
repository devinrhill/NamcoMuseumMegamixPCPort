#include "kingdom/game/menu/cTaskConsPersonWnd.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsPersonWnd::~cTaskConsPersonWnd()
{
}

cTaskConsPersonWnd::cTaskConsPersonWnd(int nId, string Key)
	: cTaskConsWndBase(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsPersonWnd::nGetWndAnmNo(uint nState)
{
	int obj[3] = {63, 64, 65};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	return obj[nState];
}

}
