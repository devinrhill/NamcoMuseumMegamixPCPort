#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuMulti.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuSingle : public cTaskConsMenuMulti
{
private:
	enum EAnita
	{
		eANITA_SINGLE = 0
	};

public:
	cTaskConsMultiMenuSingle(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuSingle();

	void SetData();

protected:
	int nGetAnmNo(int nAnmState) override;
};

}
