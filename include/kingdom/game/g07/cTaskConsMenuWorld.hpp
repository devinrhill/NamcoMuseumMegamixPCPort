#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuWorld : public cTaskConsMenuBase
{
protected:
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

private:
	uint m_nWorldNo{};

protected:
	uint& nWorldNo();

public:
	virtual ~cTaskConsMenuWorld();

	void nSetWorldNo(uint nNo);

	uint nGetWorldNo();

	cTaskConsMenuWorld(cTaskConsMenuManBase* const& pConsMan);

	void SetData(uint nWorld);

protected:
	virtual int nGetWorldAnmNo(int nAnmState);

	void WaitExec() override;

	bool AppearExec() override;

	bool DeleteExec() override;
};

}
