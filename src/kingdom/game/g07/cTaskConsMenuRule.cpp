#include "kingdom/game/g07/cTaskConsMenuRule.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/game/g07/cTaskConsMenuMan.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

int& cTaskConsMenuRule::nHitPat()
{
	return m_nHitPat;
}

cTaskConsMenuRule::cTaskConsMenuRule(cTaskConsMenuManBase* const& pConsMan)
	: cTaskConsMenuBase(pConsMan)
{
}

cTaskConsMenuRule::~cTaskConsMenuRule()
{
}

// DOL 0x801c9814 (priority 0x80000006, hit pattern 0x11, layout crc 0x62428a7d = ANT_MENU_SELECT_01)
void cTaskConsMenuRule::SetData()
{
	nDrawPriority(2147483654u);
	nHitPat() = 17;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_SELECT_01"));
	pAnita(0u)->SetAnimNo(nGetRuleAnmNo(1));
	pAnita(0u)->SetState(1u, false);
	UnityUI = (static_cast<cTaskConsMenuMan*>(pTaskConsMenuMan()))->UnityUI;
}

// DOL 0x801c98f8 (table 0x8037ec30)
int cTaskConsMenuRule::nGetRuleAnmNo(int nAnmState)
{
	int obj[4] = {319, 320, 321, 322};
	CustomKingdomUtil::Assert(0 <= nAnmState && nAnmState < 4, "State Over");
	return obj[nAnmState];
}

// DOL 0x801c9934: hit test of anita 0, pattern +0x1d8 (nHitPat) (the C# returned false: Unity used colliders)
bool cTaskConsMenuRule::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, nHitPat());
}

// DOL 0x801c9a78 (the C# drove a Unity button; the DOL plays the Anita anims like Back)
void cTaskConsMenuRule::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetRuleAnmNo(1), false);
}

// DOL 0x801c9adc
bool cTaskConsMenuRule::AppearExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetRuleAnmNo(0));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		pAnita(0u)->SetAnimNo(nGetRuleAnmNo(1));
		return true;
	}
	return false;
}

// DOL 0x801c9bd8
void cTaskConsMenuRule::PointExec()
{
	pAnita(0u)->SetAnimNo(nGetRuleAnmNo(2), false);
}

// DOL 0x801c9c3c
bool cTaskConsMenuRule::DecideExec()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetRuleAnmNo(3));
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
