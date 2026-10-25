#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuMap : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_MAP = 0
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

private:
	static const int HIT_MAP_PAT;

	uint m_nWorldNo{};

	uint m_nStageNo{};

protected:
	uint& nWorldNo();

	uint& nStageNo();

public:
	virtual ~cTaskConsMenuMap();

	virtual void ChangeData(uint nWorldNo, uint nStageNo);

	cTaskConsMenuMap(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo);

protected:
	virtual int nGetMapAnm(uint nState);

public:
	bool IsDpdHit(float x, float y) override;

protected:
	bool AppearExec() override;

	void WaitExec() override;

	void PointExec() override;

	bool DecideExec() override;

	bool DeleteExec() override;
};

}
