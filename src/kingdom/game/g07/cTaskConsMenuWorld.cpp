#include "kingdom/game/g07/cTaskConsMenuWorld.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

uint& cTaskConsMenuWorld::nWorldNo()
{
	return m_nWorldNo;
}

cTaskConsMenuWorld::~cTaskConsMenuWorld()
{
}

void cTaskConsMenuWorld::nSetWorldNo(uint nNo)
{
	nWorldNo() = nNo;
}

uint cTaskConsMenuWorld::nGetWorldNo()
{
	return nWorldNo();
}

// DOL 0x801ca818 [audited: tables, priorities and step logic checked vs decomp (Unity UI branches are native-only)]
cTaskConsMenuWorld::cTaskConsMenuWorld(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

// DOL 0x801ca85c [audited: tables, priorities and step logic checked vs decomp (Unity UI branches are native-only)]
void cTaskConsMenuWorld::SetData(uint nWorld)
{
	nDrawPriority(2147483651u);
	nWorldNo() = nWorld;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetWorldAnmNo(1));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x801ca940 [audited: tables, priorities and step logic checked vs decomp (Unity UI branches are native-only)]
int cTaskConsMenuWorld::nGetWorldAnmNo(int nAnmState)
{
	int obj[5][3] = {{36, 37, 38}, {39, 40, 41}, {42, 43, 44}, {45, 46, 47}, {48, 49, 50}};
	CustomKingdomUtil::Assert(0 <= nWorldNo() && nWorldNo() < 5, "WorldOver");
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 3, "AnmState Over");
	return obj[nWorldNo()][nAnmState];
}

// DOL 0x801ca9e8 [audited: tables, priorities and step logic checked vs decomp (Unity UI branches are native-only)]
void cTaskConsMenuWorld::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetWorldAnmNo(1), false);
}

// DOL 0x801caa4c [audited: tables, priorities and step logic checked vs decomp (Unity UI branches are native-only)]
bool cTaskConsMenuWorld::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWorldAnmNo(0), false);
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

// DOL 0x801cab10 [audited: tables, priorities and step logic checked vs decomp (Unity UI branches are native-only)]
bool cTaskConsMenuWorld::DeleteExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetWorldAnmNo(2), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
