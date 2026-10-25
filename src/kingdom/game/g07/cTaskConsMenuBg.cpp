#include "kingdom/game/g07/cTaskConsMenuBg.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"
#include "kingdom/ResourceDatabase.hpp"

namespace kingdom::game::g07
{

uint& cTaskConsMenuBg::nWorldNo()
{
	return m_nWorldNo;
}

cTaskConsMenuBg::cTaskConsMenuBg(cTaskConsMenuManBase* const& pConsMan, const uint& nWorldNo)
	: cTaskConsMenuBase(pConsMan)
{
	this->nWorldNo() = nWorldNo;
}

cTaskConsMenuBg::~cTaskConsMenuBg()
{
}

uint cTaskConsMenuBg::nGetWorldNo()
{
	return nWorldNo();
}

void cTaskConsMenuBg::nSetWorldNo(uint nNo)
{
	nWorldNo() = nNo;
}

void cTaskConsMenuBg::SetData()
{
	nDrawPriority(65537u);
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANT_MENU_BG"));
	pAnita(0u)->SetAnimNo((int)nWorldNo());
}

int cTaskConsMenuBg::nGetBgAnmNo()
{
	CustomKingdomUtil::Assert(nWorldNo() < 5, "error");
	static const int table[5] = {0, 1, 2, 3, 4};
	return table[nWorldNo()];
}

bool cTaskConsMenuBg::AppearExec()
{
	pAnita(0u)->SetAnimNo(nGetBgAnmNo(), false);
	return true;
}

}
