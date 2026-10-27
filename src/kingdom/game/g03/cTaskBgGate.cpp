#include <cstdlib>
#include <cstdio>
#include "kingdom/game/g03/cTaskBgGate.hpp"

#include <iterator>

#include "AABB.hpp"
#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "DummyNW4R.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskPBModel.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/cTaskGateCookie.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

uint cTaskBgGate::nKind()
{
	return m_nKind;
}

bool& cTaskBgGate::IsWake()
{
	return m_bWake;
}

bool cTaskBgGate::IsBreak()
{
	if (m_nNeedCookie != 0)
	{
		return false;
	}
	return true;
}

uint& cTaskBgGate::nNeedCookie()
{
	return m_nNeedCookie;
}

uint cTaskBgGate::GetPriority()
{
	return m_nPriority;
}

bool cTaskBgGate::IsDrain()
{
	return m_bDrain;
}

cTaskBgGate::cTaskBgGate()
{
	Init();
}

void cTaskBgGate::Init()
{
	m_bWake = false;
	m_nNeedCookie = 0u;
	m_nCnt = 0u;
	m_nKind = 0u;
	m_pTaskGateCookie = nullptr;
	m_pBreakModel = nullptr;
	m_nPriority = 0u;
	m_nDrawFlag = 0u;
	m_fCheckRange = 5.0f;
	m_bDrain = false;
	m_bFirstBreak = false;
	NextMode(0u, 0u);
	this->Model->Visible();
	cKingdomCSV* obj = new cKingdomCSV();
	obj->RegistData("BIN_GATE_PARAM_CSV");
	int num = obj->SearchCol(0, "GATE_PARAM");
	CustomKingdomUtil::Assert(0 <= num, "error");
	int num2 = obj->SearchRow(1, "COOKE_CHECK_RANGE");
	CustomKingdomUtil::Assert(0 <= num2, "error");
	CustomKingdomUtil::Assert(obj->F32(num2, num, m_fCheckRange), "error");
	for (int i = 0; i < (int)std::size(m_PtkHnd); i++)
	{
		m_PtkHnd[i] = new cKingdomPtkHnd();
	}
}

void cTaskBgGate::Term()
{
	m_pBreakModel->TermOnce();
	m_pTaskGateCookie->TermOnce();
	cTaskBgHit::Term();
}

