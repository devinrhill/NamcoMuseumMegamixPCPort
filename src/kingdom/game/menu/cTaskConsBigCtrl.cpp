#include "kingdom/game/menu/cTaskConsBigCtrl.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

cTaskConsBigCtrl::cTaskConsBigCtrl(int nId, string Key, uint nPlayerNo)
	: cTaskConsCtrl(nId, Key, nPlayerNo)
{
}

cTaskConsBigCtrl::~cTaskConsBigCtrl()
{
}

int cTaskConsBigCtrl::nGetCtrlAnmNo(uint nState)
{
	int obj[4][3] = {{66, 67, 151}, {72, 73, 152}, {78, 79, 153}, {84, 85, 154}};
	CustomKingdomUtil::Assert(0 <= nState && nState < 3, "error");
	int num = obj[nPlayerNo()][nState];
	if (nState == 1 && bActive())
	{
		num += (int)(1 + nCtrlNo());
	}
	return num;
}

}
