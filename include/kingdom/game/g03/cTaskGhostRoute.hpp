#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g03/sEnemyRouteData.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"

namespace kingdom::game::g03
{

class cTaskGhostRoute : public cTaskKingdom
{
public:
	enum EKind
	{
		eKIND_ROUTE_ROUTE = 0,
		eKIND_ROUTE_CLYD = 1,
		eKIND_ROUTE_PINK = 2,
		eKIND_ROUTE_BLIN = 3,
		eKIND_ROUTE_INKY = 4,
		eKIND_MAX = 5
	};

	enum EKindRide
	{
		eKIND_RIDE_NONE = 0,
		eKIND_RIDE_CAR = 1,
		eKIND_RIDE_UFO = 2,
		eKIND_RIDE_BOAT = 3,
		eKIND_RIDE_PLANE = 4,
		eKIND_RIDE_HOVER = 5
	};

private:
	sEnemyRouteData m_pEnemyRouteData{};

	cTaskCharRouteGhost* m_pTaskCharRouteGhost{};

	bool m_bBottomCheck{};

public:
	cTaskCharRouteGhost* pTaskCharRouteGhost();

	bool IsNeedBottomCheck();

	cTaskGhostRoute(sEnemyRouteData& data);

private:
	void Init();

public:
	void Term();

	bool Execute(cJetTask* pParam) override;

	bool RegistData(sEnemyRouteData& data);

	bool SetOption();

	bool IsRouteGhostAlive();

	void DeadRouteGhost();

	int nRouteNum();

protected:
	void ReleaseTerm() override { cTaskGhostRoute::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
