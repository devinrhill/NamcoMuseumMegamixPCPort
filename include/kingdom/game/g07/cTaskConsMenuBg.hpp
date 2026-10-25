#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuBg : public cTaskConsMenuBase
{
protected:
	enum EAnita
	{
		eANITA_BG = 0,
		eANITA_MAX = 1
	};

private:
	uint m_nWorldNo{};

protected:
	uint& nWorldNo();

public:
	cTaskConsMenuBg(cTaskConsMenuManBase* const& pConsMan, const uint& nWorldNo);

	virtual ~cTaskConsMenuBg();

	uint nGetWorldNo();

	void nSetWorldNo(uint nNo);

	void SetData();

protected:
	virtual int nGetBgAnmNo();

	bool AppearExec() override;
};

}
