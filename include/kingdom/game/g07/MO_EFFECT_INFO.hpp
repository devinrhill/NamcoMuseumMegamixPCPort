#pragma once

#include "common.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g07
{

class MO_EFFECT_INFO
{
public:
	using Func = std::function<void(MO_EFFECT_INFO* info)>;

	ushort m_nFlag{};

	ushort m_nID{};

	ushort m_nKind{};

	ushort m_nTimer{};

	ushort m_nDelay{};

	bool m_bDie{};

	cKingdomPtkHnd* m_phPtk{};

	MO_EFFECT_INFO::Func m_Func{};

	cJetTask* m_Param[3]{};

	MO_EFFECT_INFO();
};

}
