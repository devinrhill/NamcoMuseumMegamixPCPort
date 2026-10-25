#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

class MOTOS_VS_SETTING
{
public:
	sbyte m_nSelColor{};

	sbyte m_nItemNumNo{};

	sbyte m_nRoundNum{};

	sbyte m_nSPLimit{};

	sbyte m_nLimitTimeNo{};

	byte m_nWinNum[4]{};

	byte m_nColor[4]{};

	sbyte m_nSelStage[10]{};

	sbyte m_nSharingMap{};

	byte m_nItemNum{};

	byte m_nRestRound{};

	byte m_nRoundWinNum[4]{};

	byte m_nSPCount[4]{};

	ushort m_nLimitTime{};

	sbyte m_nStage[10]{};
};

}
