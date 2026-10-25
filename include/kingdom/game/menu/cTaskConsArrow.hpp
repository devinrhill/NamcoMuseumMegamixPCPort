#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"

namespace kingdom::game::menu
{

class cTaskConsArrow : public cTaskConsMatching
{
private:
	enum EAnt
	{
		eANT_ARROW = 0
	};

protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_DELETE = 4,
		eANM_MAX = 5
	};

public:
	enum EType
	{
		eTYPE_LEFT = 0,
		eTYPE_RIGHT = 1,
		eTYPE_MAX = 2
	};

private:
	int HIT_PAT[2] = {15, 16};

	uint m_nType{};

	int m_nHitPat{};

protected:
	uint& nType();

	int& nHitPat();

public:
	virtual ~cTaskConsArrow();

	cTaskConsArrow(int nId, string Key, uint nType);

protected:
	virtual int nGetArrowAnmNo(uint nState);

public:
	bool IsDpdHit(float x, float y) override;

protected:
	bool AppearProc() override;

	void WaitProc() override;

	void PointProc() override;

	bool DecideProc() override;

	bool DeleteProc() override;

public:
	void Visible(bool b);

	bool IsVisible();
};

}
