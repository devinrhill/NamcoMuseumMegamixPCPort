#include "kingdom/game/cTaskConsMenuManBase.hpp"

#include "kingdom/game/cAppGameBaseMenu.hpp"

namespace kingdom::game
{

cAppGameBaseMenu*& cTaskConsMenuManBase::pApp()
{
	return m_pApp;
}

cTaskConsMenuManBase::cTaskConsMenuManBase(cAppGameBaseMenu* pApp)
{
	this->pApp() = pApp;
}

cTaskConsMenuManBase::~cTaskConsMenuManBase()
{
}

void cTaskConsMenuManBase::Uplode()
{
}

void cTaskConsMenuManBase::PlaySndSe(const string& pSndName, int ctrl)
{
	pApp()->PlaySndSe(pSndName, ctrl);
}

}
