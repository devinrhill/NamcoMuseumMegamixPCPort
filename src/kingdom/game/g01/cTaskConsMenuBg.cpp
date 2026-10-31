#include "kingdom/game/g01/cTaskConsMenuBg.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

// Galaga stage select background. Only WaitExec is overridden; Appear / Point / Decide / Delete are the base ones.

namespace kingdom::game::g01
{

// DOL 0x80370120 [audited]
const int cTaskConsMenuBg::s_AnmTbl[3] = {0, 1, 2};

// DOL 0x800bbf38 [audited]: starts in the wait state (+0x1d0 = 1), layout crc 0x9b7d968e = ANT_BG on anita 0 (not hidden),
// SetAnimNo(nGetBgAnmNo(0), reset = 1, frame 0.0 r2-0x6ae0)
cTaskConsMenuBg::cTaskConsMenuBg(cTaskConsMenuManBase* const& pConsMan, int nWorldNo)
	: cTaskConsMenuBase(pConsMan)
{
	nState() = 1;
	m_nWorldNo = nWorldNo;
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_BG"));
	// virtual call through slot +0x94 inside the DOL ctor: the vtable is already cTaskConsMenuBg's, as it is here
	pAnita(0u)->SetAnimNo(nGetBgAnmNo(0), true);
}

// DOL 0x800bc010 [audited]
cTaskConsMenuBg::~cTaskConsMenuBg()
{
}

// DOL 0x800bc068 [audited]: the DOL copies the 3-word table to the stack and reads word [world + nAnmState]; only nAnmState 0 is ever passed
// here, so world 0..2 stays inside the table. UNVERIFIED for other indices (the DOL would read past the copy).
int cTaskConsMenuBg::nGetBgAnmNo(int nAnmState)
{
	return s_AnmTbl[m_nWorldNo + nAnmState];
}

// DOL 0x800bc0a8 [audited]
void cTaskConsMenuBg::WaitExec()
{
	pAnita(0u)->SetAnimNo(nGetBgAnmNo(0), false);
}

}
