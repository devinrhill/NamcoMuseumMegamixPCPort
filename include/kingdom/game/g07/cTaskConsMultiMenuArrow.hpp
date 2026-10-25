#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuArrow.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuArrow : public cTaskConsMenuArrow
{
public:
	cTaskConsMultiMenuArrow(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuArrow();

	void SetData(uint nType, bool bActive);

protected:
	int nGetArrowAnmNo(int nAnmState) override;
};

}
