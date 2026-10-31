#include "kingdom/game/g07/cTaskConsPacIcon.hpp"

#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

float cTaskConsPacIcon::get_PAC_ICON_POS_RIVISE()
{
	return 30.0f;
}

cTaskConsPacIcon::cTaskConsPacIcon()
{
	Init();
}

cTaskConsPacIcon::~cTaskConsPacIcon()
{
	Free();
}

void cTaskConsPacIcon::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

float& cTaskConsPacIcon::fConsoleX(uint set)
{
	CustomKingdomUtil::Assert(set < 5, "error");
	return m_aConsoleX[set];
}

cTaskAnita* cTaskConsPacIcon::Construct()
{
	return new cTaskConsPacIcon();
}

// DOL 0x801009f8 [audited: per AUDIT_G03_G07 row]
void cTaskConsPacIcon::SetpExecNumber(Ref<int>* set)
{
	m_pExecNumber = set;
}

uint& cTaskConsPacIcon::nPlayerID()
{
	return m_nPlayerID;
}

void cTaskConsPacIcon::Init()
{
	m_pExecNumber = nullptr;
	for (uint num = 0u; num < 5; num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num)->SetAnimNo(130);
		pAnita(num)->SetState(3u, false);
		fConsoleX(num) = get_PAC_ICON_POS_RIVISE() * (float)num;
	}
	::Constants::jetGeoVecUnit(vTrans());
	m_nPlayerID = 0u;
}

void cTaskConsPacIcon::Term()
{
}

// DOL 0x80102730 [audited: per AUDIT_G03_G07 row] (compared 2026-10-30)
bool cTaskConsPacIcon::Execute(cJetTask* pParam)
{
	CustomKingdomUtil::Assert(m_pExecNumber != nullptr, "nullpo");
	for (uint num = 0u; num < 5; num++)
	{
		if ((long)num < (long)m_pExecNumber->Value)
		{
			if (!IsAnmNoSame(num, 134) && !IsAnmNoSame(num, 135))
			{
				SetAnm(num, 134);
			}
			else if ((IsAnmNoSame(num, 134) || IsAnmNoSame(num, 135)) && IsAnmNoSame(num, 134) && IsAnmEnd(num))
			{
				SetAnmDiffCurrent(num, 135);
			}
		}
		else if (IsAnmNoSame(num, 130) && IsAnmEnd(num))
		{
			SetAnmDiffCurrent(num, 131);
		}
		::Constants::jetGeoVecCopy(pAnita(num)->vTrans(), vTrans());
		pAnita(num)->vTrans()[0] += fConsoleX(num);
	}
	return cTaskConsoleBase::Execute(pParam);
}

// DOL 0x80102954 [audited: per AUDIT_G03_G07 row]
void cTaskConsPacIcon::Visible(uint set)
{
	if (set == std::numeric_limits<uint>::max())
	{
		for (uint num = 0u; num < 5; num++)
		{
			pAnita(num)->SetState(3u);
		}
	}
	else
	{
		CustomKingdomUtil::Assert(pAnita(set) != nullptr, "nullpo");
		pAnita(set)->SetState(3u);
	}
}

// DOL 0x801029d0 [audited: per AUDIT_G03_G07 row]
void cTaskConsPacIcon::Invisible(uint set)
{
	if (set == std::numeric_limits<uint>::max())
	{
		for (uint num = 0u; num < 5; num++)
		{
			pAnita(num)->SetState(3u, false);
		}
	}
	else
	{
		CustomKingdomUtil::Assert(pAnita(set) != nullptr, "nullpo");
		pAnita(set)->SetState(3u, false);
	}
}

}
