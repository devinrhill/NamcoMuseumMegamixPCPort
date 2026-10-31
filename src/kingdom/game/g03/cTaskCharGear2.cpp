#include "kingdom/game/g03/cTaskCharGear2.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include <iterator>
#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis05.hpp"
#include <Constants.hpp>

namespace kingdom::game::g03
{

const float cTaskCharGear2::m_fGearsForceDirs[4] = {::Constants::JET_DEG2RAD(3.0f), ::Constants::JET_DEG2RAD(270.0f), ::Constants::JET_DEG2RAD(177.0f), ::Constants::JET_DEG2RAD(90.0f)};
const Vector4 cTaskCharGear2::vPosList[4] = {Vector4(0.0f, 0.0f, -14.0f, 1.0f), Vector4(24.5f, 0.0f, 9.0f, 1.0f), Vector4(0.0f, 0.0f, 33.0f, 1.0f), Vector4(-24.5f, 0.0f, 10.0f, 1.0f)};

uint& cTaskCharGear2::nFlag()
{
	return m_nFlag;
}

void cTaskCharGear2::SetLevel(uint level)
{
	m_nLevel = level;
}

cTaskCharGolvis05* cTaskCharGear2::pBoss()
{
	CustomKingdomUtil::Assert(!m_pInfo.empty(), "nullpo");
	return m_pBoss;
}

// DOL 0x8011e8bc [audited: per AUDIT_G03_G07 row] (info array = 0x1c-byte records at +0x1c4, +0x14 / +0x18 left unwritten by the DOL; boss at +0x1c0; speeds FLOAT_8047bf10 = 3 deg/1200 frames; search point DAT_80378600 = (0,0,10,1), start distance 10000)
cTaskCharGear2::cTaskCharGear2(cTaskKingdom* pParent)
{
	pParent->ConnectChild(this);
	m_pBoss = static_cast<cTaskCharGolvis05*>(pParent);
	m_nLevel = 0u;
	m_nFlag = 0u;
	m_nGearNum = Constants::PACN_APP()->GetBgGimmickNum();
	m_pInfo.assign(m_nGearNum, {});
	m_nGearSpeed[0] = 0.0026179939f;
	m_nGearSpeed[1] = 0.0026179939f;
	m_nGearSpeed[2] = 0.0026179939f;
	m_nGearSpeed[3] = 0.0026179939f;
	uint num = std::numeric_limits<uint>::max();
	float num2 = 0.0f;
	float num3 = 10000.0f;
	Vector4 v = Vector4(0.0f, 0.0f, 10.0f, 1.0f);
	for (uint num4 = 0u; num4 < m_nGearNum; num4++)
	{
		m_pInfo[num4] = new cTaskCharGear2::GEARS_INFO();
		m_pInfo[num4]->m_nKind = 0u;
		m_pInfo[num4]->m_nSpeedNow = 0.0f;
		m_pInfo[num4]->m_nHeight = 0;
		m_pInfo[num4]->m_nStep = 0;
		m_pInfo[num4]->m_nMode = 0u;
		m_pInfo[num4]->m_nKind = 0u;
		Vector4 v2;
		::Constants::jetGeoVecSub(v2, v, Constants::PACN_APP()->pTaskBgGimmick(num4)->Model->vTrans());
		num2 = ::Constants::jetGeoVecLength(v2);
		if (num2 < num3)
		{
			num3 = num2;
			num = num4;
		}
		Constants::PACN_APP()->pTaskBgGimmick(num4)->nActFlag() |= 1073741824u;
	}
	m_pInfo[num]->m_nKind = 1u;
}

// DOL 0x8011eb2c [audited: class row done in the 43rd session; function not individually re-read]
cTaskCharGear2::~cTaskCharGear2()
{
	m_pInfo.clear();
}

// DOL 0x8011ebc0 [audited: per AUDIT_G03_G07 row] (step 1 = height change: speed step 5E-05 FLOAT_8047bf18, height step 0.1 FLOAT_8047bf1c, heights DAT_8043fbb8 = s_fHeight; UpdateFrame = gimmick +0x50 vtable +0x14)
bool cTaskCharGear2::Execute(cJetTask* pParam)
{
	bool flag = !Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss();
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss() && !Constants::IS_NAKAJIMA_PAUSE(this))
	{
		for (uint num = 0u; num < m_nGearNum; num++)
		{
			cTaskCharGear2::GEARS_INFO* gEARS_INFO = m_pInfo[num];
			if (gEARS_INFO->m_nKind != 1)
			{
				Vector4 _discard1;
				::Constants::jetGeoVecCopy(_discard1, Constants::PACN_APP()->pTaskBgGimmick(num)->Model->vTrans());
			}
			switch (gEARS_INFO->m_nStep)
			{
			case 0:
				if (gEARS_INFO->m_nMode == 0)
				{
					::Constants::bizMathNearValue(gEARS_INFO->m_nSpeedNow, gEARS_INFO->m_nSpeedNow, m_nGearSpeed[m_nLevel], 5E-05f);
				}
				else
				{
					::Constants::bizMathNearValue(gEARS_INFO->m_nSpeedNow, gEARS_INFO->m_nSpeedNow, 0.0f - m_nGearSpeed[m_nLevel], 5E-05f);
				}
				break;
			case 1:
				{
					if (gEARS_INFO->m_nMode == 0)
					{
						::Constants::bizMathNearValue(gEARS_INFO->m_nSpeedNow, gEARS_INFO->m_nSpeedNow, m_nGearSpeed[m_nLevel], 5E-05f);
					}
					else
					{
						::Constants::bizMathNearValue(gEARS_INFO->m_nSpeedNow, gEARS_INFO->m_nSpeedNow, 0.0f - m_nGearSpeed[m_nLevel], 5E-05f);
					}
					float pOut;
					if (::Constants::bizMathNearValue(pOut, Constants::PACN_APP()->pTaskBgGimmick(num)->Model->vTrans()[1], cTaskCharGolvis05::s_fHeight[gEARS_INFO->m_nHeight], 0.1f))
					{
						gEARS_INFO->m_nStep = 0;
					}
					Constants::PACN_APP()->pTaskBgGimmick(num)->Model->vTrans()[1] = pOut;
					break;
				}
			}
			if (flag)
			{
				Constants::PACN_APP()->pTaskBgGimmick(num)->Model->UpdateFrame();
			}
			RotateGear(num);
		}
	}
	return true;
}

