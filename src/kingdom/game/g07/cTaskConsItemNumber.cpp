#include "kingdom/game/g07/cTaskConsItemNumber.hpp"

#include <iterator>
#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

float cTaskConsItemNumber::X_OFFSET = 20.0f;

cTaskConsItemNumber::cTaskConsItemNumber(int nFigure)
{
	CustomKingdomUtil::Assert(nFigure > 0, "error!");
	m_nDrawFigure = nFigure;
	Init();
}

cTaskConsItemNumber::~cTaskConsItemNumber()
{
	Free();
}

void cTaskConsItemNumber::Free()
{
	Term();
	cTaskConsoleBase::Free();
}

bool& cTaskConsItemNumber::bChange(uint set)
{
	CustomKingdomUtil::Assert(set < 9, "error");
	return m_bChange[set];
}

void cTaskConsItemNumber::pExecNumber(Ref<int>* set)
{
	m_pExecNumber = set;
}

bool& cTaskConsItemNumber::bExec()
{
	return m_bExec;
}

float& cTaskConsItemNumber::fOffsetX()
{
	return m_fOffsetX;
}

void cTaskConsItemNumber::SetFlag(uint flag, bool sw)
{
	if (sw)
	{
		m_nFlag |= flag;
	}
	else
	{
		m_nFlag &= ~flag;
	}
}

uint cTaskConsItemNumber::GetFlag()
{
	return m_nFlag;
}

void cTaskConsItemNumber::Init()
{
	m_bExec = false;
	m_fOffsetX = X_OFFSET;
	::Constants::jetGeoVecUnit(vTrans());
	for (int i = 0; i < (int)std::size(m_nNumber); i++)
	{
		m_nNumber[i] = std::numeric_limits<uint>::max();
	}
	m_pExecNumber = nullptr;
	m_nFlag = 1u;
	for (uint num = 0u; (long)num < (long)(m_nDrawFigure + 1); num++)
	{
		pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
		pAnita(num)->SetAnimNo(66);
		pAnita(num)->SetState(3u, false);
	}
	for (int j = 0; j < (int)std::size(m_bChange); j++)
	{
		m_bChange[j] = false;
	}
}

void cTaskConsItemNumber::Term()
{
}

// DOL 0x800ff220 [audited: per AUDIT_G03_G07 row] (digit tables 0x803762f0 = AnmList_d, 0x80376318 = AnmList_t)
bool cTaskConsItemNumber::Execute(cJetTask* pParam)
{
	CustomKingdomUtil::Assert(m_pExecNumber != nullptr, "nullpo");
	int num = 0;
	if ((m_nFlag & 4) != 0)
	{
		num = 1;
		Invisible(0u);
	}
	uint num2 = (uint)num;
	uint num3 = (uint)(1 + m_nDrawFigure);
	int num4 = 0;
	if ((m_nFlag & 0x20) != 0)
	{
		int value = m_pExecNumber->Value;
		for (int i = 0; i < m_nDrawFigure; i++)
		{
			int num5 = value;
			float num6 = 1.0f;
			for (int j = 0; j < i; j++)
			{
				num6 *= 10.0f;
			}
			num5 = (int)((float)num5 / num6);
			if (num5 % 10 != 0)
			{
				num4 = i;
			}
		}
	}
	for (uint num7 = num2; num7 < num3; num7++)
	{
		if (m_bExec)
		{
			if ((m_nFlag & 0x20) != 0)
			{
				if (num4 == 0 && num7 == num3 - 1)
				{
					Visible(num7);
				}
				else if ((long)num7 <= (long)num4)
				{
					uint set = num3 - 1 - num7;
					Visible(set);
				}
				else
				{
					uint set2 = num3 - 1 - num7;
					Invisible(set2);
				}
			}
			else
			{
				Visible(num7);
			}
			int num8 = m_pExecNumber->Value;
			if (num8 < 0)
			{
				num8 = 0;
			}
			if (num7 != 0)
			{
				int num9 = num8;
				int num10 = m_nDrawFigure - (int)(num7 - 1) - 1;
				if (num10 == 0)
				{
					num9 %= 10;
				}
				else
				{
					float num11 = 10.0f;
					for (uint num12 = 0u; (long)num12 < (long)(num10 - 1); num12++)
					{
						num11 *= 10.0f;
					}
					num9 = (int)((float)num9 / num11);
					num9 %= 10;
				}
				if ((m_nFlag & 0x40) != 0)
				{
					if (pAnita(num7)->GetAnimNo() != AnmList_t[num9])
					{
						float frame = pAnita(num7)->GetFrame();
						SetAnm(num7, AnmList_t[num9]);
						pAnita(num7)->SetFrame(frame);
						pAnita(num7)->AnimUpdate(0.0f);   // DOL 0x80141c44(0)
					}
				}
				else if (IsAnmEnd(num7) && pAnita(num7)->GetAnimNo() == AnmList_d[num9])
				{
					pAnita(num7)->SetAnimNo(AnmList_t[num9]);
					bChange(num7) = true;
				}
				else if (bChange(num7) && pAnita(num7)->GetAnimNo() != AnmList_t[num9])
				{
					float frame2 = pAnita(num7)->GetFrame();
					SetAnm(num7, AnmList_t[num9]);
					pAnita(num7)->SetFrame(frame2);
					pAnita(num7)->AnimUpdate(0.0f);   // DOL 0x80141c44(0)
				}
				else if (pAnita(num7)->GetAnimNo() != AnmList_t[num9] && pAnita(num7)->GetAnimNo() != AnmList_d[num9])
				{
					pAnita(num7)->SetAnimNo(AnmList_d[num9]);
				}
			}
			if (num7 != 0)
			{
				if ((m_nFlag & 1) != 0)
				{
					::Constants::jetGeoVecCopy(pAnita(num7)->vTrans(), vTrans());
					pAnita(num7)->vTrans()[0] += m_fOffsetX * ((float)num7 - (float)num);
				}
				else if ((m_nFlag & 2) != 0)
				{
					int n = (int)(num3 - num7);
					float num13 = m_fOffsetX * ((float)num7 - (float)num);
					::Constants::jetGeoVecCopy(pAnita((uint)n)->vTrans(), vTrans());
					pAnita((uint)n)->vTrans()[0] -= num13;
				}
			}
			else
			{
				::Constants::jetGeoVecCopy(pAnita(num7)->vTrans(), vTrans());
				if ((m_nFlag & 0x10) != 0)
				{
					SetAnmDiffCurrent(num7, 64);
				}
				else if (!m_bChange[num7])
				{
					SetAnmDiffCurrent(num7, 66);
					m_bChange[num7] = true;
				}
				else if (IsAnmEnd(num7))
				{
					SetAnmDiffCurrent(num7, 67);
				}
			}
		}
		else
		{
			pAnita(num7)->SetState(3u, false);
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

}
