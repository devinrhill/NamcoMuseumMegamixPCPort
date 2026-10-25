#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsDataBase.hpp"

namespace kingdom::game::menu
{

class cTaskConsRemote : public cTaskConsDataBase
{
public:
	virtual ~cTaskConsRemote();

	cTaskConsRemote(int nId, string Key);

protected:
	int nGetDataBaseAnmNo(uint nState) override;

public:
	bool IsDpdHit(float x, float y) override;
};

}
