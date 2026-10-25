#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuWorld.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuWorld : public cTaskConsMenuWorld
{
public:
	cTaskConsMultiMenuWorld(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuWorld();

	void SetData(uint nWorld);

protected:
	int nGetWorldAnmNo(int nAnmState) override;
};

}
