#pragma once

#include "common.hpp"
#include "kingdom/game/g03/AREA_POINT.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"

namespace kingdom::game::g03
{

class cGolvisAIArea
{
public:
	enum EMode
	{
		MODE_NONE = 0,
		MODE_RANDOM = 1,
		MODE_CHASE = 2,
		MODE_ESCAPE = 3
	};

	enum EModeSub
	{
		MODESUB_NONE = 0,
		MODESUB_ROUTE = 1,
		MODESUB_APPROACH = 2,
		MODESUB_LOST = 3
	};

	static constexpr int MAX_SEARCH_AREA = 16;

private:
	int m_nAreaStep{};

	int m_nAreaMax{};

	AREA_POINT* m_aAreas[16]{};

	Vector4 m_vTarget{};

	uint m_nStage{};

	int m_nMode{};

	int m_nModeSub{};

	int m_nRefTime{};

	int m_nRefCounter{};

	int m_nGiveUpTime{};

	int m_nGiveUpCounter{};

	cTaskCharGolvis* m_pGolvis{};

	int m_nMyPrevArea{};

	int m_nMyNextArea{};

	int m_nMyArea{};

	Vector4 m_vPacPos{};

	int m_nPacArea{};

	int m_nPrevPacArea{};

	int m_nEscapeArea{};

public:
	cGolvisAIArea(cTaskCharGolvis* pParent, uint nStage);

	virtual ~cGolvisAIArea();

	bool SearchRoute(int nSrc, int nDst, uint nParam);

	bool SearchRoute(const Vector4& vPos, uint nParam);

	bool IsOnRoute(int nPos);

	virtual void SetKind(int nMode, int nRefTime, int nGiveUpTime = 180);

	virtual void Think(const Vector4& vPos, uint nParam = 0u);

	virtual Vector4 vGoalPos();
};

}
