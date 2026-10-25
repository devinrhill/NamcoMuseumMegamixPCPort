#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuMap.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuMap : public cTaskConsMenuMap
{
private:
	void ChangeData(uint nWorldNo, uint nStageNo);

public:
	cTaskConsMultiMenuMap(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo);

	virtual ~cTaskConsMultiMenuMap();

	void ChangeData(uint nWorldNo);

protected:
	int nGetMapAnm(uint nState) override;
};

}
