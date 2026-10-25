#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMultiMenuTime : public cTaskConsMenuBase
{
protected:
	enum EAnita
	{
		eANITA_NUMBER_03 = 0,
		eANITA_NUMBER_02 = 1,
		eANITA_NUMBER_01 = 2,
		eANITA_CONMA = 3,
		eANITA_MAX = 4
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

private:
	static const int SET_TWO_PAT;

	static const uint SET_TWO_LAY[3];

	static const int SET_CONMA_PAT;

	static const uint SET_CONMA_LAY;

	uint m_nTime{};

protected:
	uint& nTime();

public:
	virtual ~cTaskConsMultiMenuTime();

	void nSetTime(uint nNo);

	uint nGetTime();

	cTaskConsMultiMenuTime(cTaskConsMenuManBase* const& pConsMan);

protected:
	virtual int nGetNumberAnmNo(uint nNo, int nAnmState);

	virtual int nGetConmaAnmNo(int nAnmState);

public:
	void SetData(uint nTime);

protected:
	virtual void SetTimeAnm(int nAnmState);

	void WaitExec() override;

	bool AppearExec() override;

	bool DeleteExec() override;
};

}
