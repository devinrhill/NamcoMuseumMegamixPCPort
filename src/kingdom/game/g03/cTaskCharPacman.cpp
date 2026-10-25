#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "GamePad.hpp"
#include "wii/Anim.hpp"
#include "kingdom/cKingdomModelBase.hpp"
#include "platform/devtools.hpp"
#include "DummyNW4R.hpp"

#include <iterator>
#include <limits>
#include <numbers>

#include "cJetTask.hpp"
#include "CommonUtil.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCollision.hpp"
#include "kingdom/game/g03/cTaskGoal.hpp"
#include "kingdom/game/g03/sSaveDataStart.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "RtpaParam.hpp"
#include <AudioListenerManager.hpp>
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <ReadFileUtil.hpp>
#include <ReleaseObjectManager.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g03
{

void cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV* csv, string name, int col, float& data)
{
	int num = csv->SearchRow(1, name);
	CustomKingdomUtil::Assert(0 <= num, "error");
	CustomKingdomUtil::Assert(csv->F32(num, col, data), "error");
}

void cTaskCharPacman::GET_PARAM_INT(cKingdomCSV* csv, string name, int col, int& data)
{
	int num = csv->SearchRow(1, name);
	CustomKingdomUtil::Assert(0 <= num, "error");
	CustomKingdomUtil::Assert(csv->S32(num, col, data), "error");
}

void cTaskCharPacman::GET_PARAM_UINT(cKingdomCSV* csv, string name, int col, uint& data)
{
	int num = csv->SearchRow(1, name);
	CustomKingdomUtil::Assert(0 <= num, "error");
	int _out;
	CustomKingdomUtil::Assert(csv->S32(num, col, _out), "error");
	data = (uint)_out;
}

Vector4& cTaskCharPacman::vTransOld2()
{
	return m_vTransOld2;
}

float cTaskCharPacman::vTransOld2(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vTransOld2[n];
}

Vector4& cTaskCharPacman::vForceMove()
{
	return m_vForceMove;
}

float cTaskCharPacman::vForceMove(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vForceMove[n];
}

Vector4& cTaskCharPacman::vLastFloorPos()
{
	return m_vLastFloorPos;
}

float cTaskCharPacman::vLastFloorPos(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_vLastFloorPos[n];
}

cTaskBgHit*& cTaskCharPacman::pHitObj(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_pHitObj[n];
}

Constants::sFace*& cTaskCharPacman::pHitFace(int n)
{
	CustomKingdomUtil::Assert(4 > n, "error");
	return m_pHitFace[n];
}

uint& cTaskCharPacman::nLife()
{
	return m_nLife;
}

uint& cTaskCharPacman::nHP()
{
	return m_nHP;
}

uint& cTaskCharPacman::nMaxHP()
{
	return m_nMaxHP;
}

uint& cTaskCharPacman::nEatEnemy()
{
	return m_nEatEnemy;
}

uint& cTaskCharPacman::nAloneCnt()
{
	return m_nAloneCnt;
}

uint& cTaskCharPacman::nEatComboCnt()
{
	return m_nEatComboCnt;
}

cTaskCharPacman::sParam* cTaskCharPacman::GetParam()
{
	return m_Param[nType()];
}

cTaskCharPacman::sParamCommon* cTaskCharPacman::GetParamCommon()
{
	return m_ParamCommon;
}

bool cTaskCharPacman::IsGoal()
{
	return m_bGoal;
}

std::vector<Constants::sHitParam*>* cTaskCharPacman::pHitList()
{
	return m_vecHitParam;
}

cTaskCharPacman::cTaskCharPacman()
	: s_AnmChrList{{"BIN_PAC_N_MOTION_01_CHA", "BIN_PAC_W_MOTION_01_CHA", "BIN_PAC_K_MOTION_01_CHA"}, {"BIN_PAC_N_MOTION_02_CHA", "BIN_PAC_W_MOTION_02_CHA", "BIN_PAC_K_MOTION_02_CHA"}, {"BIN_PAC_N_MOTION_03_CHA", "BIN_PAC_W_MOTION_03_CHA", "BIN_PAC_K_MOTION_03_CHA"}, {"BIN_PAC_N_MOTION_04_CHA", "BIN_PAC_W_MOTION_04_CHA", "BIN_PAC_K_MOTION_04_CHA"}, {"BIN_PAC_N_MOTION_05_CHA", "BIN_PAC_W_MOTION_05_CHA", "BIN_PAC_K_MOTION_05_CHA"}, {"BIN_PAC_N_MOTION_06_CHA", "BIN_PAC_W_MOTION_06_CHA", "BIN_PAC_K_MOTION_06_CHA"}, {"BIN_PAC_N_MOTION_07_CHA", "BIN_PAC_W_MOTION_07_CHA", "BIN_PAC_K_MOTION_07_CHA"}, {"BIN_PAC_N_MOTION_08_CHA", "BIN_PAC_W_MOTION_08_CHA", "BIN_PAC_K_MOTION_08_CHA"}, {"BIN_PAC_N_MOTION_09_CHA", "BIN_PAC_W_MOTION_09_CHA", "BIN_PAC_K_MOTION_09_CHA"}, {"BIN_PAC_N_MOTION_10_CHA", "BIN_PAC_W_MOTION_10_CHA", "BIN_PAC_K_MOTION_10_CHA"}}, m_PtkHndFlame(new cKingdomPtkHnd()), m_PtkHndChange(new cKingdomPtkHnd()), m_PtkHndSweat(new cKingdomPtkHnd()), m_PtkHndBrake(new cKingdomPtkHnd()), m_PtkHndPoson(new cKingdomPtkHnd()), m_PtkHndDash(new cKingdomPtkHnd()), m_PtkHndRipple(new cKingdomPtkHnd()), m_PtkHndWing(new cKingdomPtkHnd()), m_PtkHndPowerCookie(new cKingdomPtkHnd()), m_PtkHndWaterIn(new cKingdomPtkHnd())
{
	SetName("cTaskCharPacman");
	Init();
}

void cTaskCharPacman::Init()
{
	::Constants::jetGeoMtxIdentity(m_mRoll);
	::Constants::jetGeoMtxIdentity(m_mAnm);
	nKind() = cTaskChar::eKIND::eKIND_PACMAN;
	fRadius() = 0.38f;
	nModeState() = 0u;
	m_nDamageCnt = 0u;
	m_bGoal = false;
	m_fStickX = 0.0f;
	m_fStickY = 0.0f;
	::Constants::jetGeoVecUnit(m_vLastFloorPos);
	m_nAloneCnt = 0u;
	m_nDashCnt = 0u;
	m_nNoBoundCnt = 0u;
	m_nNoDashCnt = 0u;
	m_bBrakeAnmMax = false;
	::Constants::jetGeoVecUnit(m_vWaterForce);
	m_fWaterHeight = 0.0f;
	m_nEatComboCnt = 0u;
	m_nFreeFallCnt = 0u;
	m_bBrink = false;
	m_nEatEnemy = 0u;
	m_nHitKindFace = 0u;
	m_nAnmState = 0u;
	m_nAnmStateNext = 0u;
	m_nAnmCuurentIdx = 0u;
	m_nAnmChangeTime = 0u;
	m_nAnmKeepTime = 0u;
	m_nWaterCount = 0u;
	m_bGameOver = false;
	m_nEffWingCnt = 0u;
	m_nJumpCnt = 0u;
	m_bRollCheck = false;
	m_pHitFaceFloorOld = nullptr;
	m_pHitObjFloorOld = nullptr;
	for (int i = 0; i < (int)std::size(m_Param); i++)
	{
		m_Param[i] = new cTaskCharPacman::sParam();
	}
	m_ParamCommon = new cTaskCharPacman::sParamCommon();
	for (int j = 0; j < (int)std::size(m_sHitObjHistry); j++)
	{
		m_sHitObjHistry[j] = new cTaskCharPacman::BGHIT_HISTORY();
	}
	::Constants::jetGeoVecZero(m_vForceMove);
	::Constants::jetGeoVecZero(m_vForceMoveStock);
	::Constants::jetGeoVecUnit(m_vTransOld2);
	::Constants::jetGeoVecZero(m_vJumpVec);
	::Constants::jetGeoVecZero(m_vAfterMove);
	::Constants::jetGeoVecZero(m_vFloorDashVec);
	for (uint num = 0u; num < 4; num++)
	{
		m_pHitObj[num] = nullptr;
		m_pHitFace[num] = nullptr;
		m_vecHitParam[num].clear();
		m_vecHitParam[num].reserve(8);
	}
	InitParam();
	cKingdomModel* obj = CreateModel();
	obj->RegistModel("BIN_PAC_NORMAL_MDL");
	obj->IsDrawShadowDepth(true);
	obj->SetLightSetIdx(2);
	obj->SetFogIdx(2);
	cKingdomModel* obj2 = CreateModel(1u);
	obj2->RegistModel("BIN_PAC_WING_MDL");
	obj2->IsDrawShadowDepth(true);
	obj2->SetLightSetIdx(2);
	obj2->SetFogIdx(2);
	cKingdomModel* obj3 = CreateModel(2u);
	obj3->RegistModel("BIN_PAC_KNIGHT_MDL");
	obj3->IsDrawShadowDepth(true);
	obj3->SetLightSetIdx(2);
	obj3->SetFogIdx(2);
	int l;
	for (int k = 0; k < 3; k++)
	{
		string text;
		switch (k)
		{
		case 0:
			text = "BIN_PAC_NORMAL_MDL";
			break;
		case 1:
			text = "BIN_PAC_WING_MDL";
			break;
		case 2:
			text = "BIN_PAC_KNIGHT_MDL";
			break;
		}
		if (text.empty())
		{
			continue;
		}
		GameObject* original = kingdom::LoadModelObject(text);
		// TODO(unity): m_dummyAnimObj[k] = UnityEngine.Object.Instantiate(original);
		// TODO(unity): m_dummyAnimObj[k].name += "_dummy";
		// TODO(unity): m_dummyAnimation[k] = m_dummyAnimObj[k].GetComponent<Animation>();
		std::vector<Renderer*> componentsInChildren; // TODO(unity): componentsInChildren = m_dummyAnimObj[k].GetComponentsInChildren<Renderer>()
		for (l = 0; l < (int)componentsInChildren.size(); l++)
		{
			// TODO(unity): componentsInChildren[l].enabled = false;
		}
		std::vector<GameObject*> allChildren;
		CommonUtil::GetChildren(m_dummyAnimObj[k], allChildren);
		for (GameObject* item : allChildren)
		{
			if (nullptr != m_dummyAnimObjRootNode[k] && nullptr != m_dummyAnimObjBaseNode[k])
			{
				break;
			}
			if (false /* TODO(unity): item.name == "root" */)
			{
				// TODO(unity): m_dummyAnimObjRootNode[k] = item.transform;
			}
			if (false /* TODO(unity): item.name == "base" */)
			{
				// TODO(unity): m_dummyAnimObjBaseNode[k] = item.transform;
			}
		}
	}
	SetType(0u);
	SetState(0u);
	SetAnmState(0u, 0u, 0u);
	::Constants::jetGeoVecSet(vTrans(), 0.0f, 1.0f, 0.0f, 1.0f);
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (KingdomGlobal::PacnRollSys->IsRetry() && cAppG03PacnRollGame2->pTaskPoint()->nRetryIdx() >= 0)
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, cAppG03PacnRollGame2->pTaskPoint()->vRetryPos());
		v[1] += fRadius() * 1.3f;
		::Constants::jetGeoVecCopy(vTrans(), v);
		::Constants::jetGeoVecCopy(m_vLastFloorPos, v);
		float num2 = cAppG03PacnRollGame2->pTaskPoint()->vRetryRot(1u) + ::Constants::JET_DEG2RAD(180.0f);
		if (std::numeric_limits<float>::lowest() < Mathf::Abs(num2) && Mathf::Abs(num2) < std::numeric_limits<float>::max())
		{
			::Constants::jetGeoMtxRotY(m_mRoll, m_mRoll, num2);
		}
	}
	else
	{
		const std::vector<byte>& array = KingdomGlobal::KingSys->SearchData("BIN_START_SET_DAT");
		sSaveDataStart* outParam;
		ReadFileUtil::Convert<sSaveDataStart>(array, outParam, l);
		if (!array.empty())
		{
			::Constants::jetGeoVecCopy(vTrans(), outParam->trans);
			{ float o[3]; if (devtools::G03StartOverride(o)) { vTrans() = Vector4(o[0], o[1], o[2], 1.0f); } }   // devtools: --g03-start (not 1:1)
			::Constants::jetGeoVecCopy(m_vLastFloorPos, vTrans());
			float num3 = ::Constants::JET_DEG2RAD(outParam->rotY);
			if (std::numeric_limits<float>::lowest() < Mathf::Abs(num3) && Mathf::Abs(num3) < std::numeric_limits<float>::max())
			{
				::Constants::jetGeoMtxRotY(m_mRoll, m_mRoll, num3);
			}
		}
	}
	if (KingdomGlobal::PacnRollSys->IsRetry() || KingdomGlobal::PacnRollSys->nRound() == 25)
	{
		m_nLife = KingdomGlobal::PacnRollSys->pRetryData()->nLife;
		m_nMaxHP = KingdomGlobal::PacnRollSys->pRetryData()->nMaxHP;
	}
	else
	{
		m_nLife = KingdomGlobal::PacnRollSys->pSaveLoadMan()->nLife();
		if (devtools::G03Life() >= 0) m_nLife = devtools::G03Life();   // devtools: --g03-life (not 1:1)
		m_nMaxHP = KingdomGlobal::PacnRollSys->pSaveLoadMan()->nMaxHP();
	}
	if (7 < m_nMaxHP)
	{
		m_nMaxHP = 7u;
	}
	m_nHP = m_nMaxHP;
	if (cAppG03PacnRollGame2->pTaskCamera() != nullptr)
	{
		cAppG03PacnRollGame2->pTaskCamera()->RegistFirstPacPos(vTrans());
	}
	m_nFloorDashCnt = 0u;
	cHitChar* cHitChar2 = new cHitChar(this, 0);
	cHitChar2->FlagOff(8u);
	pvecHitChar().push_back(cHitChar2);
	for (uint num4 = 0u; num4 < 3; num4++)
	{
		m_AnmChrMiss[num4] = dynamic_cast<AnimationClip*>(ResourceDatabase::Get<Object>(s_AnmChrList[5][num4]));
		CustomKingdomUtil::Assert(m_AnmChrMiss[num4] != nullptr, "error");
		m_AnmChrBrakeOn[num4] = dynamic_cast<AnimationClip*>(ResourceDatabase::Get<Object>(s_AnmChrList[8][num4]));
		CustomKingdomUtil::Assert(m_AnmChrBrakeOn[num4] != nullptr, "error");
		m_AnmChrBrakeOff[num4] = dynamic_cast<AnimationClip*>(ResourceDatabase::Get<Object>(s_AnmChrList[9][num4]));
		CustomKingdomUtil::Assert(m_AnmChrBrakeOff[num4] != nullptr, "error");
	}
	m_fAnmCharMissFrame = 0.0f;
	m_fAnmCharBrakeOffFrame = 0.0f;
	m_fAnmCharBrakeOnFrame = 0.0f;
	AudioListenerManager::SetPosStalkTarget(pPBModel()->pScnMdl());
}

