#include "kingdom/game/g01/cTaskConsMenuWorld.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select, world plate console. Ported 1:1 from main.dol (no C# exists for Galaga).
// Every SetAnimNo of this class passes reset = 0. No IsDpdHit override (base: never hit).

namespace kingdom::game::g01
{

// DOL 0x80370610 [audited]: [world 0..2][appear, wait]
const int cTaskConsMenuWorld::ANM_LIST[3][eANM_MAX] = {
	{0x2b, 0x2c},
	{0x2d, 0x2e},
	{0x2f, 0x30},
};

// DOL 0x800bfdc4 [audited]: no draw priority; layout ANT_MENU loaded hidden
cTaskConsMenuWorld::cTaskConsMenuWorld(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
	m_nWorldNo = -1;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU"));
	pAnita(0u)->SetState(1u, false);
}

// DOL 0x800bfe74 [audited]
cTaskConsMenuWorld::~cTaskConsMenuWorld()
{
}

// DOL 0x800bfecc [audited]
void cTaskConsMenuWorld::SetWorldNo(int nWorldNo)
{
	m_nWorldNo = nWorldNo;
}

// DOL 0x800bfed4 [audited] (no range check on +0x1d8)
void cTaskConsMenuWorld::WaitExec()
{
	pAnita(0u)->SetAnimNo(ANM_LIST[m_nWorldNo][eANM_WAIT], false);
}

// DOL 0x800bff58 [audited]
bool cTaskConsMenuWorld::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetState(1u);
		pAnita(0u)->SetAnimNo(ANM_LIST[m_nWorldNo][eANM_APPEAR], false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(0x10000u))
	{
		return true;
	}
	return false;
}

}
