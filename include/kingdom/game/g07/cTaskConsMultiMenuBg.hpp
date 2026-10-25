#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuBg.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuBg : public cTaskConsMenuBg
{
public:
	cTaskConsMultiMenuBg(cTaskConsMenuManBase* const& pConsMan, const uint& nWorldNo);

	virtual ~cTaskConsMultiMenuBg();

	void SetData();

protected:
	int nGetBgAnmNo() override;
};

}