void cTaskCharPacman::Term()
{
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE090_PAC_DASH_DANSA);
	if (nullptr != m_SndHndMove)
	{
		SoundManager::get_SE()->Stop(m_SndHndMove);
	}
	if (nullptr != m_SndHndDash)
	{
		SoundManager::get_SE()->Stop(m_SndHndDash);
	}
	if (nullptr != m_SndHndInWater)
	{
		SoundManager::get_SE()->Stop(m_SndHndInWater);
	}
	if (nullptr != m_SndHndSwim)
	{
		SoundManager::get_SE()->Stop(m_SndHndSwim);
	}
	if (nullptr != m_SndHndGrass)
	{
		SoundManager::get_SE()->Stop(m_SndHndGrass);
	}
	for (int i = 0; i < (int)std::size(m_dummyAnimObj); i++)
	{
		if (nullptr != m_dummyAnimObj[i])
		{
			ReleaseObjectManager::Regist(m_dummyAnimObj[i]);
		}
	}
	cTaskChar::Term();
}

bool cTaskCharPacman::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (Constants::IS_PAC_PAUSE() || cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		return true;
	}
	int pad1No = InGamePadManager::GetPad1No();
	::Constants::jetGeoVecCopy(vTransOld(), vTrans());
	::Constants::jetGeoVecCopy(vTransOld2(), vTrans());
	SetStateOld(nState());
	::Constants::jetGeoVecUnit(vAccel());
	::Constants::jetGeoVecZero(vForceMove());
	::Constants::jetGeoVecZero(m_vForceMoveStock);
	OffState(32u);
	OffState(64u);
	OffState(1024u);
	OffState(2048u);
	::Constants::jetGeoVecUnit(m_vWaterForce);
	::Constants::jetGeoMtxIdentity(m_mAnm);
	m_fStickX = (m_fStickY = 0.0f);
	HitObjTick();
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch2_0u:
			nModeStep(n) = 1u;
			cAppG03PacnRollGame2->pTaskCamera()->SetCameraType(3u);
			goto _switch2_1u;
		case 1u:
		_switch2_1u:
			vMove()[1] -= GetParam()->fGravityFall;
			cAppG03PacnRollGame2->pTaskCollision()->PushBackChar(this);
			OnState(64u);
			if (cAppG03PacnRollGame2->IsStart())
			{
				NextMode(n, 1u);
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
			cAppG03PacnRollGame2->pTaskCamera()->SetCameraType(0u);
			break;
		}
	}
	else if (1 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch3_0u:
			nModeStep(n) = 1u;
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			if (pHitObj(1) != nullptr)
			{
				::Constants::jetGeoVecCopy(vLastFloorPos(), vTrans());
			}
			if (m_nHP == 0)
			{
				m_PtkHndPowerCookie->Detach(true);
				SetState(256u);
				NextMode(n, 4u);
			}
			else if (IsState(8192u))
			{
				NextMode(n, 3u);
			}
			else if (cAppG03PacnRollGame2->pTaskGoal()->IsGoal())
			{
				::Constants::jetGeoVecZero(vMove());
				m_PtkHndPowerCookie->Detach(true);
				SetState(0u);
				NextMode(n, 6u);
			}
			else if (cAppG03PacnRollGame2->IsClear())
			{
				DisposeEffect();
				Constants::PACN_APP()->StopIjikeSnd();
				SetState(0u);
				NextMode(n, 7u);
			}
			else if (IsState(16u))
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE092_PAC_SIGAMI);
				NextMode(n, 2u);
			}
			else if (CheckFallArea())
			{
				cAppG03PacnRollGame2->pTaskCamera()->SetCameraType(2u);
				SetState(384u);
				NextMode(n, 4u);
			}
			else
			{
				bool flag = false;
				if (CustomKingdomUtil::IF(m_vJumpVec[0]) || CustomKingdomUtil::IF(m_vJumpVec[1]) || CustomKingdomUtil::IF(m_vJumpVec[2]))
				{
					vMove()[0] += m_vJumpVec[0];
					vMove()[1] = m_vJumpVec[1];
					vMove()[2] += m_vJumpVec[2];
					::Constants::jetGeoVecZero(m_vJumpVec);
					OnState(16384u);
					flag = true;
				}
				if (CustomKingdomUtil::IF(m_vAfterMove[0]) || CustomKingdomUtil::IF(m_vAfterMove[1]) || CustomKingdomUtil::IF(m_vAfterMove[2]))
				{
					::Constants::jetGeoVecAdd(vMove(), vMove(), m_vAfterMove);
					::Constants::jetGeoVecZero(m_vAfterMove);
				}
				if (CustomKingdomUtil::IF(m_vFloorDashVec[0]) || CustomKingdomUtil::IF(m_vFloorDashVec[1]) || CustomKingdomUtil::IF(m_vFloorDashVec[2]))
				{
					::Constants::jetGeoVecAdd(vMove(), vMove(), m_vFloorDashVec);
					::Constants::jetGeoVecZero(m_vFloorDashVec);
				}
				if (m_nNoBoundCnt != 0)
				{
					m_nNoBoundCnt--;
				}
				if (m_nNoDashCnt != 0)
				{
					m_nNoDashCnt--;
				}
				if (IsState(8u))
				{
					pHitChar(0)->FlagOff(4u);
					if (++m_nDamageCnt >= m_ParamCommon->nDamageTime)
					{
						OffState(8u);
						pHitChar(0)->FlagOn(4u);
						m_nDamageCnt = 0u;
						m_bBrink = false;
					}
					else if (m_nDamageCnt % 2 == 0)
					{
						m_bBrink = true;
					}
					else
					{
						m_bBrink = false;
					}
				}
				Vector4 _discard4;
				::Constants::jetGeoVecCopy(_discard4, vMove());
				// WII (80165458): the idle count also requires pac-man to be standing still
				// (|vMove|^2 < 2.350989e-38f, tested after Control()); the Unity port dropped that test.
				bool bControl = Control();
				if (!bControl && pHitObj(1) != nullptr && !pHitObj(1)->IsMove() && ::Constants::jetGeoVecInnerProduct(vMove(), vMove()) < 2.350989e-38f && !IsState(32u) && !m_bRollCheck)
				{
					m_nAloneCnt++;
					if (m_nAloneCnt == m_ParamCommon->nWaitAnimeTime)
					{
						LookCameraRollMatrix();
						if (Random::Range(0, 2) == 0)
						{
							SetAnmState(11u, 0u, 0u);
						}
						else
						{
							SetAnmState(12u, 0u, 0u);
						}
					}
				}
				else
				{
					if (m_nAloneCnt >= m_ParamCommon->nWaitAnimeTime)
					{
						SetAnmState(0u, 0u, 0u);
					}
					m_nAloneCnt = 0u;
				}
				uint num = nState();
				switch (cAppG03PacnRollGame2->pTaskWater()->AreaCheck(vTrans(), fRadius(), m_vWaterForce, m_fWaterHeight))
				{
				case 1u:
					OnState(1024u);
					break;
				case 2u:
					OnState(2048u);
					break;
				}
				if (pHitObj(1) == nullptr && !IsState(3072u) && vMove()[1] >= 0.2f && IsState(1u))
				{
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE090_PAC_DASH_DANSA, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				}
				if (IsState(2048u) && !CustomKingdomUtil::IF(num & 0x800) && nType() != 2)
				{
					if (::Constants::fabs(vMove()[0]) >= 0.1f || ::Constants::fabs(vMove()[1]) >= 0.05f || ::Constants::fabs(vMove()[2]) >= 0.1f)
					{
						// WII (8016556c): the splash effect is spawned every frame (the port: 1 frame in 3)
						cAppG03PacnRollGame2->SetEffect(9u, vTrans(), Vector4());
						m_splashPtcCnt++;
						if (nullptr == m_SndHndInWater2)
						{
							m_nWaterCount = 0u;
							m_SndHndInWater2 = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE039_WATER);
						}
					}
				}
				else
				{
					m_splashPtcCnt = 0;
				}
				if (IsState(2048u) && !CustomKingdomUtil::IF(num & 0x800) && nType() == 2 && m_nHitKindFace != 5)
				{
					if (!m_PtkHndWaterIn->IsValid() && pHitFace(1) == nullptr)
					{
						cAppG03PacnRollGame2->SetEffect(9u, vTrans(), Vector4(), m_PtkHndWaterIn);
					}
					if (nullptr == m_SndHndInWater)
					{
						m_SndHndInWater = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE039_WATER);
					}
				}
				if (IsState(1024u) && nType() != 2)
				{
					m_nWaterCount++;
					if (!m_PtkHndRipple->IsValid())
					{
						cAppG03PacnRollGame2->SetEffect(8u, vTrans(), Vector4(), m_PtkHndRipple);
					}
					if ((::Constants::fabs(vMove()[0]) >= 0.05f || ::Constants::fabs(vMove()[1]) >= 0.2f || ::Constants::fabs(vMove()[2]) >= 0.05f) && (m_nWaterCount & 3) == 0)   // WII (80165638): every 4th frame; the port used % 6
					{
						cAppG03PacnRollGame2->SetEffect(8u, vTrans(), Vector4());
					}
					Vector4 v2;
					::Constants::jetGeoVecCopy(v2, vMove());
					v2[1] = 0.0f;
					float num2 = ::Constants::jetGeoVecLength(v2);
					if (nullptr == m_SndHndInWater && num2 >= 0.1f && nullptr == m_SndHndInWater2)
					{
						m_SndHndInWater = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE039_WATER);
					}
				}
				if (IsState(2048u))
				{
					if (nullptr == m_SndHndSwim && m_fWaterHeight - vTrans()[1] >= 0.7f && nType() == 2)
					{
						m_SndHndSwim = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE038_SUITYUU);
						cAppG03PacnRollGame2->SetEffect(11u, vTrans(), Vector4());
					}
				}
				else if (nullptr != m_SndHndSwim)
				{
					SoundManager::get_SE()->Stop(m_SndHndSwim);
				}
				if (nType() == 1 && pHitObj(1) == nullptr && !IsState(1024u) && !IsState(2048u))
				{
					if (vMove()[1] < 0.0f)
					{
						if (!m_PtkHndWing->IsValid())
						{
							cAppG03PacnRollGame2->SetEffect(7u, vTrans(), Vector4(), m_PtkHndWing);
						}
						m_nEffWingCnt++;
						if (m_nEffWingCnt % 3 == 0)
						{
							cAppG03PacnRollGame2->SetEffect(7u, vTrans(), Vector4());
						}
					}
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE052_KAKYUU, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
				}
				else
				{
					SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE052_KAKYUU);
				}
				if (pHitFace(1) != nullptr && nullptr == m_SndHndOnField && m_nJumpCnt >= 10)
				{
					switch (nType())
					{
					case 0u:
						m_SndHndOnField = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE061_TYAKUTI1);
						break;
					case 1u:
						m_SndHndOnField = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE062_TYAKUTI2);
						break;
					case 2u:
						m_SndHndOnField = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE063_TYAKUTI3);
						break;
					}
				}
				// DOL 0x80165b9c: dash = the remote swung faster than PAD_SWING_SPEED_DASH (get_pad1(0) + 0x44 = KPAD acc_speed); the C# used the A button
				if ((pHitObj(1) != nullptr || IsState(1024u)) && m_nNoDashCnt == 0 && m_ParamCommon->fPadSwingSpeedDash < GamePad::Get(0).accSpeed && !IsState(49u) && !IsStateOld(16u) && Constants::IsVecNormalize(vAccel()) && (CustomKingdomUtil::IF(m_fStickX) || CustomKingdomUtil::IF(m_fStickY)) && !IsStateOld(32768u))
				{
					OnState(1u);
					m_nDashCnt = 0u;
					if (m_nHitKindFace == 5)
					{
						m_SndHndDash = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE091_PAC_WATER_DASH);
					}
					else
					{
						m_SndHndDash = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE022_DASH);
						m_SndHndDash = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE093_PAC_DASH2);
					}
					SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE022_DASH);
					if (m_PtkHndDash->IsValid())
					{
						m_PtkHndDash->Detach(true);
					}
					cAppG03PacnRollGame2->SetEffect(0u, vTrans(), Vector4(), m_PtkHndDash);
					Vector4 v3;
					::Constants::jetGeoVecNormalize(v3, vAccel());
					if (Constants::IsVecNormalize(vMove()))
					{
						float t = ::Constants::jetGeoVecLength(vMove());
						::Constants::jetGeoVecScale(vMove(), v3, t);
					}
					else
					{
						::Constants::jetGeoVecCopy(vMove(), vAccel());
					}
					SetAnmState(2u, 5u, 0u);
				}
				if (IsState(1u))
				{
					if (!flag && (pHitObj(1) != nullptr || IsState(1024u)) && m_nDashCnt < GetParam()->nDashAccelTime)
					{
						Vector4 v4;
						::Constants::jetGeoVecCopy(v4, vMove());
						if (pHitObj(1) != nullptr)
						{
							Vector4 v5 = Vector4::zero;
							bool flag2 = true;
							Vector4 v6 = Vector4();
							Vector4 v7;
							if (m_pHitFaceFloorOld != nullptr)
							{
								::Constants::jetGeoVecTrans(v6, m_pHitObjFloorOld->pMtxRotate(), m_pHitFaceFloorOld->vNorm);
								if (IsState(32768u) && v6[1] <= 0.002f)
								{
									::Constants::jetGeoVecScale(v7, v6, -0.1f);
									v4[0] = v7[0];
									v4[2] = v7[2];
								}
							}
							Vector4 v8;
							::Constants::jetGeoVecTrans(v8, pHitObj(1)->pMtxRotate(), pHitFace(1)->vNorm);
							if (v8[1] < 0.002f)
							{
								flag2 = false;
							}
							else if (m_pHitFaceFloorOld != nullptr && m_pHitFaceFloorOld != pHitFace(1))
							{
								float num3 = ::Constants::jetGeoVecInnerProduct(v6, v8);
								if (0.6f > num3)
								{
									flag2 = false;
								}
							}
							if (flag2)
							{
								Vector4 v9;
								::Constants::jetGeoVecTrans(v9, pHitObj(1)->pMtxOld(), pHitFace(1)->vVrt[0]);
								Vector4 v10;
								::Constants::jetGeoVecTrans(v10, pHitObj(1)->pMtxRotateOld(), pHitFace(1)->vNorm);
								::Constants::jetGeoVecScale(v7, v10, 0.0f - fRadius());
								Vector4 v11;
								::Constants::jetGeoVecAdd(v11, vTrans(), v7);
								::Constants::jetGeoVecCopy(v7, v4);
								v7[1] = 0.0f;
								::Constants::jetGeoVecAdd(v11, v11, v7);
								::Constants::jetGeoVecSet(v5, 0.0f, -1.0f, 0.0f, 1.0f);
								Vector4 vOutCross;
								if (cTaskCollision::CollisionLinePlane(v11, v5, v9, v10, vOutCross))
								{
									v4[1] = vOutCross[1] - v11[1];
								}
							}
						}
						if (Constants::IsVecNormalize(v4))
						{
							float num4 = ::Constants::jetGeoVecLength(v4) * GetParam()->fDashForce;
							::Constants::jetGeoVecNormalize(v4, v4);
							if (num4 < GetParam()->fDashSpeedMin)
							{
								num4 = GetParam()->fDashSpeedMin;
							}
							::Constants::jetGeoVecScale(vMove(), v4, num4);
						}
						else if (Constants::IsVecNormalize(vAccel()))
						{
							float num4 = ::Constants::jetGeoVecLength(vAccel()) * GetParam()->fDashForce;
							::Constants::jetGeoVecNormalize(v4, vAccel());
							if (num4 < GetParam()->fDashSpeedMin)
							{
								num4 = GetParam()->fDashSpeedMin;
							}
							::Constants::jetGeoVecScale(vMove(), v4, num4);
						}
					}
					if (++m_nDashCnt > GetParam()->nDashTime || IsState(32u))
					{
						OffState(1u);
						SetAnmState(0u, 5u, 0u);
						m_PtkHndDash->Detach(true);
					}
					if (nType() == 2 && IsStateOld(2048u) && !IsState(2048u) && nullptr != m_SndHndDash)
					{
						SoundManager::get_SE()->Stop(m_SndHndDash, 5.0f);
					}
				}
				else
				{
					m_PtkHndDash->Detach(true);
				}
				if (IsState(2u) && (++m_nFloorDashCnt > GetParam()->nDashFloorTime || IsState(32u)))
				{
					OffState(2u);
				}
				if (!IsState(1u))
				{
					m_PtkHndDash->Detach(true);
				}
				AdjustSpeed();
				OffState(16384u);
				if (IsState(3072u))
				{
					WaterForce();
				}
				else
				{
					vMove()[1] -= GetParam()->fGravityFall;
				}
				BrakeAnime();
				cAppG03PacnRollGame2->pTaskCollision()->PushBackChar(this);
				OnState(64u);
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
			m_PtkHndFlame->Detach(true);
			m_PtkHndChange->Detach(true);
			m_PtkHndSweat->Detach(true);
			m_PtkHndBrake->Detach(true);
			m_PtkHndPoson->Detach(true);
			m_PtkHndDash->Detach(true);
			m_PtkHndWing->Detach(true);
			m_bBrink = false;
			m_nNoBoundCnt = 0u;
			m_nNoDashCnt = 0u;
			::Constants::jetGeoVecZero(m_vFloorDashVec);
			break;
		}
	}
	else if (2 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch7_0u:
			nModeStep(n) = 1u;
			if (m_PtkHndDash->IsValid())
			{
				m_PtkHndDash->Detach(true);
			}
			cAppG03PacnRollGame2->SetEffect(13u, vTrans(), Vector4(), m_PtkHndSweat);
			OffState(8u);
			pHitChar(0)->FlagOn(4u);
			SetAnmState(7u, 0u, 0u);
			goto _switch7_1u;
		case 1u:
		_switch7_1u:
			Control();
			if (!Hanging() || pHitObj(1) == nullptr)
			{
				NextMode(n, 1u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch7_2u;
		case 2u:
		_switch7_2u:
			nModeStep(n) = 0u;
			OffState(16u);
			m_PtkHndSweat->Detach(true);
			SetAnmState(0u, 0u, 0u);
			break;
		}
	}
	else if (3 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch8_0u:
			nModeStep(n) = 1u;
			::Constants::jetGeoVecCopy(vMove(), m_vJumpVec);
			::Constants::jetGeoVecZero(m_vJumpVec);
			goto _switch8_1u;
		case 1u:
		_switch8_1u:
			vMove()[1] -= GetParam()->fGravityFall;
			::Constants::jetGeoVecAdd(vTrans(), vTrans(), vMove());
			if (vMove()[1] <= 0.0f)
			{
				OffState(8192u);
				NextMode(n, 1u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch8_2u;
		case 2u:
		_switch8_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (4 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch9_0u:
			nModeStep(n) = 1u;
			// TODO(wii): cPacnRollLoopSndMan::StopAll() here (800ba454): every looping SE fades out over
			// 10 frames and no new loops start until the next round. The port has no loop-sound manager.
			if (nullptr != m_SndHndMove)
			{
				SoundManager::get_SE()->Stop(m_SndHndMove);
			}
			if (m_nLife != 0)
			{
				m_nLife--;
			}
			else
			{
				m_bGameOver = true;
			}
			m_fAnmCharMissFrame = 0.0f;
			SetAnmState(10u, 0u, 0u);
			goto _switch9_1u;
		case 1u:
		_switch9_1u:
			if (IsState(128u))
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE021_DAMAGE);
				NextMode(n, 5u);
			}
			else
			{
				// DOL 0x80166fa0 (state 4, step 1): the miss clip (m_AnmChrMiss[type], +0x67c) is sampled directly: translate of "root" at frame 0, scale + rotation of
				// "base" at the current frame (+0x6a0, then +1). The base TRANSLATION is dropped (the DOL zeroes it in the 4x4 it builds), unlike the Unity TRS.
				Matrix4x4 m;
				::Constants::jetGeoMtxIdentity(m);
				Matrix4x4 m2;
				::Constants::jetGeoMtxIdentity(m2);
				const std::shared_ptr<wii::ChrAnim> clip = cKingdomModelBase::ChrAnimOf(m_AnmChrMiss[nType()]);
				wii::BoneKey root, base;
				if (clip != nullptr)
				{
					clip->Sample("root", 0.0f, root);
					clip->Sample("base", m_fAnmCharMissFrame, base);
				}
				Vector4 v12 = Vector4(root.trans[0], root.trans[1], root.trans[2], 1.0f);
				::Constants::jetGeoMtxTrans(m, m, v12);
				m_fAnmCharMissFrame++;
				::Constants::jetGeoMtxScale(m, m, Vector4(base.scale[0], base.scale[1], base.scale[2], 1.0f));
				m2 = cKingdomModelBase::BoneRt(base, false);   // rotation only: the DOL zeroes the translation of the 4x4 it builds
				::Constants::jetGeoMtxIdentity(m_mAnm);
				::Constants::jetGeoMtxMult(m_mAnm, m_mAnm, m2);
				::Constants::jetGeoMtxMult(m_mAnm, m_mAnm, m);
				if (clip == nullptr || (float)clip->numFrame <= m_fAnmCharMissFrame)
				{
					OnState(512u);
					::Constants::jetGeoVecSet(vScale(), 0.0f, 0.0f, 0.0f, 1.0f);   // DOL 0x80167108: +0x220 = (0, 0, 0, 1): the model shrinks away
					NextMode(n, 8u);
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch9_2u;
		case 2u:
		_switch9_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (6 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch10_0u:
			nModeStep(n) = 1u;
			if (nullptr != m_SndHndMove)
			{
				SoundManager::get_SE()->Stop(m_SndHndMove);
			}
			goto _switch10_1u;
		case 1u:
		_switch10_1u:
			if (MoveGoalCenter())
			{
				::Constants::jetGeoVecZero(vMove());
				::Constants::jetGeoVecUnit(vRotate());
				NextMode(n, 7u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch10_2u;
		case 2u:
		_switch10_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (7 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch11_0u:
			nModeStep(n) = 1u;
			m_bGoal = true;
			cAppG03PacnRollGame2->SetEffect(19u, vTrans(), Vector4());
			if (cAppG03PacnRollGame2->IsBossStage())
			{
				vRotate()[1] = Get2GameraRot();
			}
			else
			{
				vRotate()[1] = cAppG03PacnRollGame2->pTaskGoal()->Model->vRotate()[1];
			}
			goto _switch11_1u;
		case 1u:
		_switch11_1u:
			if (ResetRollMatrix())
			{
				SetAnmState(9u, 0u, 0u);
				NextMode(n, 8u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch11_2u;
		case 2u:
		_switch11_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (5 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch12_0u:
			nModeStep(n) = 1u;
			m_nFreeFallCnt = 0u;
			// TODO(wii): cPacnRollLoopSndMan::StopAll() here (800ba454), as in the miss state above.
			if (nullptr != m_SndHndMove)
			{
				SoundManager::get_SE()->Stop(m_SndHndMove);
			}
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE023_RAKKA);
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE095_RAKKA);
			goto _switch12_1u;
		case 1u:
		_switch12_1u:
			if (++m_nFreeFallCnt > 40)
			{
				OnState(512u);
				NextMode(n, 8u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch12_2u;
		case 2u:
		_switch12_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (8 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch13_0u:
			nModeStep(n) = 1u;
			goto _switch13_1u;
		case 1u:
		_switch13_1u:
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch13_2u;
		case 2u:
		_switch13_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	if (IsState(128u))
	{
		if (cAppG03PacnRollGame2->pTaskConsRetry()->IsVisible())
		{
			Vector4& reference = vMove();
			float value = (vMove()[2] = 0.0f);
			reference[0] = value;
		}
		vMove()[1] -= GetParam()->fGravityFall;
		if (vMove()[1] < -0.6f)
		{
			vMove()[1] = -0.6f;
		}
		if (vTrans()[1] < -2000.0f)
		{
			vTrans()[1] = -2000.0f;
		}
		::Constants::jetGeoVecAdd(vTrans(), vTrans(), vMove());
	}
	if (pHitFace(1) != nullptr || IsState(1024u) || IsState(2048u))
	{
		m_nJumpCnt = 0u;
	}
	else
	{
		m_nJumpCnt++;
	}
	m_pHitFaceFloorOld = pHitFace(1);
	m_pHitObjFloorOld = pHitObj(1);
	ClearHitParam();
	OffState(32768u);
	return true;
}

void cTaskCharPacman::Draw(int nStatus, cJetTask* pParam)
{
	if (m_pHitObjFloorOld != nullptr && m_pHitObjFloorOld->IsState(256u) && pHitFace(1) == nullptr && m_pHitObjFloorOld->IsMove() && m_pHitObjFloorOld != nullptr && (m_pHitObjFloorOld->IsMove() || IsState(16u)) && !IsState(3072u))
	{
		Vector4 v;
		::Constants::jetGeoVecTrans(v, m_pHitObjFloorOld->pMtxInvOld(), vTransOld());
		::Constants::jetGeoVecTrans(v, m_pHitObjFloorOld->pMtx(), v);
		::Constants::jetGeoVecSub(v, v, vTransOld());
		::Constants::jetGeoVecScale(v, v, 0.45f);
		::Constants::jetGeoVecAdd(m_vAfterMove, m_vAfterMove, v);
	}
	Vector4 v2 = Vector4::zero;
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (!Constants::IS_PAC_PAUSE() && !IsGoal() && !IsMiss() && !IsState(16u) && (pHitObj(1) == nullptr || !IsState(32u)))
	{
		UpdateRollMatrix();
	}
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoMtxTrans(m, m, vTrans());
	::Constants::jetGeoMtxRotZYX(m, m, vRotate());
	Matrix4x4 m2;
	::Constants::jetGeoMtxMult(m2, m_mAnm, m_mRoll);
	::Constants::jetGeoMtxMult(m, m, m2);
	Matrix4x4 m3;
	::Constants::jetGeoMtx44to34(m3, m);
	// DOL 0x80167978: ScnObj::SetMtx(model, mtx) and then the model's scale fields (+0xdc..+0xe4) = vScale (this + 0x220): (1, 1, 1) normally, (0, 0, 0)
	// once the miss clip has played out (see Execute state 4), which is what makes Pac-Man vanish until the retry.
	for (uint num = 0u; num < 3; num++)
	{
		DummyNW4R::SetMtx(pPBModel(num)->pScnMdl(), m);
		DummyNW4R::SetScale(pPBModel(num)->pScnMdl(), vScale()[0], vScale()[1], vScale()[2]);
	}
	if (!m_bBrink || Constants::IS_PAC_PAUSE())
	{
		if (!pCurrentModel()->IsVisible())
		{
			pCurrentModel()->Visible();
		}
	}
	else if (pCurrentModel()->IsVisible())
	{
		pCurrentModel()->Invisible();
	}
	if (m_PtkHndFlame->IsValid() && m_PtkHndFlame->IsActive())
	{
		m_PtkHndFlame->vSetTrans(vTrans());
	}
	if (m_PtkHndChange->IsValid() && m_PtkHndChange->IsActive())
	{
		m_PtkHndChange->vSetTrans(vTrans());
	}
	if (m_PtkHndSweat->IsValid() && m_PtkHndSweat->IsActive())
	{
		::Constants::jetGeoVecSet(v2, 0.0f, fRadius(), 0.0f - fRadius(), 1.0f);
		::Constants::jetGeoVecTrans(v2, m_mRoll, v2);
		::Constants::jetGeoVecAdd(v2, vTrans(), v2);
		m_PtkHndSweat->vSetTrans(v2);
	}
	if (m_PtkHndDash->IsValid() && m_PtkHndDash->IsActive())
	{
		m_PtkHndDash->vSetTrans(vTrans());
	}
	if (m_PtkHndWing->IsValid())
	{
		m_PtkHndWing->IsActive();
	}
	if (pHitFace(1) != nullptr && m_PtkHndPoson->IsValid() && m_PtkHndPoson->IsActive() && pHitFace(1)->nKind == Constants::eFACE_KIND::eFACE_KIND_POIS)
	{
		m_PtkHndPoson->vSetTrans(vTrans());
	}
	if (m_PtkHndPowerCookie->IsValid() && m_PtkHndPowerCookie->IsActive())
	{
		Vector4 v3 = Vector4::zero;
		Vector4 v4 = Vector4();
		::Constants::jetGeoVecSub(v4, Constants::PACN_APP()->pTaskCamera()->vPos(), Constants::PACN_APP()->pTaskCamera()->vTarget());
		::Constants::jetGeoVecNormalize(v4, v4);
		::Constants::jetGeoVecScale(v4, v4, 0.65f);
		::Constants::jetGeoVecSet(v3, m3[0][3], m3[1][3], m3[2][3], 1.0f);
		::Constants::jetGeoVecAdd(v3, v3, v4);
		m_PtkHndPowerCookie->vSetTrans(v3);
	}
	if (Constants::IS_PAC_PAUSE() || cAppG03PacnRollGame2->IsNakajimaPause(this))
	{
		return;
	}
	if (m_nAnmChangeTime != 0)
	{
		uint num2 = ((!CustomKingdomUtil::IF(m_nAnmCuurentIdx)) ? 1u : 0u);
		if (--m_nAnmChangeTime == 0)
		{
			for (uint num3 = 0u; num3 < 3; num3++)
			{
				pModel(num3)->SetWeightAnmChr(m_nAnmCuurentIdx, 0.0f);
				pModel(num3)->SetWeightAnmChr(num2, 1.0f);
			}
			m_nAnmCuurentIdx = num2;
			m_nAnmChangeTime = 0u;
			m_nAnmState = m_nAnmStateNext;
		}
		else
		{
			float num4 = 1.0f / (float)m_nAnmChangeTime * (float)m_nAnmChangeTime;
			for (uint num5 = 0u; num5 < 3; num5++)
			{
				pModel(num5)->SetWeightAnmChr(m_nAnmCuurentIdx, num4);
				pModel(num5)->SetWeightAnmChr(num2, 1.0f - num4);
			}
		}
	}
	else if (m_nAnmKeepTime != 0 && --m_nAnmKeepTime == 0)
	{
		SetAnmState(0u, 5u, 0u);
		m_nAnmKeepTime = 0u;
	}
	if (!IsState(32u) || pHitObj(1) == nullptr)
	{
		UpdateFrame();
	}
}

void cTaskCharPacman::InitParam()
{
	cKingdomCSV* cKingdomCSV2 = new cKingdomCSV();
	cKingdomCSV2->RegistData("BIN_CSV_PACMAN_PARAM");
	string array[3] = {"PAC_NORMAL", "PAC_WING", "PAC_KNIGHT"};
	int num2;
	for (uint num = 0u; num < 3; num++)
	{
		num2 = cKingdomCSV2->SearchCol(0, array[num]);
		CustomKingdomUtil::Assert(0 <= num2, "error");
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "PAD_FORCE", num2, m_Param[num]->fPadForce);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "SPEED_MAX", num2, m_Param[num]->fSpeedMax);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "GRAVITY_FALL", num2, m_Param[num]->fGravityFall);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "GRAVITY_UP", num2, m_Param[num]->fGravityUp);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "GRAVITY_DOWN", num2, m_Param[num]->fGravityDown);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "REFLECT", num2, m_Param[num]->fReflect);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "FRICTION", num2, m_Param[num]->fFriction);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "FRICTION_BRAKE", num2, m_Param[num]->fFrictionBrake);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "AIR_RESIST", num2, m_Param[num]->fAirResist);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "AIR_RESIST_DASH", num2, m_Param[num]->fAirResistDash);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "DASH_FORCE", num2, m_Param[num]->fDashForce);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "DASH_SPEED_MAX", num2, m_Param[num]->fDashSpeedMax);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "DASH_SPEED_MIN", num2, m_Param[num]->fDashSpeedMin);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "DASH_BOUND", num2, m_Param[num]->fDashBound);
		cTaskCharPacman::GET_PARAM_UINT(cKingdomCSV2, "DASH_TIME", num2, m_Param[num]->nDashTime);
		cTaskCharPacman::GET_PARAM_UINT(cKingdomCSV2, "DASH_ACCEL_TIME", num2, m_Param[num]->nDashAccelTime);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "HANG_SPEED", num2, m_Param[num]->fHangSpeed);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "BOUND_SPEED", num2, m_Param[num]->fBoundSpeed);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "WATER_RESIST_XZ", num2, m_Param[num]->fWaterResistXZ);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "WATER_RESIST_Y", num2, m_Param[num]->fWaterResistY);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "WATER_LIFT", num2, m_Param[num]->fWaterLift);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "BOUND_FLOOR", num2, m_Param[num]->fBoundFloor);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "BOUND_WALL", num2, m_Param[num]->fBoundWall);
		cTaskCharPacman::GET_PARAM_UINT(cKingdomCSV2, "DASH_FLOOR_TIME", num2, m_Param[num]->nDashFloorTime);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "DASH_FLOOR_ACC", num2, m_Param[num]->fDashFloorAcc);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "DASH_FLOOR_SPEED_MAX", num2, m_Param[num]->fDashFloorSpeedMax);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "SPEED_MAX_AIR", num2, m_Param[num]->fSpeedMaxAir);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "SEESAW_MOMENT", num2, m_Param[num]->fSeesawMoment);
		cTaskCharPacman::GET_PARAM_FLT(cKingdomCSV2, "SEESAW_ACC_MAX", num2, m_Param[num]->fSeesawAccMax);
	}
	cKingdomCSV* obj = new cKingdomCSV();
	obj->RegistData("BIN_CSV_PACMAN_COMMON_PARAM");
	num2 = obj->SearchCol(0, "PAC_COMMON");
	CustomKingdomUtil::Assert(0 <= num2, "error");
	cTaskCharPacman::GET_PARAM_FLT(obj, "PAD_SWING_SPEED_DASH", num2, m_ParamCommon->fPadSwingSpeedDash);
	cTaskCharPacman::GET_PARAM_FLT(obj, "PAD_SWING_SPEED_ESCAPE_HANG", num2, m_ParamCommon->fPadSwingSpeedEscapeHang);
	cTaskCharPacman::GET_PARAM_FLT(obj, "HANG_HEIGHT", num2, m_ParamCommon->fHangHeight);
	cTaskCharPacman::GET_PARAM_FLT(obj, "VACUUM_RANGE", num2, m_ParamCommon->fVacuumRange);
	cTaskCharPacman::GET_PARAM_UINT(obj, "WAIT_ANIME_TIME", num2, m_ParamCommon->nWaitAnimeTime);
	cTaskCharPacman::GET_PARAM_UINT(obj, "DAMAGE_TIME", num2, m_ParamCommon->nDamageTime);
}

