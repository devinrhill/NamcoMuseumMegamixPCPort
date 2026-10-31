#include "kingdom/game/g07/cTaskConsMenuCurtain.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

cTaskConsMenuCurtain::cTaskConsMenuCurtain(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

// DOL 0x801c7060 [audited: anim ids, tables and step logic checked vs decomp]
void cTaskConsMenuCurtain::SetData()
{
	nDrawPriority(2147483649u);
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetCurtainAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x801c713c [audited: anim ids, tables and step logic checked vs decomp]
int cTaskConsMenuCurtain::nGetCurtainAnmNo(int nAnmState)
{
	int obj[2] = {342, 343};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 2, "Array Over");
	return obj[nAnmState];
}

cTaskConsMenuCurtain::~cTaskConsMenuCurtain()
{
}

// DOL 0x801c71bc [audited: anim ids, tables and step logic checked vs decomp]
void cTaskConsMenuCurtain::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetCurtainAnmNo(1), false);
}

// DOL 0x801c7220 [audited: anim ids, tables and step logic checked vs decomp]
bool cTaskConsMenuCurtain::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetCurtainAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
