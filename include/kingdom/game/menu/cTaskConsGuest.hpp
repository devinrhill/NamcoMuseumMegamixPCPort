#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsDataBase.hpp"

namespace kingdom::game::menu
{

class cTaskConsGuest : public cTaskConsDataBase
{
public:
	virtual ~cTaskConsGuest();

	cTaskConsGuest(int nId, string Key);

protected:
	int nGetDataBaseAnmNo(uint nState) override;

public:
	bool IsDpdHit(float x, float y) override;
};

}