void cTaskCharPacman::CollisionFoceMove()
{
	cTaskBgHit* cTaskBgHit2 = pHitObj(1);
	if (cTaskBgHit2 == nullptr || (!cTaskBgHit2->IsMove() && !IsState(16u)) || IsState(3072u))
	{
		return;
	}
	Vector4 v;
	::Constants::jetGeoVecTrans(v, cTaskBgHit2->pMtxInvOld(), vTransOld());
	::Constants::jetGeoVecTrans(v, cTaskBgHit2->pMtx(), v);
	::Constants::jetGeoVecSub(vForceMove(), v, vTrans());
	::Constants::jetGeoVecCopy(m_vForceMoveStock, vForceMove());
	::Constants::jetGeoVecCopy(vTransOld2(), v);
	if (0.0f < cTaskBgHit2->fMoment() && !IsState(16u))
	{
		::Constants::jetGeoVecSub(v, vTransOld2(), vTrans());
		::Constants::jetGeoVecScale(v, v, cTaskBgHit2->fMoment());
		if (::Constants::jetGeoVecInnerProduct(v, v) > 1.6000002E-05f)
		{
			::Constants::jetGeoVecNormalize(v, v);
			::Constants::jetGeoVecScale(v, v, 0.004f);
		}
		::Constants::jetGeoVecAdd(vMove(), vMove(), v);
	}
}

