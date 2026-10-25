#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/cAppGameBaseMenu.hpp"

namespace kingdom::game
{

class cTaskConsMenuManBase : public cTaskKingdom
{
private:
	cAppGameBaseMenu* m_pApp{};

protected:
	cAppGameBaseMenu*& pApp();

public:
	cTaskConsMenuManBase(cAppGameBaseMenu* pApp);

	virtual ~cTaskConsMenuManBase();

	virtual void Uplode();

	virtual void PlaySndSe(const string& pSndName, int ctrl = 0);
};

}
