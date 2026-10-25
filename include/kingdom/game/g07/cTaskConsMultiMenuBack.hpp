#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuBack.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuBack : public cTaskConsMenuBack
{
public:
	cTaskConsMultiMenuBack(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuBack();

protected:
	int nGetBackAnmNo(int nAnmState) override;

public:
	void SetData();
};

}
