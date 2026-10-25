#include "kingdom/game/menu/cTaskConsStart.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include <Constants.hpp>

namespace kingdom::game::menu
{

uint& cTaskConsStart::nPlayerNo()
{
	return m_nPlayerNo;
}

uint& cTaskConsStart::nCtrlNo()
{
	return m_nCtrlNo;
}

int& cTaskConsStart::nHitPat()
{
	return m_nHitPat;
}

bool& cTaskConsStart::bSelectFlag()
{
	return m_bSelectFlag;
}

sMatchingData::InfoData*& cTaskConsStart::pInfo()
{
	return m_pInfo;
}

Vector4& cTaskConsStart::vTrans()
{
	return m_vTrans;
}

Vector4& cTaskConsStart::vMiiNamePos()
{
	return m_vMiiNamePos;
}

float& cTaskConsStart::fMaxStringWidth()
{
	return m_fMaxStringWidth;
}

cTaskConsStart::~cTaskConsStart()
{
}

bool cTaskConsStart::IsSelectFlag()
{
	return bSelectFlag();
}

uint cTaskConsStart::nGetPlayerNo()
{
	return nPlayerNo();
}

void cTaskConsStart::nSetCtrlNo(uint nNo)
{
	nCtrlNo() = nNo;
}

uint cTaskConsStart::nGetCtrlNo()
{
	return nCtrlNo();
}

cTaskConsStart::cTaskConsStart(int nId, string AnitaKey, string SetKey, uint nPlayerNo)
	: cTaskConsMatching(nId)
{
	this->nPlayerNo() = nPlayerNo;
	bSelectFlag() = false;
	pAnita(0u)->RegistData(AnitaKey);
	pAnita(0u)->SetState(1u, false);
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	Vector4 v2;
	::Constants::jetGeoVecZero(v2);
	pAnita(0u)->GetLayerPos(v, STR_SET_LAY[0], 31);
	pAnita(0u)->GetLayerPos(v2, STR_SET_LAY[1], 31);
	fMaxStringWidth() = v2[0] - v[0];
	nHitPat() = HIT_PAT[nPlayerNo];
}

void cTaskConsStart::RegistMii(sMatchingData::InfoData* pInfo)
{
	CustomKingdomUtil::Assert(pInfo != nullptr);
	this->pInfo() = pInfo;
}

int cTaskConsStart::nGetStartAnmNo(uint nState)
{
	int obj[4][6] = {{120, 121, 122, 123, 124, 125}, {126, 127, 128, 129, 130, 131}, {132, 133, 134, 135, 136, 137}, {138, 139, 140, 141, 142, 143}};
	CustomKingdomUtil::Assert(0 <= nState && nState < 6, "error");
	return obj[nPlayerNo()][nState];
}

bool cTaskConsStart::IsDpdHit(float x, float y)
{
	return false;
}

void cTaskConsStart::Cancel()
{
	bSelectFlag() = false;
}

bool cTaskConsStart::AppearProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStartAnmNo(0u));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsStart::WaitProc()
{
	pAnita(0u)->SetAnimNo(nGetStartAnmNo(1u), false);
}

void cTaskConsStart::PointProc()
{
	pAnita(0u)->SetAnimNo(nGetStartAnmNo(2u), false);
}

bool cTaskConsStart::DecideProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetStartAnmNo(3u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		bSelectFlag() = true;
		return true;
	}
	return false;
}

bool cTaskConsStart::DeleteProc()
{
	if (nStep() == 0)
	{
		if (bSelectFlag())
		{
			pAnita(0u)->SetAnimNo(nGetStartAnmNo(5u), false);
		}
		else
		{
			pAnita(0u)->SetAnimNo(nGetStartAnmNo(4u), false);
		}
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		bSelectFlag() = false;
		return true;
	}
	return false;
}

bool cTaskConsStart::Execute(cJetTask* pParam)
{
	return cTaskConsMatching::Execute(pParam);
}

}