// DOL 0x8011ee6c [audited: per AUDIT_G03_G07 row] (the DOL inlines GetRotateGearNo + RiviseRotateVector; force-dir table DAT_803785f0, gear position table DAT_80378610)
void cTaskCharGear2::RotateGear(uint nGear)
{
	cTaskCharGear2::GEARS_INFO* gEARS_INFO = m_pInfo[nGear];
	float num = 0.0f;
	CustomKingdomUtil::Assert(pBoss() != nullptr, "error");
	num = ((gEARS_INFO->m_nKind != 1) ? (gEARS_INFO->m_nSpeedNow * 4.0f) : gEARS_INFO->m_nSpeedNow);
	int subState = pBoss()->GetSubState();
	if ((subState == 10 || subState == 11 || subState == 12) && (pBoss()->GetCount(0u) == 3 || pBoss()->GetCount(0u) == 4) && gEARS_INFO->m_nKind == 1)
	{
		return;
	}
	if (!pBoss()->IsIzike())
	{
		if ((Constants::PACN_APP()->pTaskBgGimmick(nGear)->nActFlag() & 0xFF) == 2)
		{
			Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate()[1] += num;
		}
		else
		{
			Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate()[1] -= num;
		}
		RiviseRotateVector(Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate());
		gEARS_INFO->m_nRotateDir = 0;
		return;
	}
	::Constants::JET_DEG2RAD(360.0f);
	float num2 = ::Constants::JET_DEG2RAD(180.0f);
	if (gEARS_INFO->m_nKind == 1)
	{
		return;
	}
	num = ::Constants::JET_DEG2RAD(1.0f);
	Vector4 _discard3;
	::Constants::jetGeoVecCopy(_discard3, Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate());
	uint rotateGearNo = GetRotateGearNo(Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vTrans());
	CustomKingdomUtil::Assert(rotateGearNo < m_nGearNum, "error");
	float num3 = Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate()[1] - num2;
	CustomKingdomUtil::Assert((long)rotateGearNo < (long)(int)std::size(m_fGearsForceDirs), "error");
	float num4 = m_fGearsForceDirs[rotateGearNo] - num2;
	if (gEARS_INFO->m_nRotateDir == 0)
	{
		float num5 = num3;
		if (num4 < 0.0f)
		{
			if (num5 > 0.0f)
			{
				num5 -= ::Constants::JET_DEG2RAD(180.0f);
				if (num4 > num5)
				{
					gEARS_INFO->m_nRotateDir = 1;
				}
				else
				{
					gEARS_INFO->m_nRotateDir = 2;
				}
			}
			else if (num4 > num3)
			{
				gEARS_INFO->m_nRotateDir = 2;
			}
			else
			{
				gEARS_INFO->m_nRotateDir = 1;
			}
		}
		else if (num5 < 0.0f)
		{
			num5 += ::Constants::JET_DEG2RAD(180.0f);
			if (num4 > num5)
			{
				gEARS_INFO->m_nRotateDir = 1;
			}
			else
			{
				gEARS_INFO->m_nRotateDir = 2;
			}
		}
		else if (num4 > num3)
		{
			gEARS_INFO->m_nRotateDir = 2;
		}
		else
		{
			gEARS_INFO->m_nRotateDir = 1;
		}
	}
	if (num3 == num4)
	{
		return;
	}
	if (gEARS_INFO->m_nRotateDir == 1)
	{
		num = 0.0f - num;
		Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate()[1] += num;
	}
	else
	{
		Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate()[1] += num;
	}
	RiviseRotateVector(Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate());
	float num6 = Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate()[1] - num2;
	float array[2] = {num3, num6};
	for (uint num7 = 0u; num7 < 2; num7++)
	{
		if (array[num7] > num2)
		{
			array[num7] -= ::Constants::JET_DEG2RAD(360.0f);
		}
		else if (array[num7] < 0.0f - num2)
		{
			array[num7] += ::Constants::JET_DEG2RAD(360.0f);
		}
	}
	num3 = array[0];
	num6 = array[1];
	if (num6 > num4 - ::Constants::JET_DEG2RAD(1.0f) && num6 < num4 + ::Constants::JET_DEG2RAD(1.0f))
	{
		Constants::PACN_APP()->pTaskBgGimmick(nGear)->Model->vRotate()[1] = num4 + num2;
	}
}

