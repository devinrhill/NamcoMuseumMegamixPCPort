#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g03/sEnemyAreaData.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskCharAreaGhost.hpp"

namespace kingdom::game::g03
{

class cTaskGhostArea : public cTaskKingdom
{
public:
	enum EKind
	{
		eKIND_AREA_TRIANGLE = 0,
		eKIND_AREA_SQUARE = 1,
		eKIND_AREA_MOVE = 2,
		eKIND_AREA_CLYD = 3,
		eKIND_AREA_PINK = 4,
		eKIND_AREA_BLIN = 5,
		eKIND_AREA_INKY = 6,
		eKIND_MAX = 7
	};

	static constexpr int SEARCH_TIMING = 8;

	static constexpr int AREA_GHOST_MAX = 8;

	static constexpr int AREA_MAX = 48;

	static constexpr int POINT_MAX = 64;

private:
	sEnemyAreaData m_pEnemyAreaData{};

	cTaskCharAreaGhost* m_pTaskCharGhost[8]{};

	uint m_nAreaPointFlag[48]{};

	ulong m_nPointToPointFlag[64]{};

	ulong m_nAreaToAreaFlag[48]{};

	uint m_nGhostNum{};

	bool m_bAreaFusion{};

	float m_fAreaInside[3][2]{};

	ulong m_nPlayerAreaFlag{};

	uint m_nSearchCount{};

	bool m_bBottomCheck{};

public:
	bool IsAreaInPacMan();

	cTaskCharAreaGhost* pTaskCharAreaGhost(uint id);

	bool IsNeedBottomCheck();

	uint GhostNum();

	bool IsAreaFusion();

	cTaskGhostArea(sEnemyAreaData& data);

private:
	void Init();

public:
	void Term();

	bool Execute(cJetTask* pParam) override;

	bool RegistData(sEnemyAreaData& data);

	bool ChangeAreaData(sEnemyAreaData& data);

	bool SetOption();

	void ReSetGhost();

	bool IsAreaGhostDeadOrAlive(uint id);

	void DeadAreaGhost(uint id);

	bool IsIntoAreaPos(Vector4 pos);

private:
	bool IsIntoArea(Vector4 pos, int area_id);

	int PointLineSide(Vector2 pos, Vector2 seg1, Vector2 seg2);

	int SearchRoute(int result, int& next_area_id, int area_id, ulong target_id_flag, ulong through = 0uL, int count = 0);
};

}
