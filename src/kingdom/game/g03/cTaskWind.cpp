#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
#include "kingdom/game/g03/cTaskWind.hpp"

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

Vector4& cTaskWind::vTrans()
{
	return m_vTrans;
}

float cTaskWind::vTrans(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vTrans[n];
}

Vector4& cTaskWind::vRotate()
{
	return m_vRotate;
}

float cTaskWind::vRotate(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vRotate[n];
}

Vector4& cTaskWind::vScale()
{
	return m_vScale;
}

float cTaskWind::vScale(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vScale[n];
}

Vector4& cTaskWind::vWindForce()
{
	return m_vWindForce;
}

float cTaskWind::vWindForce(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vWindForce[n];
}

// DOL 0x801893a0 [audited: per AUDIT_G03_G07 row] (record 0x30 bytes: position / 1000, rotation degrees -> radians, scale / 1000, wind rotation, force / 1000)
cTaskWind::cTaskWind(sSaveDataWind& wind)
{
	m_hPtkWind = new cKingdomPtkHnd();
	SetName("cTaskWind");
	Init();
	float num = 1000.0f;
	vTrans()[0] = (float)wind.vnTrans[0] / num;
	vTrans()[1] = (float)wind.vnTrans[1] / num;
	vTrans()[2] = (float)wind.vnTrans[2] / num;
	vTrans()[3] = 1.0f;
	vRotate()[0] = ::Constants::JET_DEG2RAD(wind.vnRotate[0]);
	vRotate()[1] = ::Constants::JET_DEG2RAD(wind.vnRotate[1]);
	vRotate()[2] = ::Constants::JET_DEG2RAD(wind.vnRotate[2]);
	vRotate()[3] = 0.0f;
	vScale()[0] = (float)wind.vnScale[0] / num;
	vScale()[1] = (float)wind.vnScale[1] / num;
	vScale()[2] = (float)wind.vnScale[2] / num;
	vScale()[3] = 1.0f;
	m_vRotWind[0] = ::Constants::JET_DEG2RAD(wind.vnRotWind[0]);
	m_vRotWind[1] = ::Constants::JET_DEG2RAD(wind.vnRotWind[1]);
	m_vRotWind[2] = ::Constants::JET_DEG2RAD(wind.vnRotWind[2]);
	m_vRotWind[3] = 1.0f;
	::Constants::jetGeoVecUnit(vWindForce());
	vWindForce()[2] = 1.0f;
	::Constants::jetGeoVecRotZYX(vWindForce(), vWindForce(), m_vRotWind);
	::Constants::jetGeoVecScale(m_vWindVecRev, vWindForce(), -1.0f);
	m_fForce = (float)wind.nForce / num;
	::Constants::jetGeoVecScale(vWindForce(), vWindForce(), m_fForce);
}

cTaskWind::~cTaskWind()
{
	Term();
}

// DOL 0x80189a64 [audited: per AUDIT_G03_G07 row]
void cTaskWind::Init()
{
	::Constants::jetGeoVecUnit(vTrans());
	::Constants::jetGeoVecUnit(vRotate());
	::Constants::jetGeoVecOne(vScale());
	::Constants::jetGeoVecUnit(vWindForce());
}

void cTaskWind::Term()
{
}

// DOL 0x80189ad8 [audited: per AUDIT_G03_G07 row] (effect by round: 11 / 13 -> 16, 15 / 17 -> 17, 23 -> 36; pushes Pac-Man's move by the wind force each frame inside the area)
bool cTaskWind::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (Constants::IS_PAC_PAUSE() || !cAppG03PacnRollGame2->IsGame())
	{
		return true;
	}
	if (AreaCheck())
	{
		if (!m_hPtkWind->IsValid())
		{
			switch (KingdomGlobal::PacnRollSys->nRound())
			{
			case 11u:
			case 13u:
				cAppG03PacnRollGame2->SetEffect(16u, cTaskCharPacman2->vTrans(), m_vRotWind, m_hPtkWind);
				break;
			case 15u:
			case 17u:
				cAppG03PacnRollGame2->SetEffect(17u, cTaskCharPacman2->vTrans(), m_vRotWind, m_hPtkWind);
				break;
			case 23u:
				cAppG03PacnRollGame2->SetEffect(36u, cTaskCharPacman2->vTrans(), m_vRotWind, m_hPtkWind);
				break;
			}
		}
		if (m_hPtkWind->IsValid())
		{
			Vector4 v;
			::Constants::jetGeoVecScale(v, m_vWindVecRev, 8.0f);
			::Constants::jetGeoVecAdd(v, cTaskCharPacman2->vTrans(), v);
			m_hPtkWind->vSetTrans(v);
			m_hPtkWind->vSetRotate(m_vRotWind);
			if (!SoundManager::get_SE()->IsPlaying(m_hSndWind))   // DOL 0x80189ad8: SoundSeBank::IsPlaying (0x80026424), not a null test: the loop restarts when the area is entered again
			{
				if (cPacnRollLoopSndMan::CanPlay()) m_hSndWind = cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE026_KAZE, true));
			}
		}
		::Constants::jetGeoVecAdd(cTaskCharPacman2->vMove(), cTaskCharPacman2->vMove(), vWindForce());
	}
	else
	{
		if (nullptr != m_hSndWind)
		{
			SoundManager::get_SE()->Stop(m_hSndWind);
		}
		if (m_hPtkWind->IsValid())
		{
			m_hPtkWind->Detach(true);
		}
	}
	return true;
}

void cTaskWind::Draw(int nStatus, cJetTask* pParam)
{
}

// DOL 0x80189cd8 [audited: per AUDIT_G03_G07 row] (oriented box: half scale 0.5, translate, then Z, Y, X rotations; inside when the squared distance is 0)
bool cTaskWind::AreaCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector3 min = Vector3();
	min.x = (0.0f - vScale()[0]) / 2.0f;
	min.y = (0.0f - vScale()[1]) / 2.0f;
	min.z = (0.0f - vScale()[2]) / 2.0f;
	Vector3 max = Vector3();
	max.x = vScale()[0] / 2.0f;
	max.y = vScale()[1] / 2.0f;
	max.z = vScale()[2] / 2.0f;
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, vRotate());
	Matrix4x4 m2;
	::Constants::jetGeoMtx44to34(m2, m);
	AABB target = AABB(min, max, false);
	target.Set(target, m2);
	Vector3 trans = Vector3();
	trans.x = cTaskCharPacman2->vTrans()[0];
	trans.y = cTaskCharPacman2->vTrans()[1];
	trans.z = cTaskCharPacman2->vTrans()[2];
	float _discard2;
	if (DummyNW4R::DistSqPoint3ToAABB(trans, target, _discard2) != 0.0f)
	{
		return false;
	}
	return true;
}

}
