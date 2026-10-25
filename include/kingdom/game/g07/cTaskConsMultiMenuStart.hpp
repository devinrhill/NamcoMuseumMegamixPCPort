#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsMenuMulti.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuStart : public cTaskConsMenuMulti
{
private:
	enum EAnita
	{
		eANITA_START = 0
	};

public:
	cTaskConsMultiMenuStart(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMultiMenuStart();

	void SetData();

protected:
	int nGetAnmNo(int nAnmState) override;
};

}
