#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

class MO_CHAR_PARAMS
{
public:
	float m_fWeight{};

	uint m_nHereticVal{};

	float m_fMaxSpeed{};

	float m_fAccelSpeed{};

	float m_fDecelSpeed{};

	float m_fHitPower{};

	float m_fChargePower{};

	float m_fSlopePower{};

	float m_fWallReflect{};

	float m_fHitSize{};

	float m_fWHitSize{};

	int m_nPoints{};

	int m_nVSPoints{};

	float m_fExParam[3]{};

	MO_CHAR_PARAMS* Clone();
};

}
