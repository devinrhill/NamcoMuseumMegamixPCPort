#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharItem.hpp"

namespace kingdom::game::g07
{

class cTaskCharItemCharge : public cTaskCharItem
{
public:
	cTaskCharItemCharge();

	static cTaskCharItem* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;
};

}
