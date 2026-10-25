#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharItem.hpp"

namespace kingdom::game::g07
{

class cTaskCharItemJump : public cTaskCharItem
{
public:
	cTaskCharItemJump();

	static cTaskCharItem* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;
};

}