void cTaskCharPacman::PostCollisionWall()
{
	if (IsState(16u) || IsState(8192u))
	{
		return;
	}
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	std::vector<Constants::sHitParam*>& list = m_vecHitParam[0];
	Constants::sHitParam* hitParam = nullptr;
	bool flag = false;
	bool flag2 = false;
	Constants::sHitParam* sHitParam = nullptr;
	Constants::sHitParam* sHitParam2 = nullptr;
	Vector4 v = Vector4();
	Vector4 v2 = Vector4::zero;
	::Constants::jetGeoVecUnit(v2);
	Vector4 v4;
	if (CustomKingdomUtil::IF((int)list.size()))
	{
		for (Constants::sHitParam* item : list)
		{
			if (nType() == 2 && IsState(1u) && item->pObj->IsState(1u))
			{
				Vector4 v3;
				::Constants::jetGeoVecTrans(v3, item->pObj->pMtx(), item->vPos);
				item->pObj->HitCallbackWall(item->pFace, item->vPos, v3);
				continue;
			}
			if (hitParam == nullptr && sHitParam2 == nullptr)
			{
				::Constants::jetGeoVecTrans(v, item->pObj->pMtxRotate(), item->vNorm);
				::Constants::jetGeoVecAdd(v2, v2, v);
			}
			else
			{
				::Constants::jetGeoVecTrans(v4, item->pObj->pMtxRotate(), item->vNorm);
				::Constants::jetGeoVecAdd(v2, v2, v4);
				if (0.0f > ::Constants::jetGeoVecInnerProduct(v, v4))
				{
					flag = true;
				}
			}
			if (KingdomGlobal::PacnRollSys->nRound() == 9 && item->pObj->IsState(64u) && (sHitParam2 == nullptr || item->fDist < sHitParam2->fDist))
			{
				sHitParam2 = item;
			}
			else if (hitParam == nullptr || item->fDist < hitParam->fDist)
			{
				hitParam = item;
			}
			if (KingdomGlobal::PacnRollSys->nRound() == 22 && item->pObj != hitParam->pObj && item->pObj->IsMove() && hitParam->pObj->IsMove() && CustomKingdomUtil::IF(item->pObj->nState() & 8) && CustomKingdomUtil::IF(hitParam->pObj->nState() & 8))
			{
				sHitParam = item;
				flag2 = true;
			}
		}
	}
	if (KingdomGlobal::PacnRollSys->nRound() == 22)
	{
		if (hitParam != nullptr)
		{
			bool flag3 = false;
			for (int i = 1; i < 8; i++)
			{
				cTaskBgHit* cTaskBgHit2 = HitObjGet(0, i);
				if (cTaskBgHit2 == nullptr)
				{
					continue;
				}
				if (cTaskBgHit2 != hitParam->pObj)
				{
					flag3 = true;
				}
				else if (cTaskBgHit2 == hitParam->pObj)
				{
					if (flag3)
					{
						flag2 = true;
					}
					break;
				}
			}
		}
	}
	else if (KingdomGlobal::PacnRollSys->nRound() == 9 && hitParam == nullptr)
	{
		hitParam = sHitParam2;
	}
	if (flag2)
	{
		::Constants::jetGeoVecZero(vMove());
		::Constants::jetGeoVecSet(m_vJumpVec, 0.0f, GetParam()->fSpeedMax, 0.0f, 1.0f);
		OnState(8192u);
	}
	else if (hitParam != nullptr)
	{
		Vector4 v5 = Vector4::zero;
		::Constants::jetGeoVecUnit(v5);
		Vector4 v6;
		::Constants::jetGeoVecZero(v6);
		HitObjRegist(0, hitParam->pObj);
		Vector4 v7;
		::Constants::jetGeoVecTrans(v7, hitParam->pObj->pMtxRotate(), hitParam->vNorm);
		Vector4 v8;
		::Constants::jetGeoVecTrans(v8, hitParam->pObj->pMtx(), hitParam->vPos);
		if (hitParam->pObj->IsMove())
		{
			::Constants::jetGeoVecTrans(v5, hitParam->pObj->pMtxRotate(), hitParam->vObjMove);
			if (hitParam->pObj != pHitObj(0) && hitParam->pObj != pHitObj(1))
			{
				::Constants::jetGeoVecCopy(v6, v5);
			}
		}
		::Constants::jetGeoVecScale(v4, v7, fRadius());
		::Constants::jetGeoVecAdd(vTrans(), v8, v4);
		Vector4 v9;
		::Constants::jetGeoVecCopy(v9, vMove());
		if (hitParam->pObj != pHitObj(1))
		{
			::Constants::jetGeoVecScale(v4, vForceMove(), 0.15f);
			::Constants::jetGeoVecAdd(v9, v9, v4);
		}
		::Constants::jetGeoVecZero(vForceMove());
		if (Constants::IsVecNormalize(v2))
		{
			::Constants::jetGeoVecNormalize(v7, v2);
		}
		float num = ::Constants::jetGeoVecInnerProduct(v9, v7);
		::Constants::jetGeoVecScale(v4, v7, num * 2.0f);
		Vector4 v10;
		::Constants::jetGeoVecSub(v10, v9, v4);
		if (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_BOUN && m_nNoBoundCnt == 0)
		{
			SetBoundWall(hitParam, v7);
		}
		if (flag && 0.2f > Mathf::Abs(num))
		{
			vMove()[0] = 0.0f;
			vMove()[2] = 0.0f;
		}
		else if ((pHitObj(0) == nullptr && 0.15f < Mathf::Abs(num)) || IsState(3u))
		{
			if (!IsState(2u))
			{
				::Constants::jetGeoVecScale(v10, v10, 0.6f);
			}
			if (KingdomGlobal::PacnRollSys->nRound() == 9 && hitParam->pObj->IsState(64u))
			{
				::Constants::jetGeoVecCopy(m_vAfterMove, v10);
				::Constants::jetGeoVecZero(vMove());
			}
			else
			{
				::Constants::jetGeoVecCopy(vMove(), v10);
			}
			if (IsState(1u) || IsOverSpeed(0.3f, Nullable<Vector4>(v9)))
			{
				cAppG03PacnRollGame2->SetEffect(1u, v8, Vector4());
				// WII (80169b..): SE089 plays when dashing into a wall unless it is already playing. The port
				// tested `!= null` on a handle that is never set, so the sound never played.
				if (IsState(1u) && nullptr == m_SndHndDash2)
				{
					m_SndHndDash2 = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE089_PAC_DASH);
				}
				if (!IsState(1u))
				{
					SetAnmState(1u, 0u, 60u);
				}
			}
		}
		else
		{
			bool flag4 = false;
			if (hitParam->pObj->IsMove())
			{
				Vector4 v11;
				::Constants::jetGeoVecCopy(v11, vAccel());
				v11[1] = 0.0f;
				Vector4 v12;
				::Constants::jetGeoVecCopy(v12, v6);
				v12[1] = 0.0f;
				if (Constants::IsVecNormalize(v11) && Constants::IsVecNormalize(v12))
				{
					::Constants::jetGeoVecNormalize(v11, v11);
					::Constants::jetGeoVecNormalize(v12, v12);
					if (::Constants::jetGeoVecInnerProduct(v11, v12) < -0.7f && ::Constants::jetGeoVecInnerProduct(v7, v11) > 0.7f)
					{
						::Constants::jetGeoVecSub(vMove(), vMove(), v6);
						flag4 = true;
					}
				}
			}
			if (!flag4)
			{
				::Constants::jetGeoVecSub(vMove(), vMove(), v6);
				::Constants::jetGeoVecCopy(v4, v9);
				v4[1] = 0.0f;
				::Constants::jetGeoVecAdd(v4, v4, v10);
				::Constants::jetGeoVecScale(v4, v4, 0.5f);
				if (KingdomGlobal::PacnRollSys->nRound() == 9 && hitParam->pObj->IsState(64u))
				{
					m_vAfterMove[0] = v4[0];
					m_vAfterMove[2] = v4[2];
				}
				else
				{
					vMove()[0] = v4[0];
					vMove()[2] = v4[2];
				}
			}
			if ((hitParam->pObj->IsMove() && hitParam->pObj->IsState(24u)) || (pHitObj(1) != nullptr && pHitObj(1)->IsState(24u)))
			{
				bool flag5 = false;
				if (Constants::IsVecNormalize(v5))
				{
					::Constants::jetGeoVecNormalize(v4, v5);
					if (-0.5f > ::Constants::jetGeoVecInnerProduct(v7, v4))
					{
						::Constants::jetGeoVecScale(v4, v5, -1.0f);
						::Constants::jetGeoVecCopy(vForceMove(), v4);
						flag5 = true;
					}
				}
				if (!flag5)
				{
					float t = ::Constants::jetGeoVecLength(v5) * 0.5f;
					::Constants::jetGeoVecScale(v4, v7, t);
					::Constants::jetGeoVecAdd(vMove(), vMove(), v4);
				}
			}
		}
		pHitObj(0) = hitParam->pObj;
		pHitFace(0) = hitParam->pFace;
		hitParam->pObj->HitCallbackWall(hitParam->pFace, hitParam->vPos, v8);
	}
	else
	{
		pHitObj(0) = nullptr;
		pHitFace(0) = nullptr;
	}
}

