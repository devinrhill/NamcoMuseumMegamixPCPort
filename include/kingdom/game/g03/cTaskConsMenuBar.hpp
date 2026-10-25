#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuBar : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_BAR = 0,
		eANITA_CROWN_COOKIE = 1,
		eANITA_CROWN_TIME = 2,
		eANITA_CROWN_GHOST = 3,
		eANITA_COOKIE_03 = 4,
		eANITA_COOKIE_02 = 5,
		eANITA_COOKIE_01 = 6,
		eANITA_COOKIE_MAX_03 = 7,
		eANITA_COOKIE_MAX_02 = 8,
		eANITA_COOKIE_MAX_01 = 9,
		eANITA_SLASH = 10,
		eANITA_TIME_05 = 11,
		eANITA_TIME_04 = 12,
		eANITA_TIME_03 = 13,
		eANITA_TIME_02 = 14,
		eANITA_TIME_01 = 15,
		eANITA_GHOST_02 = 16,
		eANITA_GHOST_01 = 17,
		eANITA_TIME_CONMA_01 = 18,
		eANITA_TIME_CONMA_02 = 19,
		eANITA_MAX = 20
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2
	};

	static const int ANM_BAR_LIST[3];

	static const int ANM_CROWN_LIST[3];

	static const int ANM_NUMBER_LIST[10][3];

	static const int ANM_SLASH_LIST[3];

	static const int ANM_CONMA_LIST[3];

	static const int SET_CROWN_PAT;

	static const uint SET_CROWN_LAY[3];

	static const int SET_GHOST_PAT;

	static const uint SET_GHOST_LAY[2];

	static const int SET_COOKIE_PAT;

	static const uint SET_COOKIE_LAY[3];

	static const int SET_COOKIE_MAX_PAT;

	static const uint SET_COOKIE_MAX_LAY[3];

	static const int SET_TIME_PAT;

	static const uint SET_TIME_LAY[5];

	static const int SET_SLASH_PAT;

	static const uint SET_SLASH_LAY;

	static const int SET_CONMA_PAT;

	static const uint SET_CONMA_LAY[2];

	uint m_nCashWorld{};

	uint m_nCashStage{};

	uint m_nCookie{};

	uint m_nCookieMax{};

	uint m_nGhost{};

	uint m_nTime{};

	bool m_bCookieNew{};

	bool m_bGhostNew{};

	bool m_bTimeNew{};

	bool m_bUpdate{};

protected:
	uint& nCashWorld();

	uint& nCashStage();

	uint& nCookie();

	uint& nCookieMax();

	uint& nGhost();

	uint& nTime();

	bool& bCookieNew();

	bool& bGhostNew();

	bool& bTimeNew();

	bool& bUpdate();

public:
	cTaskConsMenuBar(cTaskConsMenuManBase* pConsMan);

	virtual ~cTaskConsMenuBar();

protected:
	void SetAnmCookie(int nType);

	void SetAnmGhost(int nType);

	void SetAnmTime(int nType);

	void WaitExec() override;

	bool AppearExec() override;

	bool DeleteExec() override;

public:
	void LoadRecord(uint nWorldNo, uint nStageNo);
};

}
