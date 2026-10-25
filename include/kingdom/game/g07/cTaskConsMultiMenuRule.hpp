#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuRule.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuRule : public cTaskConsMenuRule
{
public:
	cTaskConsMultiMenuRule(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuRule();

	void SetData();

protected:
	int nGetRuleAnmNo(int nAnmState) override;
};

}