void cTaskCharPacman::PostCollisionFloor()
{
	if (IsState(16u))
	{
		return;
	}
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	std::vector<Constants::sHitParam*>& list = m_vecHitParam[1];
	Constants::sHitParam* hitParam = nullptr;
	Constants::sHitParam* sHitParam = nullptr;
	Constants::sHitParam* sHitParam2 = nullptr;
	Constants::sHitParam* sHitParam3 = nullptr;
	bool flag = true;
	if (CustomKingdomUtil::IF((int)list.size()))
	{
		for (Constants::sHitParam* item : list)
		{
			if (KingdomGlobal::PacnRollSys->nRound() == 15 && item->pObj->IsState(128u))
			{
				continue;
			}
			if (item->pFace == pHitFace(1))
			{
				sHitParam = item;
				if (!CustomKingdomUtil::IF(item->nFlag & 2))
				{
					flag = false;
				}
			}
			else if (item->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_JUMP)
			{
				sHitParam2 = item;
				flag = false;
			}
			else if (!CustomKingdomUtil::IF(item->nFlag & 2))
			{
				if (hitParam == nullptr || item->fDist < hitParam->fDist)
				{
					hitParam = item;
					flag = false;
				}
			}
			else if (sHitParam3 == nullptr || item->fDist < sHitParam3->fDist)
			{
				sHitParam3 = item;
			}
		}
		if (sHitParam2 != nullptr)
		{
			hitParam = sHitParam2;
		}
		else if (hitParam == nullptr)
		{
			hitParam = sHitParam2;
			if (hitParam == nullptr)
			{
				hitParam = sHitParam;
				if (hitParam == nullptr)
				{
					hitParam = sHitParam3;
				}
			}
		}
		if (hitParam != nullptr)
		{
			Vector4 v = Vector4::zero;
			HitObjRegist(1, hitParam->pObj);
			Vector4 v2;
			::Constants::jetGeoVecTrans(v2, hitParam->pObj->pMtx(), hitParam->vPos);
			Vector4 v3;
			::Constants::jetGeoVecTrans(v3, hitParam->pObj->pMtxRotate(), hitParam->vNorm);
			Vector4 v4;
			::Constants::jetGeoVecTrans(v4, hitParam->pObj->pMtx(), hitParam->vVrt[0]);
			Vector4 v5;
			::Constants::jetGeoVecSub(v5, v2, vTrans());
			Vector4 v6;
			if (::Constants::JET_ABS(v3[1]) < 0.001f && 0.0f > ::Constants::jetGeoVecInnerProduct(v5, vMove()))
			{
				::Constants::jetGeoVecCopy(v6, vMove());
			}
			else
			{
				::Constants::jetGeoVecScale(v, v3, fRadius());
				::Constants::jetGeoVecAdd(vTrans(), v2, v);
				::Constants::jetGeoVecCopy(v6, vMove());
				v6[1] = 0.0f;
			}
			if ((hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_JUMP || hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_MAGM) && m_nNoBoundCnt == 0)
			{
				SetBoundFloor(hitParam, v3);
			}
			else if (flag && hitParam->pFace->nKind != Constants::eFACE_KIND::eFACE_KIND_POIS && IsHang(hitParam))
			{
				bool num = IsState(4u);
				SetState(16u);
				if (num)
				{
					OnState(4u);
				}
				::Constants::jetGeoVecTrans(vTrans(), hitParam->pObj->pMtx(), hitParam->vEdgePos);
				::Constants::jetGeoVecCopy(m_vHangPos, hitParam->vEdgePos);
				::Constants::jetGeoVecCopy(m_vHangVec, hitParam->vEdgeVec);
				Hanging(hitParam->pObj);
			}
			else
			{
				cTaskBgHit* outDashObj = nullptr;
				Constants::sFace* sFace = cAppG03PacnRollGame2->pTaskCollision()->CheckDashFloor(this, outDashObj);
				if (sFace != nullptr)
				{
					if (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_DASH)
					{
						::Constants::jetGeoVecSet(v, 1.0f, 0.0f, 0.0f, 1.0f);
						::Constants::jetGeoVecRotY(v, v, sFace->fWork - hitParam->pObj->Model->vRotate()[1]);
						::Constants::jetGeoVecScale(m_vFloorDashVec, v, GetParam()->fDashFloorAcc);
						m_vFloorDashVec.x *= -1.0f;
					}
					else if (sFace->nKind == Constants::eFACE_KIND::eFACE_KIND_DALL)
					{
						if (Constants::IsVecNormalize(v6))
						{
							::Constants::jetGeoVecNormalize(v, v6);
							::Constants::jetGeoVecScale(m_vFloorDashVec, v, GetParam()->fDashFloorAcc);
						}
						else if (outDashObj->IsMove())
						{
							Vector4 v7 = Vector4::zero;
							Vector4 v8 = Vector4::zero;
							::Constants::jetGeoVecUnit(v7);
							::Constants::jetGeoVecUnit(v8);
							::Constants::jetGeoVecTrans(v7, outDashObj->pMtx(), v7);
							::Constants::jetGeoVecTrans(v8, outDashObj->pMtxOld(), v8);
							::Constants::jetGeoVecSub(v7, v7, v8);
							if (Constants::IsVecNormalize(v7))
							{
								::Constants::jetGeoVecNormalize(v7, v7);
								::Constants::jetGeoVecScale(m_vFloorDashVec, v7, GetParam()->fDashFloorAcc);
							}
						}
					}
					OnState(2u);
					if (nullptr == m_SndHndDash)
					{
						if (m_nHitKindFace == 5)
						{
							m_SndHndDash = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE091_PAC_WATER_DASH);
						}
						else
						{
							m_SndHndDash = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE022_DASH);
						}
					}
					else if (m_nHitKindFace != 5 && m_nFloorDashCnt >= 3)
					{
						if (nullptr != m_SndHndDash)
						{
							SoundManager::get_SE()->Stop(m_SndHndDash);
						}
						m_SndHndDash = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE022_DASH);
					}
					m_nFloorDashCnt = 0u;
				}
				if (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_POIS && !IsState(8u))
				{
					DamageProcess(1u, {}, 0.0f);
					cAppG03PacnRollGame2->SetEffect(10u, vTrans(), Vector4(), m_PtkHndPoson);
					SetAnmState(4u, 5u, 40u);
				}
				::Constants::jetGeoVecSet(v, 0.0f, 1.0f, 0.0f, 1.0f);
				float num2 = ::Constants::jetGeoVecInnerProduct(v3, v);
				if (num2 <= 0.002f)
				{
					if (vMove()[1] > 0.25f && m_pHitObjFloorOld != nullptr)
					{
						vMove()[1] += ::Constants::jetGeoVecLength(vMove()) * 0.018f;
						OnState(32768u);
					}
					if (Constants::IsVecNormalize(vMove()))
					{
						float num3 = GetParam()->fGravityDown;
						Vector4 v9;
						::Constants::jetGeoVecNormalize(v9, vMove());
						if (0.0f > ::Constants::jetGeoVecInnerProduct(v3, v9))
						{
							num3 = GetParam()->fGravityUp * 5.0f;
						}
						::Constants::jetGeoVecSet(v, 0.0f, -1.0f, 0.0f, 1.0f);
						::Constants::jetGeoVecAdd(v, v3, v);
						vMove()[0] += v[0] * num3;
						vMove()[2] += v[2] * num3;
						float t;
						if (IsState(32u))
						{
							t = GetParam()->fFrictionBrake;
							if (!IsStateOld(32u))
							{
								cAppG03PacnRollGame2->SetEffect(2u, vTransOld(), Vector4(), m_PtkHndBrake);
							}
						}
						else
						{
							t = GetParam()->fFriction * 1.5f;
						}
						::Constants::jetGeoVecScale(v, v9, t);
						::Constants::jetGeoVecSub(vMove(), vMove(), v);
					}
					if (IsState(1u))
					{
						if (vMove()[1] > 0.0f && vMove()[1] < 0.13f)
						{
							vMove()[1] = 0.13f;
						}
					}
					else
					{
						Vector4& reference = vMove();
						float value = (vMove()[2] = 0.0f);
						reference[0] = value;
					}
					if (!IsStateOld(32768u))
					{
						if (vMove()[1] > 0.3f)
						{
							vMove()[1] = 0.3f;
						}
					}
					else if (vMove()[1] > 0.3f)
					{
						vMove()[1] = 0.3f;
					}
				}
				else
				{
					if (v3[1] < 0.992f)
					{
						float num5 = GetParam()->fGravityDown;
						if (Constants::IsVecNormalize(v6))
						{
							::Constants::jetGeoVecNormalize(v, v6);
							if (0.0f > ::Constants::jetGeoVecInnerProduct(v3, v))
							{
								num5 = GetParam()->fGravityUp;
							}
						}
						::Constants::jetGeoVecSet(v, 0.0f, -1.0f, 0.0f, 1.0f);
						::Constants::jetGeoVecAdd(v, v3, v);
						v6[0] += v[0] * num5;
						v6[2] += v[2] * num5;
					}
					Vector4 v10 = Vector4::zero;
					Vector4 v11;
					::Constants::jetGeoVecAdd(v11, v2, v6);
					::Constants::jetGeoVecSet(v10, 0.0f, 1.0f, 0.0f, 1.0f);
					Vector4 vOutCross;
					if (cTaskCollision::CollisionLinePlane(v11, v10, v4, v3, vOutCross))
					{
						v6[1] = vOutCross[1] - v11[1];
					}
					else
					{
						v6[1] = vMove()[1];
					}
					if (num2 < 0.05f && v6[1] > 0.3f)
					{
						v6[1] = 0.3f;
					}
					float sp = 0.0f;
					if (IsState(32u))
					{
						sp = GetParam()->fFrictionBrake;
						if (!IsStateOld(32u))
						{
							::Constants::jetGeoVecScale(v, v3, 0.1f);
							::Constants::jetGeoVecAdd(v, v2, v);
							Vector4 v12;
							::Constants::jetGeoVecCopy(v12, vMove());
							v12[1] = 0.0f;
							if (::Constants::jetGeoVecLength(v12) > 0.1f)
							{
								if (!IsState(2048u) && !IsState(1024u))
								{
									cAppG03PacnRollGame2->SetEffect(2u, v, Vector4(), m_PtkHndBrake);
								}
								SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE024_BRAKE);
							}
						}
					}
					else if (!IsState(2048u))
					{
						sp = GetParam()->fFriction;
						if (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_GRAS)
						{
							sp *= 2.0f;
						}
					}
					if (!IsOverSpeed(sp, Nullable<Vector4>(v6)) || (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_WATE && !Constants::IsVecNormalize(vAccel()) && !IsOverSpeed(0.1f, Nullable<Vector4>(v6))))
					{
						::Constants::jetGeoVecZero(v6);
					}
					else if (Constants::IsVecNormalize(v6))
					{
						::Constants::jetGeoVecNormalize(v, v6);
						::Constants::jetGeoVecScale(v, v, sp);
						::Constants::jetGeoVecSub(v6, v6, v);
					}
					::Constants::jetGeoVecCopy(vMove(), v6);
				}
				if (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_GRAS)
				{
					if (::Constants::fabs(vMove()[0]) < 0.1f && ::Constants::fabs(vMove()[2]) < 0.1f)
					{
						if (nullptr != m_SndHndGrass)
						{
							SoundManager::get_SE()->Stop(m_SndHndGrass);
						}
					}
					else if (nullptr == m_SndHndGrass)
					{
						m_SndHndGrass = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE048_GRASS);
						cAppG03PacnRollGame2->SetEffect(35u, vTrans(), Vector4());
					}
				}
				else if (nullptr != m_SndHndGrass)
				{
					SoundManager::get_SE()->Stop(m_SndHndGrass);
				}
			}
			pHitObj(1) = hitParam->pObj;
			pHitFace(1) = hitParam->pFace;
			hitParam->pObj->HitCallbackFloor(hitParam->pFace, hitParam->vPos, v2);
			if (m_PtkHndBrake->IsValid() && m_PtkHndBrake->IsActive())
			{
				::Constants::jetGeoVecScale(v, v3, 0.1f);
				::Constants::jetGeoVecAdd(v, v2, v);
				m_PtkHndBrake->vSetTrans(v);
			}
			m_nHitKindFace = (uint)hitParam->pFace->nKind;
			return;
		}
	}
	m_nHitKindFace = 20u;
	pHitObj(1) = nullptr;
	pHitFace(1) = nullptr;
}

