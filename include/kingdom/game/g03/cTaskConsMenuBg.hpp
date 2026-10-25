#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuBg : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_BG = 0,
		eANITA_MAX = 1
	};

	uint m_nWorldNo{};

protected:
	uint& nWorldNo();

public:
	uint nGetWorldNo();

	void nSetWorldNo(uint nNo);

	cTaskConsMenuBg(cTaskConsMenuManBase* const& pConsMan, const uint& nWorldNo);

	virtual ~cTaskConsMenuBg();

protected:
	bool AppearExec() override;
};

}
