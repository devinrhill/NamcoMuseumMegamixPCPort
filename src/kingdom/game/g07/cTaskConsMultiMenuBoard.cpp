#include "kingdom/game/g07/cTaskConsMultiMenuBoard.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMultiMenuBoard::cTaskConsMultiMenuBoard(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

cTaskConsMultiMenuBoard::~cTaskConsMultiMenuBoard()
{
}

void cTaskConsMultiMenuBoard::SetData()
{
	nDrawPriority(2147483650u);
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_02"));
	pAnita(0u)->SetAnimNo(nGetBoardAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

int cTaskConsMultiMenuBoard::nGetBoardAnmNo(int nAnmState)
{
	int obj[2] = {77, 78};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 2, "State Over");
	return obj[nAnmState];
}

bool cTaskConsMultiMenuBoard::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetBoardAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsMultiMenuBoard::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetBoardAnmNo(1), false);
}

}