void cTaskCharPacman::PostCollisionWallFinish()
{
	if (IsState(16u))
	{
		return;
	}
	std::vector<Constants::sHitParam*>& list = m_vecHitParam[0];
	Constants::sHitParam* hitParam = nullptr;
	Constants::sHitParam* sHitParam = nullptr;
	if (!CustomKingdomUtil::IF((int)list.size()))
	{
		return;
	}
	for (Constants::sHitParam* item : list)
	{
		if (KingdomGlobal::PacnRollSys->nRound() == 9 && item->pObj->IsState(64u) && (sHitParam == nullptr || item->fDist < sHitParam->fDist))
		{
			sHitParam = item;
		}
		else if (KingdomGlobal::PacnRollSys->nRound() == 5 && item->pObj->IsState(2u) && (sHitParam == nullptr || item->fDist < sHitParam->fDist))
		{
			sHitParam = item;
		}
		else if (hitParam == nullptr || item->fDist < hitParam->fDist)
		{
			hitParam = item;
		}
	}
	if ((KingdomGlobal::PacnRollSys->nRound() == 9 || KingdomGlobal::PacnRollSys->nRound() == 5) && hitParam == nullptr)
	{
		hitParam = sHitParam;
	}
	if (hitParam == nullptr)
	{
		return;
	}
	Vector4 v;
	::Constants::jetGeoVecTrans(v, hitParam->pObj->pMtx(), hitParam->vPos);
	Vector4 v2;
	::Constants::jetGeoVecTrans(v2, hitParam->pObj->pMtxRotate(), hitParam->vNorm);
	Vector4 v3;
	::Constants::jetGeoVecScale(v3, v2, fRadius());
	if (hitParam->pObj->IsMove())
	{
		::Constants::jetGeoVecAdd(vTrans(), v, v3);
	}
	else
	{
		::Constants::jetGeoVecAdd(v3, v, v3);
		Vector4 v4;
		::Constants::jetGeoVecSub(v4, v3, vTrans());
		Vector4 v5;
		if (pHitObj(1) == nullptr || pHitObj(1) != hitParam->pObj)
		{
			::Constants::jetGeoVecAdd(v5, vMove(), m_vForceMoveStock);
		}
		else
		{
			::Constants::jetGeoVecCopy(v5, vMove());
		}
		if (0.0f >= ::Constants::jetGeoVecInnerProduct(v4, v5))
		{
			::Constants::jetGeoVecCopy(vTrans(), v3);
			vTrans()[3] = 1.0f;
		}
	}
	if (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_BOUN && m_nNoBoundCnt == 0)
	{
		SetBoundWall(hitParam, v2);
	}
	pHitObj(0) = hitParam->pObj;
	pHitFace(0) = hitParam->pFace;
}

void cTaskCharPacman::PostCollisionFloorFinish()
{
	if (IsState(16u))
	{
		return;
	}
	std::vector<Constants::sHitParam*>& list = m_vecHitParam[1];
	Constants::sHitParam* hitParam = nullptr;
	Constants::sHitParam* sHitParam = nullptr;
	Constants::sHitParam* sHitParam2 = nullptr;
	Constants::sHitParam* sHitParam3 = nullptr;
	if (!CustomKingdomUtil::IF((int)list.size()))
	{
		return;
	}
	for (Constants::sHitParam* item : list)
	{
		if (item->pFace == pHitFace(1))
		{
			sHitParam = item;
		}
		else if (item->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_JUMP)
		{
			sHitParam3 = item;
		}
		else if (!CustomKingdomUtil::IF(item->nFlag & 2))
		{
			if (hitParam != nullptr && !(item->fDist < hitParam->fDist))
			{
				continue;
			}
			if (pHitFace(0) != nullptr && pHitObj(0) == item->pObj)
			{
				if (-0.1f <= ::Constants::jetGeoVecInnerProduct(pHitFace(0)->vNorm, item->vNorm))
				{
					hitParam = item;
				}
			}
			else
			{
				hitParam = item;
			}
		}
		else
		{
			if (sHitParam2 != nullptr && !(item->fDist < sHitParam2->fDist))
			{
				continue;
			}
			if (pHitFace(0) != nullptr && pHitObj(0) == item->pObj)
			{
				if (-0.1f <= ::Constants::jetGeoVecInnerProduct(pHitFace(0)->vNorm, item->vNorm))
				{
					sHitParam2 = item;
				}
			}
			else
			{
				sHitParam2 = item;
			}
		}
	}
	if (sHitParam3 != nullptr)
	{
		hitParam = sHitParam3;
	}
	else if (hitParam == nullptr)
	{
		hitParam = sHitParam;
		if (hitParam == nullptr)
		{
			hitParam = sHitParam2;
		}
	}
	if (hitParam != nullptr)
	{
		Vector4 v;
		::Constants::jetGeoVecTrans(v, hitParam->pObj->pMtx(), hitParam->vPos);
		Vector4 v2;
		::Constants::jetGeoVecTrans(v2, hitParam->pObj->pMtxRotate(), hitParam->vNorm);
		Vector4 v3;
		::Constants::jetGeoVecScale(v3, v2, fRadius());
		::Constants::jetGeoVecAdd(vTrans(), v, v3);
		if (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_JUMP && m_nNoBoundCnt == 0)
		{
			SetBoundFloor(hitParam, v2);
		}
		pHitObj(1) = hitParam->pObj;
		pHitFace(1) = hitParam->pFace;
	}
}

void cTaskCharPacman::SetBoundWall(Constants::sHitParam* const& hitParam, const Vector4& vNorm)
{
	Vector4 v;
	::Constants::jetGeoVecScale(v, vNorm, GetParam()->fBoundFloor);
	if (hitParam->pObj->IsMove())
	{
		Vector4 v2;
		::Constants::jetGeoVecZero(v2);
		::Constants::jetGeoVecTrans(v2, hitParam->pObj->pMtxInvOld(), vTransOld());
		::Constants::jetGeoVecTrans(v2, hitParam->pObj->pMtx(), v2);
		::Constants::jetGeoVecSub(v2, v2, vTrans());
		::Constants::jetGeoVecAdd(v, v, v2);
	}
	SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE031_JUMP, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
	SetAnmState(6u, 5u, 40u);
	m_nNoBoundCnt = 6u;
	::Constants::jetGeoVecSet(m_vJumpVec, v[0], v[1], v[2], 1.0f);
}

void cTaskCharPacman::SetBoundFloor(Constants::sHitParam* const& hitParam, const Vector4& vNorm)
{
	CustomKingdomUtil::Assert(hitParam != nullptr, "nullpo");
	CustomKingdomUtil::Assert("nullpo");
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if ((hitParam->pFace->nKind != Constants::eFACE_KIND::eFACE_KIND_JUMP && hitParam->pFace->nKind != Constants::eFACE_KIND::eFACE_KIND_MAGM) || m_nNoBoundCnt != 0)
	{
		return;
	}
	Vector4 v;
	::Constants::jetGeoVecScale(v, vNorm, GetParam()->fBoundFloor);
	::Constants::jetGeoVecAdd(m_vJumpVec, m_vJumpVec, vForceMove());
	if (IsState(1u))
	{
		v[1] *= GetParam()->fDashBound;
	}
	::Constants::jetGeoVecSet(m_vJumpVec, v[0], v[1], v[2], 1.0f);
	if (!IsState(8u) && hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_MAGM)
	{
		cAppG03PacnRollGame2->SetEffect(14u, vTrans(), Vector4(), m_PtkHndFlame);
		DamageProcess(1u, {}, 0.0f);
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE021_DAMAGE);
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE049_YOUGAN);
		SetAnmState(4u, 5u, 40u);
	}
	else
	{
		SetAnmState(6u, 5u, 40u);
		if (nType() == 1)
		{
			cAppG03PacnRollGame2->SetEffect(7u, vTrans(), Vector4(), m_PtkHndWing);
		}
	}
	if (hitParam->pFace->nKind == Constants::eFACE_KIND::eFACE_KIND_JUMP)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE031_JUMP);
	}
	m_nNoBoundCnt = 20u;
}

void cTaskCharPacman::WaterForce()
{
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	v[0] = vMove()[0] * GetParam()->fWaterResistXZ;
	v[1] = vMove()[1] * GetParam()->fWaterResistY;
	v[2] = vMove()[2] * GetParam()->fWaterResistXZ;
	v[3] = 1.0f;
	::Constants::jetGeoVecSub(vMove(), vMove(), v);
	if (nType() == 2)
	{
		vMove()[1] = -0.06f;
	}
	else if (IsState(2048u))
	{
		vMove()[1] += GetParam()->fWaterLift;
	}
	else if (vTrans()[1] >= m_fWaterHeight && vTrans()[1] + vMove()[1] >= m_fWaterHeight)
	{
		if (vMove()[1] >= 0.0f || vMove()[1] + GetParam()->fWaterLift > 0.0f)
		{
			vMove()[1] = 0.0f;
			vTrans()[1] = m_fWaterHeight + fRadius();
		}
		else
		{
			vMove()[1] += GetParam()->fWaterLift;
		}
	}
	::Constants::jetGeoVecAdd(vMove(), vMove(), m_vWaterForce);
}

bool cTaskCharPacman::IsHang(Constants::sHitParam* hitParam)
{
	CustomKingdomUtil::Assert(hitParam != nullptr, "nullpo");
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (pHitObj(1) != nullptr && !IsState(12161u) && hitParam->nFlag == 2 && 0 <= hitParam->nEdgeVrtIdx && CustomKingdomUtil::IF((long)hitParam->pFace->nHangFlag & (long)(1 << hitParam->nEdgeVrtIdx)) && !IsOverSpeed(GetParam()->fHangSpeed, Nullable<Vector4>(vMove())) && !cAppG03PacnRollGame2->pTaskGoal()->IsGoal() && (pHitObj(0) == nullptr || !pHitObj(0)->IsState(64u)))
	{
		Constants::sFace* outFace = new Constants::sFace();
		Vector4 v;
		::Constants::jetGeoVecTrans(v, pHitObj(1)->pMtxRotate(), hitParam->vEdgeVec);
		::Constants::jetGeoVecSub(v, vTrans(), v);
		float outDist;
		Vector4 _discard14;
		cTaskBgHit* _discard15;
		if (!cAppG03PacnRollGame2->pTaskCollision()->CheckBottom(v, outDist, _discard14, outFace, _discard15))
		{
			return true;
		}
		if (outFace->nKind == Constants::eFACE_KIND::eFACE_KIND_WALL || outFace->nKind == Constants::eFACE_KIND::eFACE_KIND_WAXX || outFace->nKind == Constants::eFACE_KIND::eFACE_KIND_BOUN || m_ParamCommon->fHangHeight * m_ParamCommon->fHangHeight <= outDist)
		{
			return true;
		}
	}
	return false;
}

bool cTaskCharPacman::Hanging(bool flg)
{
	return Hanging(flg, pHitObj(1));
}

bool cTaskCharPacman::Hanging(bool flg, cTaskBgHit* hitObjFloor)
{
	CustomKingdomUtil::Assert(hitObjFloor != nullptr, "nullpo");
	InGamePadManager::GetPad1No();
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	::Constants::jetGeoVecZero(vMove());
	::Constants::jetGeoMtxIdentity(m_mRoll);
	if (hitObjFloor != nullptr)
	{
		Vector4 v2;
		::Constants::jetGeoVecTrans(v2, hitObjFloor->pMtxRotate(), m_vHangVec);
		if ((std::numeric_limits<float>::lowest() < Mathf::Abs(v2[0]) && Mathf::Abs(v2[0]) < std::numeric_limits<float>::max()) || (std::numeric_limits<float>::lowest() < Mathf::Abs(v2[2]) && Mathf::Abs(v2[2]) < std::numeric_limits<float>::max()))
		{
			v[1] = Mathf::Atan2(v2[0], v2[2]);
			if (!(std::numeric_limits<float>::lowest() < Mathf::Abs(v[1])) || !(Mathf::Abs(v[1]) < std::numeric_limits<float>::max()))
			{
				v[1] = 0.0f;
			}
		}
		if (std::numeric_limits<float>::lowest() < Mathf::Abs(v2[1]) && Mathf::Abs(v2[1]) < std::numeric_limits<float>::max())
		{
			v[0] = Mathf::Atan2(0.0f - v2[1], 1.0f);
			if (!(std::numeric_limits<float>::lowest() < Mathf::Abs(v[0])) || !(Mathf::Abs(v[0]) < std::numeric_limits<float>::max()))
			{
				v[0] = 0.0f;
			}
		}
		Vector4 v3;
		::Constants::jetGeoVecTrans(v3, hitObjFloor->pMtxRotate(), pHitFace(1)->vNorm);
		Vector4 v4;
		if (Constants::IsVecNormalize(v2))
		{
			::Constants::jetGeoVecNormalize(v4, v2);
			::Constants::jetGeoVecOuterProduct(v4, v4, v3);
			if (std::numeric_limits<float>::lowest() < Mathf::Abs(v4[1]) && Mathf::Abs(v4[1]) < std::numeric_limits<float>::max())
			{
				v[2] = Mathf::Atan2(0.0f - v4[1], 1.0f);
				if (!(std::numeric_limits<float>::lowest() < Mathf::Abs(v[2])) || !(Mathf::Abs(v[2]) < std::numeric_limits<float>::max()))
				{
					v[2] = 0.0f;
				}
			}
		}
		::Constants::jetGeoMtxRotY(m_mRoll, m_mRoll, v[1]);
		::Constants::jetGeoMtxRotX(m_mRoll, m_mRoll, v[0]);
		::Constants::jetGeoMtxRotZ(m_mRoll, m_mRoll, v[2]);
		::Constants::jetGeoVecTrans(vTrans(), hitObjFloor->pMtx(), m_vHangPos);
		::Constants::jetGeoVecScale(v4, v2, 0.2f);
		::Constants::jetGeoVecSub(vTrans(), vTrans(), v4);
		if (flg)
		{
			// DOL 0x8016c690: no swing (acc_speed <= PAD_SWING_SPEED_ESCAPE_HANG): let go only in the states 0xc20; a swing lets go at once, pushed away from the
			// wall (v2 . vAccel <= 0) or along it (the C# only had the first branch)
			bool flag = true;
			if (GamePad::Get(0).accSpeed <= m_ParamCommon->fPadSwingSpeedEscapeHang)
			{
				if (IsState(3104u))
				{
					::Constants::jetGeoVecSet(m_vAfterMove, (0.0f - v2[0]) * 0.04f, 0.2f, (0.0f - v2[2]) * 0.04f, 1.0f);
					flag = false;
				}
			}
			else
			{
				const float dot = v2[0] * vAccel()[0] + v2[1] * vAccel()[1] + v2[2] * vAccel()[2];
				const float k = dot <= 0.0f ? -0.04f : 0.04f;
				::Constants::jetGeoVecSet(m_vAfterMove, v2[0] * k, 0.2f, v2[2] * k, 1.0f);
				flag = false;
			}
			if (!flag)
			{
				vTrans()[1] += fRadius();
				m_nNoDashCnt = 20u;
				OffState(16u);
				return false;
			}
		}
	}
	return true;
}

void cTaskCharPacman::SetType(const uint& type)
{
	CustomKingdomUtil::Assert(3 > type, "error");
	for (uint num = 0u; num < 3; num++)
	{
		pPBModel(num)->Invisible();
	}
	nType() = type;
	pCurrentModel()->Visible();
}

