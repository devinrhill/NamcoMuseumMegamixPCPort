#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuStage : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_STAGE = 0
	};

public:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_DELETE = 4,
		eANM_MAX = 5
	};

	enum EStage
	{
		eSTAGE_01 = 0,
		eSTAGE_02 = 1,
		eSTAGE_03 = 2,
		eSTAGE_04 = 3,
		eSTAGE_05 = 4,
		eSTAGE_06 = 5,
		eSTAGE_07 = 6,
		eSTAGE_MAX = 7
	};

private:
	static const int ANM_CROWN_LIST[5];

	static const int HIT_PAT;

	static const int SET_STAGE_PAT[6];

	static const uint ANM_BOSS_INDEX;

	bool m_bValid{};

	bool m_bActive{};

	uint m_nWorldNo{};

	uint m_nStageNo{};

	int m_nHitPat{};

	int ANM_LIST[2][5][6][5]{};

protected:
	bool& bValid();

	bool& bActive();

	uint& nWorldNo();

	uint& nStageNo();

	int& nHitPat();

public:
	bool IsActive();

	bool IsValid();

	cTaskConsMenuStage(cTaskConsMenuManBase* pConsMan, uint nWorldNo, uint nStageNo);

	virtual ~cTaskConsMenuStage();

protected:
	virtual int nGetStageAnm(uint nState);

public:
	bool IsDpdHit(float x, float y) override;

protected:
	void WaitExec() override;

	void PointExec() override;

	bool AppearExec() override;

	bool DecideExec() override;

	bool DeleteExec() override;

public:
	void LoadStageData(uint nWorldNo);
};

}
