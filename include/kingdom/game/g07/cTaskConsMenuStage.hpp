#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuStage : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_STAGE = 0,
		eANITA_MOTOS_SCORE = 1,
		eANITA_MOTOS_TIME = 2,
		eANITA_MAX = 3
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

	static const int ANM_MOTOS_LIST[5];

	static const int HIT_PAT;

	static const int SET_STAGE_PAT[7];

	static const int SET_MOTOS_PAT;

	static const uint SET_MOTOS_LAY[2];

private:
	bool m_bActive{};

	uint m_nWorldNo{};

	uint m_nStageNo{};

	int m_nHitPat{};

	int ANM_LIST[2][5][7][5]{};

protected:
	bool& bActive();

	uint& nWorldNo();

	uint& nStageNo();

	int& nHitPat();

public:
	bool IsActive();

	cTaskConsMenuStage(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo, uint nStageNo);

	virtual ~cTaskConsMenuStage();

protected:
	virtual int nGetStageAnm(uint nState);

	void SetAnmMotos(uint nType);

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