void cTaskCharPacman::SetAnmState(const uint& state, const uint& time, const uint& keep)
{
	switch (state)
	{
	case 0u:
		SetAnmChr(0u, s_AnmChrList[0][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_01_TPA", 0u);
		break;
	case 1u:
		SetAnmChr(1u, s_AnmChrList[0][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_04_TPA", 0u);
		break;
	case 2u:
		SetAnmChr(2u, s_AnmChrList[0][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_03_TPA", 0u);
		break;
	case 3u:
		SetAnmChr(3u, s_AnmChrList[0][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_03_TPA", 0u);
		break;
	case 4u:
		SetAnmChr(4u, s_AnmChrList[0][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_04_TPA", 0u);
		break;
	case 5u:
		SetAnmChr(5u, s_AnmChrList[1][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_02_TPA", 0u);
		break;
	case 6u:
		SetAnmChr(6u, s_AnmChrList[0][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_06_TPA", 0u);
		break;
	case 7u:
		SetAnmChr(7u, s_AnmChrList[4][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_02_TPA", 0u);
		break;
	case 8u:
		SetAnmChr(8u, s_AnmChrList[4][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_03_TPA", 0u);
		break;
	case 9u:
		SetAnmChr(9u, s_AnmChrList[3][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_05_TPA", 0u);
		break;
	case 10u:
		SetAnm("BIN_PAC_FACE_06_TPA", 0u);
		break;
	case 11u:
		SetAnmChr(11u, s_AnmChrList[6][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_02_TPA", 0u);
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE088_AKUBI);
		break;
	case 12u:
		SetAnmChr(12u, s_AnmChrList[7][nType()], time, keep);
		SetAnm("BIN_PAC_FACE_02_TPA", 0u);
		break;
	case 13u:
		SetAnm("BIN_PAC_FACE_02_TPA", 0u);
		break;
	case 14u:
		SetAnm("BIN_PAC_FACE_02_TPA", 0u);
		break;
	}
}

void cTaskCharPacman::SetAnm(string anime, const uint& idx)
{
	RtpaParam* rtpaParam = ResourceDatabase::Get<RtpaParam>(anime);
	CustomKingdomUtil::Assert(nullptr != rtpaParam, "nullpo");
	for (uint num = 0u; num < 3; num++)
	{
		pModel(num)->RegistAnm(idx, rtpaParam);
	}
}

void cTaskCharPacman::SetAnmChr(const uint& kind, string anime, const uint& time, const uint& keep)
{
	AnimationClip* animationClip = ResourceDatabase::Get<AnimationClip>(anime);
	CustomKingdomUtil::Assert(nullptr != animationClip, "nullpo");
	m_nAnmKeepTime = keep;
	if (time == 0)
	{
		m_nAnmState = kind;
		m_nAnmCuurentIdx = 0u;
		m_nAnmChangeTime = 0u;
		if (!CheckRepetition(animationClip, 0u))
		{
			for (uint num = 0u; num < 3; num++)
			{
				pModel(num)->RegistAnm(0u, animationClip);
				pModel(num)->SetWeightAnmChr(m_nAnmCuurentIdx, 1.0f);
			}
		}
		return;
	}
	uint num2 = ((!CustomKingdomUtil::IF(m_nAnmCuurentIdx)) ? 1u : 0u);
	m_nAnmStateNext = kind;
	m_nAnmChangeTime = time;
	if (!CheckRepetition(animationClip, num2))
	{
		for (uint num3 = 0u; num3 < 3; num3++)
		{
			pModel(num3)->RegistAnm(num2, animationClip);
			pModel(num3)->SetWeightAnmChr(m_nAnmCuurentIdx, 1.0f);
			pModel(num3)->SetWeightAnmChr(num2, 0.0f);
		}
	}
}

bool cTaskCharPacman::Control()
{
	int pad1No = InGamePadManager::GetPad1No();
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	// DOL 0x8016d68c: `get_pad1(0)->hold & 0x840` = B (0x40) or Z (0x800) held -> brake (state 0x20); the C# only had B (CROSS)
	if (InGamePadManager::GetKey(pad1No, PadDefine::EPadKeycode::eKEY_CROSS) || InGamePadManager::GetKey(pad1No, PadDefine::EPadKeycode::eKEY_L1))
	{
		if (nullptr != m_SndHndMove)
		{
			SoundManager::get_SE()->Stop(m_SndHndMove, 0.3f);
		}
		OnState(32u);
	}
	else
	{
		Vector2 padLStickOrKey = GetPadLStickOrKey(pad1No);
		m_fStickX = ((0.08f < Mathf::Abs(padLStickOrKey.x)) ? padLStickOrKey.x : 0.0f);
		m_fStickY = ((0.08f < Mathf::Abs(padLStickOrKey.y)) ? padLStickOrKey.y : 0.0f);
		float fPadForce = GetParam()->fPadForce;
		float num = m_fStickX * fPadForce;
		float num2 = m_fStickY * fPadForce;
		Vector4 v = Vector4::zero;
		Vector4 v2 = Vector4::zero;
		Vector4 v3 = Vector4::zero;
		Vector4 v4 = Vector4();
		::Constants::jetGeoVecUnit(v);
		::Constants::jetGeoVecUnit(v2);
		::Constants::jetGeoVecUnit(v3);
		::Constants::jetGeoVecSub(v4, cAppG03PacnRollGame2->pTaskCamera()->vPos(), cAppG03PacnRollGame2->pTaskCamera()->vTarget());
		::Constants::jetGeoVecSet(v2, 0.0f, 1.0f, 0.0f, 1.0f);
		if (Constants::IsVecNormalize(v4))
		{
			::Constants::jetGeoVecNormalize(v3, v4);
			::Constants::jetGeoVecOuterProduct(v, v3, v2);
			if (Constants::IsVecNormalize(v))
			{
				::Constants::jetGeoVecNormalize(v, v);
				::Constants::jetGeoVecScale(vAccel(), v, 0.0f - num);
				Vector4 v5 = Vector4::zero;
				::Constants::jetGeoVecSet(v5, v3[0], 0.0f, v3[2], 1.0f);
				::Constants::jetGeoVecNormalize(v5, v5);
				::Constants::jetGeoVecScale(v5, v5, 0.0f - num2);
				::Constants::jetGeoVecAdd(vAccel(), vAccel(), v5);
			}
			if (::Constants::fabs(vMove()[0]) <= 0.1f && ::Constants::fabs(vMove()[2]) <= 0.1f)
			{
				if (nullptr != m_SndHndMove)
				{
					SoundManager::get_SE()->Stop(m_SndHndMove, 0.5f);
				}
			}
			else if (nullptr == m_SndHndMove && !IsState(1024u) && !IsState(2048u))
			{
				switch (nType())
				{
				case 0u:
					m_SndHndMove = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE018_KOROGARI1, true);
					break;
				case 1u:
					m_SndHndMove = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE019_KOROGARI2, true);
					break;
				case 2u:
					m_SndHndMove = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE020_KOROGARI3, true);
					break;
				}
			}
		}
	}
	if (0.0f < m_fStickX || 0.0f < m_fStickY)
	{
		return true;
	}
	return false;
}

void cTaskCharPacman::AdjustSpeed()
{
	float sp = (IsState(3u) ? ((!(GetParam()->fDashSpeedMax > GetParam()->fDashFloorSpeedMax)) ? GetParam()->fDashFloorSpeedMax : GetParam()->fDashSpeedMax) : (((pHitObj(1) != nullptr && !IsState(16384u)) || IsState(3072u)) ? GetParam()->fSpeedMax : GetParam()->fSpeedMaxAir));
	Vector4 v;
	if (pHitObj(1) == nullptr && !IsState(3072u))
	{
		::Constants::jetGeoVecCopy(v, vMove());
		v[1] = 0.0f;
		if (Constants::IsVecNormalize(v))
		{
			::Constants::jetGeoVecNormalize(v, v);
			float t = ((!IsState(1u)) ? GetParam()->fAirResist : GetParam()->fAirResistDash);
			::Constants::jetGeoVecScale(v, v, t);
			::Constants::jetGeoVecSub(vMove(), vMove(), v);
		}
	}
	if (!IsOverSpeed(sp, (Nullable<Vector4>)nullptr))
	{
		::Constants::jetGeoVecAdd(vMove(), vMove(), vAccel());
		return;
	}
	::Constants::jetGeoVecAdd(vMove(), vMove(), vAccel());
	::Constants::jetGeoVecNormalize(v, vMove());
	::Constants::jetGeoVecScale(vMove(), v, sp);
}

bool cTaskCharPacman::IsOverSpeed(const float& sp, const Nullable<Vector4>& move)
{
	float num = (move.has_value() ? ::Constants::jetGeoVecInnerProduct(move.value(), move.value()) : ::Constants::jetGeoVecInnerProduct(vMove(), vMove()));
	if (!(sp * sp < num))
	{
		return false;
	}
	return true;
}

float cTaskCharPacman::CalcSpeed()
{
	return ::Constants::jetGeoVecLength(vMove());
}

void cTaskCharPacman::ChangePacn(uint kind)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	CustomKingdomUtil::Assert(kind == 0 || 1 == kind || 2 == kind, "error");
	if (nType() != kind)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE025_HENNSHINN);
		cAppG03PacnRollGame2->SetEffect(3u, vTrans(), Vector4(), m_PtkHndChange);
		m_PtkHndWing->Detach(true);
		if (nullptr != m_SndHndMove)
		{
			SoundManager::get_SE()->Stop(m_SndHndMove);
		}
		SetType(kind);
	}
	else
	{
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE016_FLUIT);
	}
}

bool cTaskCharPacman::MoveGoalCenter()
{
	Vector4 v = Vector4();
	Vector4 v2 = Vector4::zero;
	float num = 0.06f;
	cTaskGoal* cTaskGoal2 = Constants::PACN_APP()->pTaskGoal();
	::Constants::jetGeoVecSet(v, cTaskGoal2->Model->vTrans()[0], cTaskGoal2->Model->vTrans()[1] + 0.6f, cTaskGoal2->Model->vTrans()[2], 1.0f);
	::Constants::jetGeoVecSet(v2, vTrans()[0], vTrans()[1], vTrans()[2], 1.0f);
	::Constants::jetGeoVecSub(v, v, v2);
	if (!Constants::IsVecNormalize(v) || ::Constants::jetGeoVecInnerProduct(v, v) <= num * num)
	{
		vTrans()[0] = cTaskGoal2->Model->vTrans()[0];
		vTrans()[1] = cTaskGoal2->Model->vTrans()[1] + 0.6f;
		vTrans()[2] = cTaskGoal2->Model->vTrans()[2];
		return true;
	}
	::Constants::jetGeoVecNormalize(v, v);
	::Constants::jetGeoVecScale(v, v, num);
	::Constants::jetGeoVecAdd(vTrans(), vTrans(), v);
	return false;
}

void cTaskCharPacman::UpdateRollMatrix()
{
	Vector4 v = Vector4::zero;
	Vector4 v2 = Vector4::zero;
	m_bRollCheck = false;
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	::Constants::jetGeoVecUnit(v2);
	::Constants::jetGeoVecSub(v2, vTrans(), vTransOld2());
	if (pHitObj(1) == nullptr && !IsState(33792u) && !IsStateOld(32768u))
	{
		v2[1] = 0.0f;
	}
	float num = ::Constants::jetGeoVecLength(v2);
	if (!Constants::IsVecNormalize(v2) || !(num > 0.005f))
	{
		return;
	}
	float num2 = (float)std::numbers::pi * 2.0f * fRadius();
	if (num > num2)
	{
		num -= num2;
	}
	float num3 = (float)std::numbers::pi * (num / num2);
	if (!(std::numeric_limits<float>::lowest() < Mathf::Abs(num3)) || !(Mathf::Abs(num3) < std::numeric_limits<float>::max()) || !(::Constants::fabs(num3) > 0.005f))
	{
		return;
	}
	::Constants::jetGeoVecSet(v, 0.0f, 1.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecOuterProduct(v, v, v2);
	bool flag = false;
	if (Constants::IsVecNormalize(v))
	{
		flag = true;
	}
	else if (pHitFace(1) != nullptr)
	{
		::Constants::jetGeoVecTrans(v, pHitObj(1)->pMtx(), pHitFace(1)->vNorm);
		::Constants::jetGeoVecOuterProduct(v, v, v2);
		if (Constants::IsVecNormalize(v))
		{
			flag = true;
		}
	}
	if (flag)
	{
		::Constants::jetGeoVecNormalize(v, v);
		::Constants::jetGeoMtxRotAxis(m, m, v, num3);
		::Constants::jetGeoMtxMult(m_mRoll, m, m_mRoll);
		m_bRollCheck = true;
	}
}

bool cTaskCharPacman::ResetRollMatrix()
{
	Constants::PACN_APP();
	Vector4 v = Vector4::zero;
	bool flag = true;
	float r = vRotate()[1];
	::Constants::jetGeoVecSet(v, 1.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecRotY(v, v, r);
	Vector4 v2;
	::Constants::jetGeoVecTrans(v2, m_mRoll, v);
	Matrix4x4 m;
	if (Mathf::Acos(::Constants::jetGeoVecInnerProduct(v, v2)) > ::Constants::JET_DEG2RAD(3.0f))
	{
		flag = false;
		::Constants::jetGeoVecNormalize(v2, v2);
		::Constants::jetGeoVecOuterProduct(v, v2, v);
		::Constants::jetGeoMtxIdentity(m);
		::Constants::jetGeoMtxRotAxis(m, m, v, ::Constants::JET_DEG2RAD(3.0f));
		::Constants::jetGeoMtxMult(m_mRoll, m, m_mRoll);
	}
	::Constants::jetGeoVecSet(v, 0.0f, 1.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecTrans(v2, m_mRoll, v);
	::Constants::jetGeoVecNormalize(v2, v2);
	if (Mathf::Acos(::Constants::jetGeoVecInnerProduct(v, v2)) > ::Constants::JET_DEG2RAD(3.0f))
	{
		flag = false;
		::Constants::jetGeoVecOuterProduct(v, v2, v);
		::Constants::jetGeoMtxIdentity(m);
		::Constants::jetGeoMtxRotAxis(m, m, v, ::Constants::JET_DEG2RAD(3.0f));
		::Constants::jetGeoMtxMult(m_mRoll, m, m_mRoll);
	}
	::Constants::jetGeoVecSet(v, 0.0f, 0.0f, 1.0f, 1.0f);
	::Constants::jetGeoVecRotY(v, v, r);
	::Constants::jetGeoVecTrans(v2, m_mRoll, v);
	::Constants::jetGeoVecNormalize(v2, v2);
	if (Mathf::Acos(::Constants::jetGeoVecInnerProduct(v, v2)) > ::Constants::JET_DEG2RAD(3.0f))
	{
		flag = false;
		::Constants::jetGeoVecOuterProduct(v, v2, v);
		::Constants::jetGeoMtxIdentity(m);
		::Constants::jetGeoMtxRotAxis(m, m, v, ::Constants::JET_DEG2RAD(3.0f));
		::Constants::jetGeoMtxMult(m_mRoll, m, m_mRoll);
	}
	if (flag)
	{
		::Constants::jetGeoMtxIdentity(m_mRoll);
		return true;
	}
	return false;
}

void cTaskCharPacman::LookCameraRollMatrix()
{
	::Constants::jetGeoMtxIdentity(m_mRoll);
	::Constants::jetGeoMtxRotY(m_mRoll, m_mRoll, Get2GameraRot());
}

float cTaskCharPacman::Get2GameraRot()
{
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	::Constants::jetGeoVecSub(v, Constants::PACN_APP()->pTaskCamera()->vPos(), Constants::PACN_APP()->pTaskCamera()->vTarget());
	v[1] = 0.0f;
	if (Constants::IsVecNormalize(v))
	{
		::Constants::jetGeoVecNormalize(v, v);
		if ((std::numeric_limits<float>::lowest() < Mathf::Abs(v[0]) && Mathf::Abs(v[0]) < std::numeric_limits<float>::max()) || (std::numeric_limits<float>::lowest() < Mathf::Abs(v[2]) && Mathf::Abs(v[2]) < std::numeric_limits<float>::max()))
		{
			float num = Mathf::Atan2(v[0], v[2]);
			if (std::numeric_limits<float>::lowest() < Mathf::Abs(num) && Mathf::Abs(num) < std::numeric_limits<float>::max())
			{
				return num;
			}
		}
	}
	return 0.0f;
}

void cTaskCharPacman::RegistHitParam(uint idx, Constants::sHitParam* param)
{
	CustomKingdomUtil::Assert(4 > idx, "error");
	CustomKingdomUtil::Assert(param != nullptr, "nullpo");
	m_vecHitParam[idx].push_back(param);
}

void cTaskCharPacman::ClearHitParam()
{
	for (uint num = 0u; num < 4; num++)
	{
		m_vecHitParam[num].clear();
	}
}

void cTaskCharPacman::SetRetryData()
{
	KingdomGlobal::PacnRollSys->pRetryData()->nLife = m_nLife;
	KingdomGlobal::PacnRollSys->pRetryData()->nMaxHP = m_nMaxHP;
}

bool cTaskCharPacman::IsMiss()
{
	return IsState(256u);
}

bool cTaskCharPacman::IsMissFinish()
{
	return IsState(512u);
}

bool cTaskCharPacman::IsGameOver()
{
	return m_bGameOver;
}

bool cTaskCharPacman::IsGround()
{
	if (pHitObj(1) != nullptr)
	{
		return true;
	}
	return false;
}

void cTaskCharPacman::Attacked(cHitChar* pRev, cHitChar* pAtk)
{
	Vector4 v;
	if (pAtk->bFlagTest(32u))
	{
		::Constants::jetGeoVecSub(v, pRev->vPos(), pAtk->vBoundCenter());
	}
	else
	{
		::Constants::jetGeoVecSub(v, pRev->vPos(), pAtk->vPos());
	}
	v[1] = 0.0f;
	if (IsState(4u) && (pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_RIDE_CAR || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_RIDE_UFO || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_RIDE_BOAT || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_RIDE_PLANE || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_RIDE_HOVER))
	{
		float num = ::Constants::jetGeoVecLength(v);
		if (CustomKingdomUtil::IF(num))
		{
			::Constants::jetGeoVecScale(v, v, GetParam()->fBoundSpeed / num);
			::Constants::jetGeoVecCopy(vMove(), v);
		}
		return;
	}
	if (pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_CLYDE || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_PINKY || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_BLINKY || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_INKY || pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_BOSS)
	{
		if (nHP() == 1)
		{
			if (pAtk->pParent()->nKind() == cTaskChar::eKIND::eKIND_PINKY)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE105_GH_TORITUKI2);
			}
			else if (pAtk->pParent()->nKind() != cTaskChar::eKIND::eKIND_BOSS)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE104_GH_TORITUKI);
			}
		}
		else if (pAtk->pParent()->nKind() != cTaskChar::eKIND::eKIND_BOSS)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE094_GHOST);
		}
		else if (nType() != 2)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE094_GHOST);
		}
		Constants::PACN_APP()->SetEffect(5u, pRev->pParent()->vTrans(), Vector4());
	}
	DamageProcess(1u, v, GetParam()->fBoundSpeed, pAtk->bFlagTest(16u) ? 1 : 0);
}

bool cTaskCharPacman::DamageProcess(uint nDamage, Vector4 vVec, float fSpeed, int nKnightDef)
{
	if (CustomKingdomUtil::IF(vVec))
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, vVec);
		float num = ::Constants::jetGeoVecLength(v);
		if (CustomKingdomUtil::IF(num))
		{
			::Constants::jetGeoVecScale(v, v, fSpeed / num);
			::Constants::jetGeoVecCopy(vMove(), v);
		}
	}
	if (nDamage != 0)
	{
		if (!CustomKingdomUtil::IF(nKnightDef) || nType() != 2)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE021_DAMAGE);
			if (Random::Range(0, 2) == 0)
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE096_DAMAGE);
			}
			else
			{
				SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE097_DAMAGE2);
			}
			if (nHP() >= nDamage)
			{
				nHP() -= nDamage;
			}
			else
			{
				nHP() = 0u;
			}
		}
		if (nKnightDef < 2 || nType() != 2)
		{
			SetAnmState(4u, 5u, 40u);
			OnState(8u);
		}
	}
	return true;
}

// DOL 0x8016f488 (two phases per clip pair: BrakeOn while hanging on a hit object, BrakeOff afterwards; +0x6a4 on frame, +0x6a8 off frame, +0x6ac bBrakeAnmMax).
// The clips (m_AnmChrBrakeOn +0x688 / m_AnmChrBrakeOff +0x694) are sampled directly: translate of "root" at frame 0, scale + rt of "base" at the phase frame.
void cTaskCharPacman::BrakeAnime()
{
	bool flag = false;
	if (!IsState(32u) && m_bBrakeAnmMax)
	{
		const std::shared_ptr<wii::ChrAnim> clip = cKingdomModelBase::ChrAnimOf(m_AnmChrBrakeOff[nType()]);
		const float num = clip != nullptr ? (float)clip->numFrame : 0.0f;
		m_fAnmCharBrakeOffFrame++;
		if (m_fAnmCharBrakeOffFrame >= num)
		{
			m_fAnmCharBrakeOffFrame = num;
			m_fAnmCharBrakeOnFrame = 0.0f;
			m_bBrakeAnmMax = false;
		}
		Matrix4x4 m;
		::Constants::jetGeoMtxIdentity(m);
		wii::BoneKey root, base;
		if (clip != nullptr)
		{
			clip->Sample("root", 0.0f, root);
			clip->Sample("base", m_fAnmCharBrakeOffFrame, base);
		}
		::Constants::jetGeoMtxTrans(m, m, Vector4(root.trans[0], root.trans[1], root.trans[2], 1.0f));
		::Constants::jetGeoMtxScale(m, m, Vector4(base.scale[0], base.scale[1], base.scale[2], 1.0f));
		::Constants::jetGeoMtxIdentity(m_mAnm);
		::Constants::jetGeoMtxMult(m_mAnm, m_mAnm, cKingdomModelBase::BoneRt(base, true));
		::Constants::jetGeoMtxMult(m_mAnm, m_mAnm, m);
	}
	else if (pHitObj(1) != nullptr)
	{
		const std::shared_ptr<wii::ChrAnim> clip = cKingdomModelBase::ChrAnimOf(m_AnmChrBrakeOn[nType()]);
		if (IsState(32u) && !IsState(3072u))
		{
			const float num2 = clip != nullptr ? (float)clip->numFrame : 0.0f;
			m_fAnmCharBrakeOnFrame++;
			if (m_fAnmCharBrakeOnFrame >= num2)
			{
				m_fAnmCharBrakeOnFrame = num2;
				m_fAnmCharBrakeOffFrame = 0.0f;
				m_bBrakeAnmMax = true;
			}
			flag = true;
		}
		else if (m_fAnmCharBrakeOnFrame > 0.0f)
		{
			m_fAnmCharBrakeOnFrame--;
			if (m_fAnmCharBrakeOnFrame < 0.0f)
			{
				m_fAnmCharBrakeOnFrame = 0.0f;
			}
			flag = true;
		}
		if (flag)
		{
			Matrix4x4 m3;
			::Constants::jetGeoMtxIdentity(m3);
			wii::BoneKey root, base;
			if (clip != nullptr)
			{
				clip->Sample("root", 0.0f, root);
				clip->Sample("base", m_fAnmCharBrakeOnFrame, base);
			}
			::Constants::jetGeoMtxTrans(m3, m3, Vector4(root.trans[0], root.trans[1], root.trans[2], 1.0f));
			Vector4 v3 = Vector4::zero;
			::Constants::jetGeoVecUnit(v3);
			v3[1] = 1.0f;
			Constants::sFace* sFace = pHitFace(1);
			Vector4 v4;
			::Constants::jetGeoVecTrans(v4, pHitObj(1)->pMtxRotate(), sFace->vNorm);
			float f = ::Constants::jetGeoVecInnerProduct(v3, v4);
			if (0.97f > ::Constants::fabs(f))
			{
				::Constants::jetGeoVecOuterProduct(v3, v3, v4);
				::Constants::jetGeoMtxRotAxis(m3, m3, v3, Mathf::Acos(f));
			}
			::Constants::jetGeoMtxScale(m3, m3, Vector4(base.scale[0], base.scale[1], base.scale[2], 1.0f));
			::Constants::jetGeoMtxMult(m_mAnm, m_mAnm, cKingdomModelBase::BoneRt(base, true));
			::Constants::jetGeoMtxMult(m_mAnm, m_mAnm, m3);
		}
	}
	else
	{
		m_fAnmCharBrakeOnFrame = 0.0f;
		m_fAnmCharBrakeOffFrame = 0.0f;
		m_bBrakeAnmMax = false;
	}
}

void cTaskCharPacman::RecoverHp(uint n)
{
	if (!IsState(256u))
	{
		if (n == 0)
		{
			m_nHP = m_nMaxHP;
		}
		else
		{
			m_nHP += n;
		}
	}
	if (m_nMaxHP < m_nHP)
	{
		m_nHP = m_nMaxHP;
	}
}

void cTaskCharPacman::GainHpLimit(uint n)
{
	if (!IsState(256u))
	{
		m_nMaxHP += n;
	}
	if (7 < m_nMaxHP)
	{
		m_nMaxHP = 7u;
	}
}

void cTaskCharPacman::GainLife(uint n)
{
	if (!IsState(256u))
	{
		m_nLife += n;
	}
	if (99 < m_nLife)
	{
		m_nLife = 99u;
	}
}

bool cTaskCharPacman::CheckFallArea()
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	Vector4 v;
	if (pHitObj(1) == nullptr)
	{
		::Constants::jetGeoVecCopy(v, vLastFloorPos());
		return cAppG03PacnRollGame2->pTaskFallArea()->AreaCheck(vTrans(), v);
	}
	if (pHitObj(1)->IsState(32u))
	{
		::Constants::jetGeoVecTrans(v, pHitObj(1)->pMtxInv(), vTrans());
		::Constants::jetGeoVecTrans(v, pHitObj(1)->pMtxOld(), v);
		return cAppG03PacnRollGame2->pTaskFallArea()->AreaCheck(vTrans(), v);
	}
	return false;
}

void cTaskCharPacman::SetMaterialAlpha(byte alpha)
{
	for (uint num = 0u; num < 3; num++)
	{
		SetMaterialAlpha(num);
	}
}

void cTaskCharPacman::IjikeStart()
{
	if (!IsState(4u))
	{
		m_nEatComboCnt = 0u;
	}
	if (m_PtkHndPowerCookie->IsValid())
	{
		m_PtkHndPowerCookie->Detach(true);
	}
	if (!m_PtkHndPowerCookie->IsValid())
	{
		Constants::PACN_APP()->SetEffect(12u, vTrans(), Vector4(), m_PtkHndPowerCookie);
	}
	cTaskChar::IjikeStart();
	OnState(4u);
	SetAnmState(3u, 5u, 0u);
}

void cTaskCharPacman::IjikeEnd()
{
	m_PtkHndPowerCookie->Detach(true);
	cTaskChar::IjikeEnd();
	OffState(4u);
	SetAnmState(0u, 5u, 0u);
}

void cTaskCharPacman::RotateYPacMan(float rot)
{
	if (KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		::Constants::jetGeoMtxIdentity(m_mRoll);
		::Constants::jetGeoMtxRotY(m_mRoll, m_mRoll, rot);
	}
}

void cTaskCharPacman::DisposeEffect()
{
	m_PtkHndPowerCookie->Detach(true);
	m_PtkHndDash->Detach(true);
	m_PtkHndSweat->Detach(true);
	m_PtkHndPoson->Detach(true);
	m_PtkHndBrake->Detach(true);
	m_PtkHndWing->Detach(true);
	m_PtkHndWaterIn->Detach(true);
	m_PtkHndRipple->Detach(true);
	SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE052_KAKYUU);
	if (nullptr != m_SndHndMove)
	{
		SoundManager::get_SE()->Stop(m_SndHndMove);
	}
}

