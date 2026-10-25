#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

class cTaskConsMenuBar : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_BAR = 0,
		eANITA_MOTOS_SCORE = 1,
		eANITA_MOTOS_TIME = 2,
		eANITA_SCORE_06 = 3,
		eANITA_SCORE_05 = 4,
		eANITA_SCORE_04 = 5,
		eANITA_SCORE_03 = 6,
		eANITA_SCORE_02 = 7,
		eANITA_SCORE_01 = 8,
		eANITA_TIME_06 = 9,
		eANITA_TIME_05 = 10,
		eANITA_TIME_04 = 11,
		eANITA_TIME_03 = 12,
		eANITA_TIME_02 = 13,
		eANITA_TIME_01 = 14,
		eANITA_TIME_CONMA_01 = 15,
		eANITA_TIME_CONMA_02 = 16,
		eANITA_MAX = 17
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2
	};

	static const int ANM_BAR_LIST[3];

	static const int ANM_MOTOS_LIST[3];

	static const int ANM_NUMBER_LIST[10][3];

	static const int ANM_CONMA_LIST[3];

	static const int SET_MOTOS_PAT;

	static const uint SET_MOTOS_LAY[2];

	static const int SET_SCORE_PAT;

	static const uint SET_SCORE_LAY[6];

	static const int SET_TIME_PAT;

	static const uint SET_TIME_LAY[6];

	static const int SET_CONMA_PAT;

	static const uint SET_CONMA_LAY[2];

	uint m_nWorldNo{};

	uint m_nStageNo{};

	uint m_nScore{};

	uint m_nTime{};

	bool m_bScoreNew{};

	bool m_bTimeNew{};

protected:
	uint& nWorldNo();

	uint& nStageNo();

	uint& nScore();

	uint& nTime();

	bool& bScoreNew();

	bool& bTimeNew();

public:
	cTaskConsMenuBar(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMenuBar();

protected:
	void SetAnmScore(int nType);

	void SetAnmTime(int nType);

	void WaitExec() override;

	bool AppearExec() override;

	bool DeleteExec() override;

public:
	void LoadRecord(uint nWorld, uint nStage);
};

}
