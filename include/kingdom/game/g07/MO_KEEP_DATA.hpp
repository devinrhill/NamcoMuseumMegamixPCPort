#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

class MO_KEEP_DATA
{
public:
	uint m_nMyScore{};

	uint m_nHiScore{};

	byte m_nItemNum[4]{};

	byte m_nUseItem[4]{};

	short m_nLife{};

	short m_nExtendCount{};

	int m_nEnemyTaikiBits{};

	sbyte m_nNowVsRound{};

	short m_nBossHP{};

	short m_nBossHPLast{};
};

}
