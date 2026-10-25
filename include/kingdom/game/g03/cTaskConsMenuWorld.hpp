#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuWorld : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_WORLD = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

	static const int ANM_LIST[5][3];

	uint m_nWorldNo{};

protected:
	uint& nWorldNo();

public:
	void nSetWorldNo(uint nNo);

	uint nGetWorldNo();

	cTaskConsMenuWorld(cTaskConsMenuManBase* pConsMan);

	virtual ~cTaskConsMenuWorld();

protected:
	void WaitExec() override;

	bool AppearExec() override;

	bool DeleteExec() override;
};

}
