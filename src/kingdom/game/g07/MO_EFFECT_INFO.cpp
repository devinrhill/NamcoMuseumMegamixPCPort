#include "kingdom/game/g07/MO_EFFECT_INFO.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g07
{

MO_EFFECT_INFO::MO_EFFECT_INFO()
{
	m_nFlag = 0;
	m_nID = 0;
	m_nKind = 0;
	m_nTimer = 0;
	m_nDelay = 0;
	m_phPtk = nullptr;
	m_bDie = true;
	m_Func = nullptr;
	std::fill(std::begin(m_Param), std::end(m_Param), nullptr);
}

}