void cTaskCharPacman::HitObjRegist(int nKind, cTaskBgHit* pOBJ)
{
	CustomKingdomUtil::Assert(nKind < 4, "over");
	CustomKingdomUtil::Assert(nKind >= 0, "under");
	cTaskCharPacman::BGHIT_HISTORY* bGHIT_HISTORY = m_sHitObjHistry[nKind];
	bGHIT_HISTORY->m_aData[bGHIT_HISTORY->m_nIndex] = pOBJ;
}

cTaskBgHit* cTaskCharPacman::HitObjGet(int nKind, int nIndex)
{
	CustomKingdomUtil::Assert(nKind < 4, "over");
	CustomKingdomUtil::Assert(nIndex < 8, "over");
	CustomKingdomUtil::Assert(nKind >= 0, "under");
	CustomKingdomUtil::Assert(nIndex >= 0, "under");
	cTaskCharPacman::BGHIT_HISTORY* bGHIT_HISTORY = m_sHitObjHistry[nKind];
	int i;
	for (i = bGHIT_HISTORY->m_nIndex - nIndex; i < 0; i += 8)
	{
	}
	return bGHIT_HISTORY->m_aData[i];
}

void cTaskCharPacman::HitObjTick()
{
	for (int i = 0; i < 4; i++)
	{
		if (++m_sHitObjHistry[i]->m_nIndex >= 8)
		{
			m_sHitObjHistry[i]->m_nIndex = 0;
		}
		m_sHitObjHistry[i]->m_aData[m_sHitObjHistry[i]->m_nIndex] = nullptr;
	}
}

// DOL 0x8016d68c reads only the Nunchuk stick floats of get_pad1 (+0x10 x, +0x14 y); the D-pad never steers (it turns the camera). The C# also let the
// D-pad drive the stick (a Unity-port convenience): removed. Keyboard steering = WASD (input.cfg StickUp / Down / Left / Right), gamepad = left stick.
Vector2 cTaskCharPacman::GetPadLStickOrKey(int padNum)
{
	return InGamePadManager::GetStick(padNum, PadDefine::scLSTICK);
}

}
