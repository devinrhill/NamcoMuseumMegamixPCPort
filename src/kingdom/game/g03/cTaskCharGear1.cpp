#include "kingdom/game/g03/cTaskCharGear1.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include <numbers>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis04.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

void cTaskCharGear1::SetLevel(uint level)
{
	m_nLevel = level;
}

uint& cTaskCharGear1::nFlag()
{
	return m_nFlag;
}

cTaskCharGear1::cTaskCharGear1(cTaskKingdom* pParent)
{
	cKingdomCSV* obj = new cKingdomCSV();
	obj->RegistData("BIN_CSV_BOSS_PARAM");
	pParent->ConnectChild(this);
	m_nLevel = 0u;
	m_nFlag = 0u;
	m_nGearNum = Constants::PACN_APP()->GetBgGimmickNum();
	m_nGearKind.assign(m_nGearNum, {});
	m_nGearSpeedNow.assign(m_nGearNum, {});
	int num = obj->SearchRow(0, "GEAR_LIFE3");
	CustomKingdomUtil::Assert(0 <= num, "error");
	int _out;
	obj->S32(num, 1, _out);
	m_nGearSpeed[0] = (float)std::numbers::pi * 2.0f / (float)_out;
	num = obj->SearchRow(0, "GEAR_LIFE2");
	CustomKingdomUtil::Assert(0 <= num, "error");
	obj->S32(num, 1, _out);
	m_nGearSpeed[1] = (float)std::numbers::pi * 2.0f / (float)_out;
	num = obj->SearchRow(0, "GEAR_LIFE1");
	CustomKingdomUtil::Assert(0 <= num, "error");
	obj->S32(num, 1, _out);
	m_nGearSpeed[2] = (float)std::numbers::pi * 2.0f / (float)_out;
	m_nGearSpeed[3] = 0.0f;
	for (uint num2 = 0u; num2 < m_nGearNum; num2++)
	{
		m_nGearKind[num2] = 0u;
		m_nGearSpeedNow[num2] = 0.0f;
		Constants::PACN_APP()->pTaskBgGimmick(num2)->nActFlag() |= 1073741824u;
		m_nGearKind[num2] = ((Constants::PACN_APP()->pTaskBgGimmick(num2)->GetRotFrameMax() == 1200) ? 1u : 0u);
	}
}

cTaskCharGear1::~cTaskCharGear1()
{
	m_nGearKind.clear();
	m_nGearSpeedNow.clear();
}

bool cTaskCharGear1::Execute(cJetTask* pParam)
{
	float num = 1.0f;
	Constants::PACN_APP()->pTaskBoss()->GetAIMode();
	(void)(13);
	if ((!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss() && !Constants::IS_NAKAJIMA_PAUSE()) || (!Constants::IS_PAC_PAUSE() && (static_cast<cTaskCharGolvis04*>(pParent()))->GetDeathProc() >= 2))
	{
		for (uint num2 = 2u; num2 < m_nGearNum; num2++)
		{
			if (CustomKingdomUtil::IF(m_nFlag & 2))
			{
				::Constants::bizMathNearValue(m_nGearSpeedNow[num2], m_nGearSpeedNow[num2], 0.0f, 0.0002f);
			}
			else if (CustomKingdomUtil::IF(m_nFlag & 1))
			{
				::Constants::bizMathNearValue(m_nGearSpeedNow[num2], m_nGearSpeedNow[num2], 0.0f - m_nGearSpeed[m_nLevel], 5E-05f);
			}
			else
			{
				::Constants::bizMathNearValue(m_nGearSpeedNow[num2], m_nGearSpeedNow[num2], m_nGearSpeed[m_nLevel], 5E-05f);
			}
		}
		for (uint num3 = 2u; num3 < m_nGearNum; num3++)
		{
			float num4 = ((m_nGearKind[num3] != 1) ? (m_nGearSpeedNow[num3] * 2.0f * num) : (m_nGearSpeedNow[num3] * num));
			if ((Constants::PACN_APP()->pTaskBgGimmick(num3)->nActFlag() & 0xFF) == 2)
			{
				Constants::PACN_APP()->pTaskBgGimmick(num3)->Model->vRotate()[1] += num4;
			}
			else
			{
				Constants::PACN_APP()->pTaskBgGimmick(num3)->Model->vRotate()[1] -= num4;
			}
		}
	}
	return true;
}

void cTaskCharGear1::Draw(int nStatus, cJetTask* pParam)
{
}

bool cTaskCharGear1::s_CheckLimit(const Vector4& vPos)
{
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskBgGimmick(2u)->Model->vTrans());
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, Constants::PACN_APP()->pTaskBgGimmick(3u)->Model->vTrans());
	bool result = false;
	if (vPos[0] > v[0] - 10.0f && vPos[0] < v[0] + 10.0f && vPos[2] > v[2] - 10.0f && vPos[2] < v[2] + 10.0f)
	{
		result = true;
	}
	if (vPos[0] > v2[0] - 10.0f && vPos[0] < v2[0] + 10.0f)
	{
		result = true;
	}
	if (vPos[2] > v2[2] - 10.0f && vPos[2] < v2[2] + 10.0f)
	{
		result = true;
	}
	return result;
}

int cTaskCharGear1::GetGear(const Vector4& vPos)
{
	for (int i = 2; i <= 3; i++)
	{
		if (::Constants::bizGeoVecDistance2XZ(Constants::PACN_APP()->pTaskBgGimmick((uint)i)->Model->vTrans(), vPos) < 100.0f)
		{
			return i;
		}
	}
	for (int j = 4; j <= 7; j++)
	{
		if (::Constants::bizGeoVecDistance2XZ(Constants::PACN_APP()->pTaskBgGimmick((uint)j)->Model->vTrans(), vPos) < 25.0f)
		{
			return j;
		}
	}
	if (vPos[0] > Constants::CENTER_GEAR_POS[0] - 4.0f && vPos[0] < Constants::CENTER_GEAR_POS[0] + 4.0f && vPos[2] > Constants::CENTER_GEAR_POS[2] - 4.0f && vPos[2] < Constants::CENTER_GEAR_POS[2] + 4.0f)
	{
		return 2;
	}
	return -1;
}

bool cTaskCharGear1::LimitLargeArea(Vector4& vOut, const Vector4& vPos, const Vector4& vVec)
{
	Vector4 v;
	::Constants::jetGeoVecAdd(v, vPos, vVec);
	if ((!cTaskCharGear1::s_CheckLimit(vPos) && cTaskCharGear1::s_CheckLimit(v)) || (cTaskCharGear1::s_CheckLimit(vPos) && cTaskCharGear1::s_CheckLimit(v)))
	{
		::Constants::jetGeoVecCopy(vOut, v);
		return true;
	}
	return false;
}

}
