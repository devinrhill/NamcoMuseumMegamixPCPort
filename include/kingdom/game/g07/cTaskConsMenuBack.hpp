#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuBack : public cTaskConsMenuBase
{
protected:
	enum EAnita
	{
		eANITA_MULTI = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_MAX = 4
	};

private:
	int m_nHitPat{};

protected:
	int& nHitPat();

public:
	cTaskConsMenuBack(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMenuBack();

protected:
	virtual int nGetBackAnmNo(int nAnmState);

public:
	void SetData();

	bool IsDpdHit(float x, float y) override;

protected:
	void WaitExec() override;

	bool AppearExec() override;

	void PointExec() override;

	bool DecideExec() override;
};

}
