#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuArrow.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuMiniArrow : public cTaskConsMenuArrow
{
public:
	enum EMiniArrow
	{
		eMINI_ARROW_ROUND_LEFT = 0,
		eMINI_ARROW_ROUND_RIGHT = 1,
		eMINI_ARROW_TIME_LEFT = 2,
		eMINI_ARROW_TIME_RIGHT = 3,
		eMINI_ARROW_ITEM_LEFT = 4,
		eMINI_ARROW_ITEM_RIGHT = 5,
		eMINI_ARROW_MAX = 6
	};

	cTaskConsMultiMenuMiniArrow(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuMiniArrow();

	void SetData(uint nType);

protected:
	int nGetArrowAnmNo(int nAnmState) override;
};

}
