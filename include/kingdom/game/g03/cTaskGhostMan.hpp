#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"
#include "kingdom/game/g03/cTaskCharGhostBomb.hpp"
#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"
#include "kingdom/game/g03/cTaskGhostArea.hpp"
#include "kingdom/game/g03/cTaskGhostRoute.hpp"

namespace kingdom::game::g03
{

class cTaskGhostMan : public cTaskKingdom
{
private:
	static constexpr int GHOST_ALL_MAX = 32;

	static constexpr int GHOST_AREA_MAX = 8;

	static constexpr int GHOST_ROUTE_MAX = 6;

	static constexpr int GHOST_BOMB_MAX = 3;

	static constexpr int GHOST_EAT_COMBO_MAX = 99;

	uint m_nEatedGhost{};

	uint m_nEatCombo{};

	uint m_nAllGhostNum{};

	bool m_bIjike{};

	bool m_bIjikeOld{};

	uint m_nChangeArea{};

	float m_fGondolaAnim{};

	cTaskGhostArea* m_pGhostArea[8]{};

	cTaskGhostRoute* m_pGhostRoute[6]{};

	cTaskCharGhost* m_pTaskCharGhost[32]{};

	cTaskCharGhostBomb* m_pTaskCharGhostBomb[3]{};

public:
	void IjikeStart();

	void IjikeEnd();

	bool IsIjike();

	uint GetGhostNum();

	void SetGondolaAnim(float frame);

	float GetGondolaAnim();

	cTaskGhostMan();

private:
	void Init();

public:
	void Term();

	bool Execute(cJetTask* pParam) override;

	void SetRetryData();

	uint AllGhostNum();

	uint DeadGhostNum();

	uint LiveGhostNum();

	void ShotBomb(cTaskCharRouteGhost* pParent);

	bool GhostIsActive(uint no);

	bool GhostRevive(uint no);
};

}
