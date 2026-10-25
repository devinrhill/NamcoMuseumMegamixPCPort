#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskModelEx.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cItem.hpp"
#include "kingdom/game/g03/cTaskBgBox.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"

namespace kingdom::game::g03
{

class cTaskItem : public cTaskModelEx
{
private:
	enum EResult
	{
		eRESULT_FAIL = 0,
		eRESULT_HIT = 1,
		eRESULT_NEAR = 2
	};

public:
	enum EKind
	{
		eKIND_COOKIE = 0,
		eKIND_COOKIE_POW = 1,
		eKIND_CHOCO_KNIGHT = 2,
		eKIND_CHOCO_WING = 3,
		eKIND_CHOCO_PACN = 4,
		eKIND_APPLE_L = 5,
		eKIND_APPLE_S = 6,
		eKIND_GRAPE_L = 7,
		eKIND_GRAPE_S = 8,
		eKIND_BANANA_L = 9,
		eKIND_BANANA_S = 10,
		eKIND_HAMBURGER_L = 11,
		eKIND_HAMBURGER_S = 12,
		eKIND_PARFAIT_L = 13,
		eKIND_PARFAIT_S = 14,
		eKIND_MEAT_L = 15,
		eKIND_MEAT_S = 16,
		eKIND_PIZZA = 17,
		eKIND_SFLAG = 18,
		eKIND_MAX = 19
	};

	enum EDFlag
	{
		DFLAG_NORMAL = 0,
		DFLAG_EATEN = 1,
		DFLAG_INBOX = 2,
		DFLAG_HOMING = 4,
		DFLAG_GHOST = 8,
		DFLAG_HIDE = 0x10
	};

private:
	static constexpr float ITEM_RADIUS = 0.4f;

	static constexpr uint ITEM_REVIVAL_CNT = 120u;

	static constexpr uint BOSS_COOKIE_REVIVAL_CNT = 600u;

	uint m_nCookieMax{};

	uint m_nAteCookie{};

	uint m_nHaveCookie{};

	bool m_bEatPizza{};

	GameObject* m_SndYahoo{};

	std::vector<cItem*> m_vecItem;

	std::vector<uint> m_pvecBossItem[3];

	static string s_aBossCookie[3];

public:
	uint GetCookieMax();

	uint GetAteCookie();

	uint& nHaveCookie();

	std::vector<cItem*>& pVecItem();

	bool IsEatPizza();

	cTaskItem();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	string Kind2Key(uint kind);

public:
	uint HitCheck(cItem* item);

	void Homing(cItem* item);

private:
	void EatItem(cItem* item);

public:
	void PushBackItem(cItem* item);

	bool SetBoxItem(Vector4 trans, float rotY, uint treasure, cTaskBgBox* box);

	void SetEnemyItem(uint kind, cTaskCharGhost* ghost, Vector4 trans = Vector4());

	void SetRetryData();

	int BossCookieRevivalTime();

	void InitBossCookie();

	void SetBossCookie(uint nLife);

	void UpdateTrans(cTaskModelEx::cPBModel* model);

	void Visible(cItem* item);

	void Visible(cTaskModelEx::cPBModel* model);

	void Invisible(cItem* item);

	void Invisible(cTaskModelEx::cPBModel* model);
};

}
