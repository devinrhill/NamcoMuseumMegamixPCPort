#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuItem : public cTaskConsMenuBase
{
protected:
	enum EAnita
	{
		eANITA_NUMBER_02 = 0,
		eANITA_NUMBER_01 = 1,
		eANITA_MAX = 2
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

private:
	static const int SET_ONE_PAT;

	static const int SET_TWO_PAT;

	static const uint SET_TWO_LAY[2];

	uint m_nItem{};

protected:
	uint& nItem();

public:
	virtual ~cTaskConsMultiMenuItem();

	void nSetItem(uint nNo);

	uint nGetItem();

	cTaskConsMultiMenuItem(cTaskConsMenuManBase* const& pConsMan);

protected:
	virtual int nGetNumberAnmNo(uint nNo, int nAnmState);

public:
	void SetData(uint nItem);

protected:
	virtual void SetItemAnm(int nAnmState);

	void WaitExec() override;

	bool AppearExec() override;

	bool DeleteExec() override;
};

}
