#pragma once

#include "common.hpp"

namespace kingdom::game::g07
{

class MOTOS_VS_PARAM
{
public:
	int m_nEnemyTouch{};

	int m_nMotosTouch{};

	int m_nLPFrame{};

	int m_nLPPoint{};

	float m_fRobPoint{};

	float m_fFallPoint{};

	int m_nPDPoint{};

	float m_fPDDecel{};

	float m_fPanelRebirth{};

	float m_fPlayerMuteki{};

	int m_nSPCharge{};

	int m_nSPPiyoEnemy{};

	int m_nSPPiyoMySelf{};
};

}
