#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskCharEnemy.hpp"

namespace kingdom::game::g03
{

class cTaskCharGhost : public cTaskCharEnemy
{
public:
	enum EHaveItem
	{
		eHAVE_ITEM_NONE = 0,
		eHAVE_ITEM_P_COOKIE = 1,
		eHAVE_ITEM_CHOCO_N = 2,
		eHAVE_ITEM_CHOCO_K = 3,
		eHAVE_ITEM_CHOCO_W = 4,
		eHAVE_ITEM_MAX = 5
	};

	enum EAnimeState
	{
		ANIME_STATE_NORMAL = 0,
		ANIME_STATE_IZIKE_0 = 1,
		ANIME_STATE_IZIKE_1 = 2,
		ANIME_STATE_MAX = 3
	};

protected:
	static inline const string NODE_ITEM = "item_root";

	static inline const string NODE_FOOT = "asi";

	static inline const string NODE_MAN = "man_root";

	static constexpr float EATED_SPEED = 0.02f;

	static constexpr float BOMB_ATTACK_AREA = 2.0f;

	static constexpr float EATED_FRAME = 25.0f;

	static constexpr int IZIKE_FLASH_CHANGE_TIME = 15;

	static constexpr int IZIKE_FLASH_TIME = 30;

	static const Vector4 CHOCO_POS[2];

private:
	int m_nStatus{};

	int m_nOptionItem{};

	bool m_bRevival{};

	Vector4 m_vStartPos{};

protected:
	bool m_bSndEngineStop{};

public:
	void Revive();

	int& nStat();

	int& nOptionItem();

	bool& bRevival();

	Vector4& vStartPos();

	virtual bool IsDead();

	cTaskCharGhost();

private:
	void Init();

public:
	void Term() override;

protected:
	void ReleaseTerm() override { cTaskCharGhost::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
