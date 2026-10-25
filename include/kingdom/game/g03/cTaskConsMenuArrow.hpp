#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuArrow : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_ARROW = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_MAX = 4
	};

public:
	enum EArrow
	{
		eARROW_LEFT = 0,
		eARROW_RIGHT = 1,
		eARROW_MAX = 2
	};

private:
	static const int ANM_LIST[2][4];

	static const int HIT_PAT[2];

	bool m_bActive{};

	uint m_nArrowType{};

	int m_nHitPat{};

protected:
	bool& bActive();

	uint& nArrowType();

	int& nHitPat();

public:
	bool IsActive();

	cTaskConsMenuArrow(cTaskConsMenuManBase* pConsMan, uint nType, bool bActive);

	virtual ~cTaskConsMenuArrow();

	bool IsDpdHit(float x, float y) override;

protected:
	void WaitExec() override;

	void PointExec() override;

	bool AppearExec() override;

	bool DecideExec() override;
};

}
