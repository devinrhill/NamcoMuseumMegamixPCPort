#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuCurtain.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuCurtain : public cTaskConsMenuCurtain
{
public:
	cTaskConsMultiMenuCurtain(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuCurtain();

	void SetData();

protected:
	int nGetCurtainAnmNo(int nAnmState) override;
};

}
