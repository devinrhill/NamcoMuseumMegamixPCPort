#include "kingdom/game/menu/cTaskConsAddWnd.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsAddWnd::~cTaskConsAddWnd()
{
}

cTaskConsAddWnd::cTaskConsAddWnd(int nId, string Key)
	: cTaskConsWndBase(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsAddWnd::nGetWndAnmNo(uint nState)
{
	int obj[3] = {144, 145, 146};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	return obj[nState];
}

}