void cTaskCharGear2::RiviseRotateVector(Vector4& v)
{
	float num = ::Constants::JET_DEG2RAD(360.0f);
	while (true)
	{
		if (v[1] > num)
		{
			v[1] -= num;
			continue;
		}
		if (v[1] < 0.0f)
		{
			v[1] += num;
			continue;
		}
		break;
	}
}

// DOL 0x8011f3a0 [audited: blr] (blr)
void cTaskCharGear2::Draw(int nStatus, cJetTask* pParam)
{
}

// inlined in the DOL: 0x8011da4c cTaskCharGolvis05 gear patterns (mode = height > level, step 1, height = level)
void cTaskCharGear2::ChangeHeight(int nIndex, int nLevel)
{
	if (m_pInfo[nIndex]->m_nHeight != nLevel)
	{
		if (m_pInfo[nIndex]->m_nHeight > nLevel)
		{
			m_pInfo[nIndex]->m_nMode = 1u;
		}
		else
		{
			m_pInfo[nIndex]->m_nMode = 0u;
		}
		m_pInfo[nIndex]->m_nStep = 1;
		m_pInfo[nIndex]->m_nHeight = nLevel;
	}
}

bool cTaskCharGear2::LimitLargeArea(Vector4& vOut, const Vector4& vPos, const Vector4& vVec)
{
	Vector4 v;
	::Constants::jetGeoVecAdd(v, vPos, vVec);
	if (cTaskCharGolvis05::s_CheckLimit(vPos) || cTaskCharGolvis05::s_CheckLimit(v))
	{
		::Constants::jetGeoVecCopy(vOut, v);
		return true;
	}
	return false;
}

uint cTaskCharGear2::GetRotateGearNo(const Vector4& v)
{
	float num = 0.0f;
	uint num2 = std::numeric_limits<uint>::max();
	float num3 = 100000.0f;
	for (uint num4 = 0u; num4 < 4; num4++)
	{
		Vector4 v2;
		::Constants::jetGeoVecSub(v2, v, vPosList[num4]);
		num = ::Constants::jetGeoVecLength(v2);
		if (num < num3)
		{
			num3 = num;
			num2 = num4;
		}
	}
	CustomKingdomUtil::Assert(num2 != std::numeric_limits<uint>::max(), "error");
	return num2;
}

}