bool cTaskBgGate::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch1_0u:
			nModeStep(n) = 1u;
			if (m_nNeedCookie != 0)
			{
				NextMode(n, 1u);
			}
			else
			{
				m_PtkHnd[0]->Detach(true);
				m_PtkHnd[1]->Detach(true);
				m_pTaskGateCookie->SetCnt(m_nNeedCookie);
				m_pTaskGateCookie->Invisible();
				NextMode(n, 3u);
			}
			goto _switch1_1u;
		case 1u:
		_switch1_1u:
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch1_2u;
		case 2u:
		_switch1_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (1 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch2_0u:
			nModeStep(n) = 1u;
			m_nCnt = 0u;
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			if (IsWake())
			{
				if (AreaCheck())
				{
					m_bDrain = true;
					if (CustomKingdomUtil::IF(cAppG03PacnRollGame2->pTaskItem()->nHaveCookie()) && CustomKingdomUtil::IF(m_nNeedCookie) && ++m_nCnt % 2 == 0)
					{
						SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE027_COUNT);
						cAppG03PacnRollGame2->pTaskItem()->nHaveCookie()--;
						m_nNeedCookie--;
						if (m_nNeedCookie == 0)
						{
							SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE030_ESA_TRITA);
						}
					}
				}
				else
				{
					m_bDrain = false;
				}
			}
			m_pTaskGateCookie->SetCnt(m_nNeedCookie);
			m_nDrawFlag = 0u;
			if (m_nNeedCookie == 0)
			{
				m_PtkHnd[0]->Detach(true);
				m_PtkHnd[1]->Detach(true);
				m_bDrain = false;
				m_pTaskGateCookie->Invisible();
				NextMode(n, 2u);
			}
			else if (pFaceList() != nullptr)
			{
				cAppG03PacnRollGame2->pTaskCollision()->PushBackCollisionObj(this);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch2_2u;
		case 2u:
		_switch2_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (2 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch3_0u:
			nModeStep(n) = 1u;
			m_nCnt = 0u;
			if (m_bFirstBreak)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE037_BOX_HAKAI);
			}
			cAppG03PacnRollGame2->SetEffect((m_nKind == 1) ? 32u : 31u, this->Model->vTrans(), Vector4(), nullptr, this->Model->vRotate()[1]);
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			m_nCnt++;
			m_nDrawFlag = 1u;
			if (m_nCnt >= 60)
			{
				NextMode(n, 3u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch3_2u;
		case 2u:
		_switch3_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (3 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch4_0u:
			nModeStep(n) = 1u;
			m_nDrawFlag = 2u;
			goto _switch4_1u;
		case 1u:
		_switch4_1u:
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch4_2u;
		case 2u:
		_switch4_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	return true;
}

void cTaskBgGate::Draw(int nStatus, cJetTask* pParam)
{
	if (m_nDrawFlag == 0)
	{
		PushBack();
		if (!this->Model->IsVisible())
		{
			this->Model->Visible();
		}
		if (m_pBreakModel->Model->IsVisible())
		{
			m_pBreakModel->Model->Invisible();
		}
	}
	else if (m_nDrawFlag == 1)
	{
		m_pBreakModel->PushBack();
		if (this->Model->IsVisible())
		{
			this->Model->Invisible();
		}
		if (m_nCnt % 2 == 0)
		{
			if (!m_pBreakModel->Model->IsVisible())
			{
				m_pBreakModel->Model->Visible();
			}
		}
		else if (m_pBreakModel->Model->IsVisible())
		{
			m_pBreakModel->Model->Invisible();
		}
	}
	else
	{
		if (this->Model->IsVisible())
		{
			this->Model->Invisible();
		}
		if (m_pBreakModel->Model->IsVisible())
		{
			m_pBreakModel->Model->Invisible();
		}
	}
}

bool cTaskBgGate::RegistData(const uint& kind, const uint& cookieNum, const uint& pri, const Vector4& trans, const Vector4& rotate)
{
	Constants::PACN_APP();
	m_nNeedCookie = cookieNum;
	m_nPriority = pri;
	m_nKind = kind;
	GameObject* gameObject = kingdom::LoadModelObject("BIN_GATE_1_MDL");
	this->Model->RegistModel(gameObject);
	if (kind == 1)
	{
		RegistHitData("BIN_GATE_HIT_S_MDL");
	}
	else
	{
		RegistHitData("BIN_GATE_HIT_MDL");
	}
	this->Model->SetLightSetIdx(3);
	this->Model->SetFogIdx(0);
	::Constants::jetGeoVecCopy(this->Model->vTrans(), trans);
	::Constants::jetGeoVecCopy(this->Model->vRotate(), rotate);
	if (kind == 0 || kind == 2)
	{
		Vector4 v = Vector4::zero;
		Vector4 v2 = Vector4::zero;
		Vector4 v3 = Vector4::zero;
		string array[2] = {"ef_gate01", "ef_gate02"};
		ResMdl* resMdl = new ResMdl(gameObject);
		for (uint num = 0u; num < 2; num++)
		{
			ResNode* resNode = resMdl->GetResNode(array[num]);
			if (resNode->IsValid())
			{
				Vector3 vector = resNode->GetTranslate();
				::Constants::jetGeoVecSet(v, vector.x, vector.y, vector.z, 1.0f);
				::Constants::jetGeoVecRotY(v, v, this->Model->vRotate()[1]);
				::Constants::jetGeoVecAdd(v, v, this->Model->vTrans());
				vector = resNode->GetRotate();
				::Constants::jetGeoVecSet(v2, ::Constants::JET_DEG2RAD(vector.x), ::Constants::JET_DEG2RAD(vector.y), ::Constants::JET_DEG2RAD(vector.z), 1.0f);
				::Constants::jetGeoVecAdd(v2, v2, this->Model->vRotate());
				vector = resNode->GetScale();
				::Constants::jetGeoVecSet(v3, vector.x, vector.y, vector.z, 1.0f);
				m_PtkHnd[num]->Generate(0u, false, false, true, 0.0f, v, v2, v3);
			}
		}
	}
	UpdateMatrix();
	Matrix4x4 matrix = pMtxRotTrans();
	DummyNW4R::SetMtx(this->Model->pScnMdl(), matrix);
	DummyNW4R::SetScale(this->Model->pScnMdl(), this->Model->vScale()[0], this->Model->vScale()[1], this->Model->vScale()[2]);
	m_pTaskGateCookie = new cTaskGateCookie(this->Model->vTrans());
	CustomKingdomUtil::Assert(m_pTaskGateCookie != nullptr, "nullpo");
	ConnectChild(m_pTaskGateCookie);
	m_pBreakModel = new cTaskPBModel("BIN_GATE_2_MDL");
	m_pBreakModel->Model->RegistModel("BIN_GATE_2_MDL");
	m_pBreakModel->Model->SetLightSetIdx(3);
	m_pBreakModel->Model->SetFogIdx(0);
	::Constants::jetGeoVecCopy(m_pBreakModel->Model->vTrans(), this->Model->vTrans());
	m_pBreakModel->Model->vRotate()[1] = this->Model->vRotate()[1];
	m_pBreakModel->Model->UpdateMatrix();
	DummyNW4R::SetMtx(m_pBreakModel->Model->pScnMdl(), matrix);
	m_pBreakModel->Model->Invisible();
	return true;
}

bool cTaskBgGate::RegistData(sSaveDataGate& gate)
{
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	v[1] = ::Constants::JET_DEG2RAD(gate.rotY);
	if (std::getenv("MEGAMIX_GATELOG")) std::fprintf(stderr, "gate: kind %d cookies %d no %d trans (%.2f %.2f %.2f) rotY %.1f\n", (int)gate.kind, (int)gate.cookie, (int)gate.no, gate.trans[0], gate.trans[1], gate.trans[2], (double)gate.rotY);
	return RegistData(gate.kind, gate.cookie, gate.no, gate.trans, v);
}

bool cTaskBgGate::AreaCheck()
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector3 min = Vector3(-5.0f, 0.0f, 0.0f - m_fCheckRange);
	Vector3 max = Vector3(5.0f, 4.0f, m_fCheckRange);
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, this->Model->vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, this->Model->vRotate());
	Matrix4x4 m2;
	::Constants::jetGeoMtx44to34(m2, m);
	AABB target = AABB(min, max, false);
	target.Set(target, m2);
	Vector3 trans = Vector3(cTaskCharPacman2->vTrans()[0], cTaskCharPacman2->vTrans()[1], cTaskCharPacman2->vTrans()[2]);
	float _discard5;
	if (DummyNW4R::DistSqPoint3ToAABB(trans, target, _discard5) != 0.0f)
	{
		return false;
	}
	return true;
}

}
