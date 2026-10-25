#include "kingdom/game/g03/cTaskConsMenuBg.hpp"

#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

uint& cTaskConsMenuBg::nWorldNo()
{
	return m_nWorldNo;
}

uint cTaskConsMenuBg::nGetWorldNo()
{
	return nWorldNo();
}

void cTaskConsMenuBg::nSetWorldNo(uint nNo)
{
	nWorldNo() = nNo;
}

cTaskConsMenuBg::cTaskConsMenuBg(cTaskConsMenuManBase* const& pConsMan, const uint& nWorldNo)
	: cTaskConsMenuBase(pConsMan)
{
	nDrawPriority(65537u);
	this->nWorldNo() = nWorldNo;
	string fileName = "BIN_ANT_MENU_BG";
	pAnita(0u)->RegistData(fileName);
	pAnita(0u)->SetAnimNo((int)this->nWorldNo());
}

cTaskConsMenuBg::~cTaskConsMenuBg()
{
}

bool cTaskConsMenuBg::AppearExec()
{
	pAnita(0u)->SetAnimNo((int)nWorldNo(), false);
	return true;
}

}
