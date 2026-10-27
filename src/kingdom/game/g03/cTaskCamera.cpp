#include "kingdom/game/g03/cTaskCamera.hpp"
#include "GamePad.hpp"
#include <cstdio>
#include <cstdlib>
#include "kingdom/NativeScene.hpp"
#include "platform/devtools.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"

#include <numbers>
#include <string>

#include "CSharpCompat.hpp"
#include "cJetTask.hpp"
#include "CSVParam.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/game/g03/cCameraParam.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include <AudioListenerManager.hpp>
#include <Constants.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>

namespace kingdom::game::g03
{

float cTaskCamera::dot{};
float cTaskCamera::fEscAngle{};

Camera* cTaskCamera::get_MainCam()
{
	return m_mainCam;
}

float cTaskCamera::FX32TOF32(float val)
{
	return val / 4096.0f;
}

void cTaskCamera::SetNextThetaLerp(float fAngle, bool rflg)
{
	if (!rflg)
	{
		m_fThetaSpeed = ::Constants::JET_DEG2RAD(0.0f);
		m_fThetaAccel = ::Constants::JET_DEG2RAD(0.2f);
	}
	m_fNextTheta = fAngle;
	m_bThetaLerp = true;
}

void cTaskCamera::SetNextPhiLerp(float fAngle)
{
	m_fPhiSpeed = 0.0f;
	m_fNextPhi = fAngle;
	m_bPhiLerp = true;
}

void cTaskCamera::SetNextDistance(float fDist)
{
	m_fDistSpeed = 0.0f;
	m_fNextDist = fDist;
	m_bDistLerp = true;
}

bool cTaskCamera::bCameraKeepOut()
{
	return m_bCameraKeepOut;
}

void cTaskCamera::SetTarget(Vector4 v)
{
	::Constants::jetGeoVecCopy(m_vTarget, v);
}

uint& cTaskCamera::nType()
{
	return m_nType;
}

Vector4& cTaskCamera::vPos()
{
	return m_vPos;
}

float cTaskCamera::vPos(uint n)
{
	CustomKingdomUtil::Assert(n < 4, "error");
	return m_vPos[(int)n];
}

Vector4& cTaskCamera::vTarget()
{
	return m_vTarget;
}

float cTaskCamera::vTarget(uint n)
{
	CustomKingdomUtil::Assert(n < 4, "error");
	return m_vTarget[(int)n];
}

void cTaskCamera::SetCamIdx(uint camIdx)
{
	m_nCamIdx = camIdx;
}

uint cTaskCamera::GetCamIdx()
{
	return m_nCamIdx;
}

void cTaskCamera::RegistFirstPacPos(Vector4 vPos)
{
	::Constants::jetGeoVecCopy(m_vTarget, vPos);
}

float& cTaskCamera::fMenuZoom()
{
	return m_fMenuZoom;
}

Vector4& cTaskCamera::vMenuCamera()
{
	return m_vMenuCamera;
}

float cTaskCamera::vMenuCamera(uint n)
{
	CustomKingdomUtil::Assert(n < 4, "error");
	return m_vMenuCamera[(int)n];
}

cTaskCamera::cTaskCamera()
	
{
	SetName("cTaskCamera");
	Init();
	Update();
}

cTaskCamera::~cTaskCamera()
{
	Term();
}

void cTaskCamera::Init()
{
	m_nCamIdx = 0u;
	m_nType = 0u;
	m_fFollowRot = 0.0f;
	m_nBackWaitCnt = 0u;
	m_fFollowHeight = 0.0f;
	m_bInputFixed = false;
	m_fFollowHeight = 10.0f;
	m_fDistanceOld = 10.0f;
	::Constants::jetGeoVecSet(m_vTarget, 0.0f, 0.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(m_vPos, 0.0f, 1.0f, 0.0f, 1.0f);
	::Constants::jetGeoVecSet(m_vPosLocal, 0.0f, 0.0f, 8.0f, 1.0f);
	::Constants::jetGeoVecSet(m_vUp, 0.0f, 1.0f, 0.0f, 1.0f);
	m_fMenuZoom = 0.0f;
	::Constants::jetGeoVecZero(m_vMenuCamera);
	m_nNowArea = 999;
	m_bInitParam = false;
	m_bStopCamera = false;
	m_fWaitDistance = 3.4f;
	::Constants::jetGeoVecSet(m_vNextTarget, 0.0f, 0.0f, 0.0f, 1.0f);
	m_fAtDelay = 0.1f;
	m_fEyeDelay = 0.0f;
	m_nEscapeDir = 0;
	m_bPrevFrameCameraHit = 0u;
	m_bNowFrameCameraHit = 0u;
	m_fParamDistanceXZ = 0.0f;
	m_fParamHeight = 0.0f;
	m_fThetaAngle = 0.0f;
	m_fPhiAngle = 0.0f;
	m_fDistance = 0.0f;
	m_bThetaLerp = false;
	m_bPhiLerp = false;
	m_bDistLerp = false;
	m_fNextTheta = 0.0f;
	m_fNextPhi = 0.0f;
	m_fNextDist = 0.0f;
	m_fThetaSpeed = 0.0f;
	m_fPhiSpeed = 0.0f;
	m_fDistSpeed = 0.0f;
	m_fThetaAccel = 0.0f;
	m_fPhiAccel = ::Constants::JET_DEG2RAD(0.05f);
	m_fDistAccel = 0.1f;
	m_bRecoAngle = false;
	m_fRecoAngle = 0.0f;
	m_fRecoPower = 0.0f;
	m_fThetaSpeed = ::Constants::JET_DEG2RAD(0.0f);
	m_fThetaAccel = ::Constants::JET_DEG2RAD(1.0f);
	m_fClipFarDist = 150.0f;
	m_fClipValue = 1.0f;
	::Constants::jetGeoVecSet(m_vClipVec, 0.0f, 0.0f, 1.0f, 1.0f);
	::Constants::jetGeoVecSet(m_vClipVecOut, 1.0f, 0.0f, 0.0f, 1.0f);
	if (nullptr == m_mainCam)
	{
		// TODO(unity): m_mainCam = Camera.main;
	}
	// TODO(unity): AudioListenerManager.SetRotStalkTarget(m_mainCam.gameObject);
}

void cTaskCamera::Term()
{
}

bool cTaskCamera::Execute(cJetTask* pParam)
{
	if (std::getenv("MEGAMIX_SCENE_DEBUG")) std::fprintf(stderr, "camera: type %u topmenu %d pac %p pause %d nakajima %d\n", m_nType, (int)KingdomGlobal::PacnRollSys->IsTopMenuMode(), (void*)Constants::PACMAN_OBJ(), (int)Constants::IS_PAC_PAUSE(), (int)Constants::IS_NAKAJIMA_PAUSE());
	if (std::getenv("MEGAMIX_SCENE_DEBUG")) std::fprintf(stderr, "camera: dist %.2f phi %.2f theta %.2f pos (%.1f %.1f %.1f) target (%.1f %.1f %.1f) nowparam %p flag %x\n", m_fDistance, m_fPhiAngle, m_fThetaAngle, m_vPos.x, m_vPos.y, m_vPos.z, m_vTarget.x, m_vTarget.y, m_vTarget.z, (void*)m_NowParam, m_NowParam ? m_NowParam->m_nFlag : 0u);
	m_bCameraKeepOut = false;
	m_bPrevFrameCameraHit = m_bNowFrameCameraHit;
	m_bNowFrameCameraHit = 0u;
	if (!CustomKingdomUtil::IF(m_bPrevFrameCameraHit))
	{
		m_nEscapeDir = 0;
	}
	if (!KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		switch (m_nType)
		{
		case 0u:
			FrameMovePurSuit();
			break;
		case 1u:
			FrameMoveGameClearCamera();
			break;
		case 2u:
			FrameMoveFreeFall();
			break;
		case 3u:
			FrameMoveInitGame();
			break;
		}
	}
	else
	{
		ControlFreeMenu();
	}
	return true;
}

void cTaskCamera::Draw(int nStatus, cJetTask* pParam)
{
	Update();
}

void cTaskCamera::Setup(string key, uint camIdx, uint anmIdx, float frame)
{
	if (nullptr == m_mainCam)
	{
		// TODO(unity): m_mainCam = Camera.main;
	}
	// TODO(unity): m_mainCam.fieldOfView = 40f;
	// TODO(unity): m_mainCam.nearClipPlane = 0.1f;
	// TODO(unity): m_mainCam.farClipPlane = 32768f;
	InitAreaParam();
}

void cTaskCamera::Update()
{
	if (nullptr == m_mainCam)
	{
		// TODO(unity): m_mainCam = Camera.main;
	}
	if (nullptr == m_gazePoint)
	{
		// TODO(unity): m_gazePoint = GameObject.Find("GazePoint");
	}
	// native: the camera of the 3D pass (Wii space, no X mirror; Unity's camera was fov 40, near 0.1, far 32768)
	{
		const float eye[3] = {m_vPos.x, m_vPos.y, m_vPos.z}, target[3] = {m_vTarget.x, m_vTarget.y + devtools::G03FreecamTargetY(m_fDistance), m_vTarget.z};   // devtools: --g03-freecam
		kingdom::NativeScene::SetCamera(eye, target, 40.0f, 0.1f, 32768.0f);
	}
	Vector4 v = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	Vector4 v2;
	::Constants::jetGeoVecSub(v2, m_vTarget, m_vPos);
	v2[1] += devtools::G03FreecamTargetY(m_fDistance);   // devtools: --g03-freecam
	::Constants::jetGeoVecOuterProduct(m_vClipVecOut, v2, v);
	if (Constants::IsVecNormalize(m_vClipVec))
	{
		::Constants::jetGeoVecNormalize(m_vClipVec, v2);
	}
	if (Constants::IsVecNormalize(m_vClipVecOut))
	{
		::Constants::jetGeoVecNormalize(m_vClipVecOut, m_vClipVecOut);
	}
}

void cTaskCamera::ControlFree()
{
	if (kingdom::Constants::IS_PAUSE())
	{
		return;
	}
	int pad1No = InGamePadManager::GetPad1No();
	if (Constants::PACMAN_OBJ() != nullptr)
	{
		SetTarget(Constants::PACMAN_OBJ()->vTrans());
	}
	float num = 0.0f;
	float num2 = 0.0f;
	if (!Constants::PACN_APP()->pTaskConsRetry()->IsVisible())
	{
		if (GetPadCnt(pad1No, cTaskCamera::EKeyType::eLEFT))
		{
			num = -1.0f;
		}
		else if (GetPadCnt(pad1No, cTaskCamera::EKeyType::eRIGHT))
		{
			num = 1.0f;
		}
		if (GetPadCnt(pad1No, cTaskCamera::EKeyType::eUP))
		{
			num2 = 1.0f;
		}
		else if (GetPadCnt(pad1No, cTaskCamera::EKeyType::eDOWN))
		{
			num2 = -1.0f;
		}
		num *= 1.2f;
		num2 *= 1.2f;
	}
	Vector4 v;
	::Constants::jetGeoVecCopy(v, m_vPosLocal);
	if ((CustomKingdomUtil::IF(num) || CustomKingdomUtil::IF(num2)) && !m_bInputFixed)
	{
		Matrix4x4 m;
		::Constants::jetGeoMtxIdentity(m);
		Vector4 v2;
		::Constants::jetGeoVecNormalize(v2, v);
		Vector4 v3;
		::Constants::jetGeoVecOuterProduct(v3, v2, m_vUp);
		::Constants::jetGeoVecNormalize(v3, v3);
		::Constants::jetGeoMtxRotY(m, m, ::Constants::JET_DEG2RAD(2.0f) * num);
		::Constants::jetGeoVecTrans(m_vPosLocal, m, v);
	}
	::Constants::jetGeoVecAdd(m_vPos, m_vTarget, m_vPosLocal);
}

void cTaskCamera::ControlFollow()
{
	if (kingdom::Constants::IS_PAUSE())
	{
		return;
	}
	ControlFree();
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	bool flag = false;
	Vector4 v;
	::Constants::jetGeoVecScale(v, cTaskCharPacman2->vAccel(), -1.0f);
	v[1] = 0.0f;
	Vector4 v2;
	::Constants::jetGeoVecCopy(v2, m_vPosLocal);
	v2[1] = 0.0f;
	float num = 0.0f;
	if (Constants::IsVecNormalize(v) && Constants::IsVecNormalize(v2))
	{
		::Constants::jetGeoVecNormalize(v, v);
		::Constants::jetGeoVecNormalize(v2, v2);
		dot = ::Constants::jetGeoVecInnerProduct(v, v2);
		if (dot > 1.0f)
		{
			dot = 1.0f;
		}
		if (dot < -1.0f)
		{
			dot = -1.0f;
		}
		float num2 = v[0] * v2[2] - v[2] * v2[0];
		float num3 = dot * -1.0f + 1.0f;
		num3 *= 0.03f;
		float num4 = ((!(num2 > 0.0f) && !(dot < -0.95f)) ? (-1.0f) : 1.0f);
		if (dot < -0.5f)
		{
			m_nBackWaitCnt += 2u;
			num = ((m_nBackWaitCnt >= 180) ? (num3 * num4) : 0.0f);
			flag = true;
		}
		else
		{
			num = num3 * num4;
		}
		Matrix4x4 m;
		::Constants::jetGeoMtxIdentity(m);
		::Constants::jetGeoMtxRotY(m, m, num);
		::Constants::jetGeoVecTrans(m_vPosLocal, m, m_vPosLocal);
	}
	if (!flag)
	{
		if (m_nBackWaitCnt < 100)
		{
			m_nBackWaitCnt = 0u;
		}
		else
		{
			m_nBackWaitCnt -= 100u;
		}
	}
	if (m_nBackWaitCnt > 240)
	{
		m_nBackWaitCnt = 240u;
	}
	::Constants::jetGeoVecAdd(m_vPos, m_vTarget, m_vPosLocal);
}

void cTaskCamera::RegistAnmScn(string key)
{
}

uint cTaskCamera::GetAnmNum()
{
	return 0u;
}

void cTaskCamera::SetAnmIdx(uint anmIdx)
{
	m_nAnmIdx = anmIdx;
}

uint cTaskCamera::GetAnmIdx()
{
	return m_nAnmIdx;
}

void cTaskCamera::SetFrame(float frame)
{
}

float cTaskCamera::GetFrame()
{
	return 0.0f;
}

float cTaskCamera::GetFrameMax()
{
	return 0.0f;
}

void cTaskCamera::SetUpdateRate(float rate)
{
}

float cTaskCamera::GetUpdateRate()
{
	return 0.0f;
}

void cTaskCamera::UpdateFrame()
{
}

void cTaskCamera::RegistCameraData(Constants::CAM_PARAM* pParam, bool bInit)
{
	if (std::getenv("MEGAMIX_SCENE_DEBUG")) std::fprintf(stderr, "camera: RegistCameraData init %d flag %x phi %.1f camdist %.1f initangle %.1f\n", (int)bInit, pParam->m_nFlag, pParam->m_fPhiAngle, pParam->m_fCamDistance, pParam->m_fInitAngle);
	m_fAtDelay = pParam->m_fTargetDelay;
	m_fEyeDelay = pParam->m_fCameraDelay;
	m_fClipFarDist = pParam->m_fClipFar;
	float f = ::Constants::JET_DEG2RAD(pParam->m_fClipAngle);
	m_fClipValue = Mathf::Cos(f) / Mathf::Sin(f);
	if (CustomKingdomUtil::IF(pParam->m_nFlag & 8))
	{
		m_bRecoAngle = true;
	}
	else
	{
		m_bRecoAngle = false;
	}
	m_fRecoAngle = ::Constants::JET_DEG2RAD(pParam->m_fRecoAngle);
	m_fRecoPower = pParam->m_fRecoPower;
	m_fPhiAccel = ::Constants::JET_DEG2RAD(pParam->m_fPhiAccel);
	m_fDistAccel = pParam->m_fDistAccel;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			m_fExRange[i][j] = ::Constants::JET_DEG2RAD(pParam->m_fExRange[i][j]);
			if (m_fExRange[i][j] > (float)std::numbers::pi * 2.0f)
			{
				m_fExRange[i][j] -= (float)std::numbers::pi * 2.0f;
			}
			if (m_fExRange[i][j] < 0.0f)
			{
				m_fExRange[i][j] += (float)std::numbers::pi * 2.0f;
			}
		}
	}
	if (bInit)
	{
		if (CustomKingdomUtil::IF(pParam->m_nFlag & 0x20) && CustomKingdomUtil::IF(pParam->m_nFlag & 0x40))
		{
			m_fPhiAngle = ::Constants::JET_DEG2RAD(pParam->m_fPhiAngle);
			m_fDistance = pParam->m_fCamDistance;
		}
		else
		{
			Vector4 v = Vector4::zero;
			::Constants::jetGeoVecSet(v, 0.0f, pParam->m_fParamHeight, pParam->m_fParamDistanceXZ, 1.0f);
			m_fPhiAngle = Mathf::Atan2(v[1], v[2]);
			m_fDistance = ::Constants::jetGeoVecLength(v);
		}
		m_bThetaLerp = false;
		m_bPhiLerp = false;
		m_bDistLerp = false;
		if (KingdomGlobal::PacnRollSys->IsRetry() || !Constants::CAM_PARAM_CHECK(pParam, 19))
		{
			m_fThetaAngle = ::Constants::JET_DEG2RAD(pParam->m_fInitAngle);
		}
	}
	else
	{
		if (Constants::CAM_PARAM_CHECK(pParam, 5))
		{
			SetNextPhiLerp(::Constants::JET_DEG2RAD(pParam->m_fPhiAngle));
		}
		if (Constants::CAM_PARAM_CHECK(pParam, 6))
		{
			SetNextDistance(pParam->m_fCamDistance);
		}
	}
	if (bInit)
	{
		Vector4 v2 = Vector4::zero;
		::Constants::jetGeoVecSet(v2, 0.0f, 0.0f, m_fDistance, 1.0f);
		::Constants::jetGeoVecRotX(v2, v2, 0.0f - m_fPhiAngle);
		m_fThetaAngleOld = m_fThetaAngle;
		m_fFollowHeight = v2[1];
		m_fDistanceOld = v2[2];
		::Constants::jetGeoVecSet(m_vPosLocal, 0.0f, m_fFollowHeight, m_fDistanceOld, 1.0f);
		::Constants::jetGeoVecRotY(m_vPosLocal, m_vPosLocal, m_fThetaAngleOld);
	}
	if (nType() == 0)
	{
		m_bInputFixed = false;
	}
}

void cTaskCamera::ChangeArea(int area)
{
	if (std::getenv("MEGAMIX_SCENE_DEBUG")) std::fprintf(stderr, "camera: ChangeArea %d\n", area);
	string key = string("w") + std::to_string(KingdomGlobal::PacnRollSys->nWorld() + 1) + "_st" + std::to_string(KingdomGlobal::PacnRollSys->nStage() + 1) + "_area" + CSharpCompat::ToString(area, "00") + ".csv";
	if (nullptr != ResourceDatabase::Get<CSVParam>(key))
	{
		(new cCameraParam(key))->GetParam(m_NowParam);
		cCameraParam::MergeParam(m_NowParam, m_DefParam);
	}
	else
	{
		m_NowParam = m_DefParam->Clone();
	}
	RegistCameraData(m_NowParam, !m_bInitParam);
	m_nNowArea = area;
	m_bInitParam = true;
}

void cTaskCamera::InitAreaParam()
{
	cCameraParam::InitStandardParam();
	string key = string("w") + std::to_string(KingdomGlobal::PacnRollSys->nWorld() + 1) + "_st" + std::to_string(KingdomGlobal::PacnRollSys->nStage() + 1) + "_area00.csv";
	if (nullptr != ResourceDatabase::Get<CSVParam>(key))
	{
		(new cCameraParam(key))->GetParam(m_DefParam);
		cCameraParam::MergeParam(m_DefParam, cCameraParam::GetStandardParam());
	}
	else
	{
		m_DefParam = cCameraParam::GetStandardParam()->Clone();
	}
	RegistCameraData(m_DefParam, false);
	m_NowParam = m_DefParam->Clone();
	if (Constants::PACMAN_OBJ() != nullptr)
	{
		::Constants::jetGeoVecCopy(m_vTarget, Constants::PACMAN_OBJ()->vTrans());
	}
	m_nNowArea = 999;
}

void cTaskCamera::SetCameraType(uint cameraType)
{
	nType() = cameraType;
	switch (cameraType)
	{
	default:
		return;
	case 1u:
		if (Constants::CAM_PARAM_CHECK(m_NowParam, 15))
		{
			SetNextPhiLerp(::Constants::JET_DEG2RAD(m_NowParam->m_fGoalPhi));
		}
		if (Constants::CAM_PARAM_CHECK(m_NowParam, 16))
		{
			SetNextDistance(m_NowParam->m_fGoalDist);
		}
		break;
	case 3u:
		break;
	case 0u:
	case 2u:
		return;
	}
	if (!KingdomGlobal::PacnRollSys->IsRetry() && Constants::CAM_PARAM_CHECK(m_NowParam, 19))
	{
		m_fThetaAngle = ::Constants::JET_DEG2RAD(m_NowParam->m_fStartAngle);
	}
}

void cTaskCamera::ControlFreeMenu()
{
	if (kingdom::Constants::IS_PAUSE())
	{
		return;
	}
	InGamePadManager::GetPad1No();
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	::Constants::jetGeoVecCopy(v, m_vMenuCamera);
	Matrix4x4 m;
	::Constants::jetGeoMtxIdentity(m);
	Vector4 v2;
	::Constants::jetGeoVecNormalize(v2, v);
	Vector4 v3;
	::Constants::jetGeoVecOuterProduct(v3, v2, m_vUp);
	::Constants::jetGeoVecNormalize(v3, v3);
	if (KingdomGlobal::SysCore->GetJobNo() != 146800640)
	{
		::Constants::jetGeoMtxRotY(m, m, Mathf::Atan2(0.0f - m_vTarget[0], 0.0f - m_vTarget[2]));
	}
	::Constants::jetGeoVecTrans(m_vPosLocal, m, v);
	float num = 0.0f;
	num = m_fMenuZoom;
	if (CustomKingdomUtil::IF(num))
	{
		::Constants::jetGeoVecCopy(v, m_vPosLocal);
		::Constants::jetGeoVecNormalize(v2, v);
		float num2 = ::Constants::jetGeoVecLength(v);
		if (0.0f > num && 2.0f > num2 + num)
		{
			::Constants::jetGeoVecScale(m_vPosLocal, v2, 2.0f);
		}
		else
		{
			Vector4 v4;
			::Constants::jetGeoVecScale(v4, v2, num);
			::Constants::jetGeoVecAdd(m_vPosLocal, v, v4);
		}
	}
	::Constants::jetGeoVecAdd(m_vPos, m_vTarget, m_vPosLocal);
}

void cTaskCamera::ThetaAngleLerp()
{
	m_fThetaSpeed += m_fThetaAccel;
	if (m_fThetaAngle < m_fNextTheta)
	{
		m_fThetaAngle += m_fThetaSpeed;
		if (m_fThetaAngle >= m_fNextTheta)
		{
			m_fThetaAngle = m_fNextTheta;
			m_bThetaLerp = false;
		}
	}
	else
	{
		m_fThetaAngle -= m_fThetaSpeed;
		if (m_fThetaAngle <= m_fNextTheta)
		{
			m_fThetaAngle = m_fNextTheta;
			m_bThetaLerp = false;
		}
	}
}

void cTaskCamera::PhiAngleLerp()
{
	m_fPhiSpeed += m_fPhiAccel;
	if (m_fPhiAngle < m_fNextPhi)
	{
		m_fPhiAngle += m_fPhiSpeed;
		if (m_fPhiAngle >= m_fNextPhi)
		{
			m_fPhiAngle = m_fNextPhi;
			m_bPhiLerp = false;
		}
	}
	else
	{
		m_fPhiAngle -= m_fPhiSpeed;
		if (m_fPhiAngle <= m_fNextPhi)
		{
			m_fPhiAngle = m_fNextPhi;
			m_bPhiLerp = false;
		}
	}
}

void cTaskCamera::DistanceLerp()
{
	m_fDistSpeed += m_fDistAccel;
	if (m_fDistance < m_fNextDist)
	{
		m_fDistance += m_fDistSpeed;
		if (m_fDistance >= m_fNextDist)
		{
			m_fDistance = m_fNextDist;
			m_bDistLerp = false;
		}
	}
	else
	{
		m_fDistance -= m_fDistSpeed;
		if (m_fDistance <= m_fNextDist)
		{
			m_fDistance = m_fNextDist;
			m_bDistLerp = false;
		}
	}
}

void cTaskCamera::MakeCamPositionRT()
{
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecSet(v, 0.0f, 0.0f, m_fDistance, 1.0f);
	::Constants::jetGeoVecRotX(v, v, 0.0f - m_fPhiAngle);
	::Constants::jetGeoVecRotY(v, v, m_fThetaAngle);
	::Constants::jetGeoVecAdd(m_vPos, m_vTarget, v);
}

void cTaskCamera::FrameMovePurSuit()
{
	if (Constants::IS_PAC_PAUSE() || Constants::IS_NAKAJIMA_PAUSE() || Constants::PACMAN_OBJ() == nullptr)
	{
		return;
	}
	m_fOldThetaAngle = m_fThetaAngle;
	::Constants::jetGeoVecCopy(m_vPrevPos, m_vPos);
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACMAN_OBJ()->vTrans());
	Vector4 v2;
	::Constants::jetGeoVecSub(v2, v, m_vTarget);
	float num = ::Constants::jetGeoVecLength(v2);
	float num2 = Constants::PACMAN_OBJ()->CalcSpeed();
	float num3 = num2 / 0.35f;
	num3 = ((num3 > 1.0f) ? 1.0f : num3);
	if (m_bStopCamera)
	{
		if (num >= m_fWaitDistance)
		{
			::Constants::jetGeoVecCopy(m_vNextTarget, v);
		}
		m_bStopCamera = false;
	}
	else
	{
		::Constants::jetGeoVecCopy(m_vNextTarget, v);
	}
	Vector4 v3;
	::Constants::jetGeoVecCopy(v3, Constants::PACMAN_OBJ()->vMove());
	Vector4 v4 = Vector4();
	float num4 = Mathf::Sin(m_fThetaAngle);
	float num5 = Mathf::Cos(m_fThetaAngle);
	v4[0] = m_fDistance * num4 + m_vTarget[0];
	v4[1] = 0.0f;
	v4[2] = m_fDistance * num5 + m_vTarget[2];
	v4[3] = 1.0f;
	Vector4 v5;
	::Constants::jetGeoVecSub(v5, v, v4);
	float value = (v5[1] = 0.0f);
	v2[1] = value;
	if (Constants::IsVecNormalize(v2))
	{
		::Constants::jetGeoVecNormalize(v2, v2);
	}
	if (Constants::IsVecNormalize(v5))
	{
		::Constants::jetGeoVecNormalize(v5, v5);
	}
	float num7 = v2[0] * v5[0] + v2[2] * v5[2];
	::Constants::jetGeoVecSub(v2, m_vNextTarget, m_vTarget);
	Vector4 v6;
	::Constants::jetGeoVecScale(v6, v2, m_fAtDelay);
	Vector4 v7;
	if (!m_bRecoAngle)
	{
		::Constants::jetGeoVecScale(v5, v2, m_fEyeDelay);
		::Constants::jetGeoVecAdd(v7, m_vTarget, v5);
	}
	else
	{
		::Constants::jetGeoVecSet(v2, 0.0f, 0.0f, 1.0f, 1.0f);
		::Constants::jetGeoVecRotY(v2, v2, m_fRecoAngle + (float)std::numbers::pi);
		::Constants::jetGeoVecScale(v5, v2, m_fRecoPower);
		::Constants::jetGeoVecAdd(v7, m_vTarget, v5);
	}
	::Constants::jetGeoVecAdd(m_vTarget, m_vTarget, v6);
	::Constants::jetGeoVecSub(v2, m_vPos, v7);
	if (m_bThetaLerp)
	{
		ThetaAngleLerp();
	}
	else if (num2 > 0.0f && CustomKingdomUtil::IF(v2[2]) && (CustomKingdomUtil::IF(v3[0]) || CustomKingdomUtil::IF(v3[2])))
	{
		float num8 = v2[0] / v2[2];
		if (num8 == 1.0f || num8 == -1.0f)
		{
			if (v2[0] > 0.0f)
			{
				v2[0] = 1.0f;
			}
			else if (v2[0] < 0.0f)
			{
				v2[0] = -1.0f;
			}
			if (v2[2] > 0.0f)
			{
				v2[2] = 1.0f;
			}
			else if (v2[2] < 0.0f)
			{
				v2[2] = -1.0f;
			}
		}
		float fThetaAngleOld = Mathf::Atan2(v2[0], v2[2]);
		m_fThetaAngle = (m_fThetaAngleOld = fThetaAngleOld);
	}
	FrameMoveCameraOpe();
	MakeCamPositionRT();
	if (num7 < -3932.0f)
	{
		(void)(m_fEyeDelay);
		(void)(40.0f);
	}
	if (m_bPhiLerp)
	{
		PhiAngleLerp();
	}
	if (m_bDistLerp)
	{
		DistanceLerp();
	}
	m_bNowFrameCameraHit = 0u;
	if (!devtools::G03Freecam()) HitProcess();   // devtools: --g03-freecam (no wall escape turns)
	MakeCamPositionRT();
}

void cTaskCamera::FrameMoveCameraOpe()
{
	float pAngle = m_fThetaAngle;
	int pad1No = InGamePadManager::GetPad1No();
	if (m_bPrevFrameCameraHit != 2 || devtools::G03Freecam())   // devtools: --g03-freecam
	{
		if (!Constants::PACN_APP()->pTaskConsRetry()->IsVisible())
		{
			// DOL 0x80158b08: hold & 8 (RIGHT) -> theta += 2.4 deg, else hold & 4 (LEFT) -> theta -= 2.4 deg (the C# had the two directions the other way round for its stick)
			if (GetPadCnt(pad1No, cTaskCamera::EKeyType::eRIGHT))
			{
				pAngle += ::Constants::JET_DEG2RAD(2.4f);
			}
			else if (GetPadCnt(pad1No, cTaskCamera::EKeyType::eLEFT))
			{
				pAngle -= ::Constants::JET_DEG2RAD(2.4f);
			}
			Vector4 _discard3;
			if (!devtools::G03Freecam() && GetPadCnt(pad1No, cTaskCamera::EKeyType::eLEFT_RIGHT) && CheckBGHit(pAngle, _discard3))   // devtools: --g03-freecam
			{
				pAngle = m_fOldThetaAngle;
				m_bCameraKeepOut = true;
			}
			float _discard4;
			if (!devtools::G03Freecam() && GetPadCnt(pad1No, cTaskCamera::EKeyType::eLEFT_RIGHT) && CheckExRange(_discard4, pAngle, ::Constants::JET_DEG2RAD(2.4f)))
			{
				pAngle = m_fOldThetaAngle;
				m_bCameraKeepOut = true;
			}
		}
	}
	else if (!Constants::PACN_APP()->pTaskConsRetry()->IsVisible() && GetPadCnt(pad1No, cTaskCamera::EKeyType::eLEFT_RIGHT))
	{
		m_bCameraKeepOut = true;
	}
	if (!devtools::G03Freecam() && CheckExRange(pAngle, pAngle, ::Constants::JET_DEG2RAD(2.4f)))   // devtools: --g03-freecam
	{
		m_bCameraKeepOut = true;
	}
	m_fThetaAngle = pAngle;
	if (m_fThetaAngle < -(float)std::numbers::pi)
	{
		m_fThetaAngle += (float)std::numbers::pi * 2.0f;
	}
	if (m_fThetaAngle > (float)std::numbers::pi)
	{
		m_fThetaAngle -= (float)std::numbers::pi * 2.0f;
	}
}

bool cTaskCamera::CheckExRange(float& pAngle, float fAngle, float fPower)
{
	pAngle = fAngle;
	bool flag = false;
	for (int i = 0; i < 2; i++)
	{
		if (m_fExRange[i][0] != m_fExRange[i][1])
		{
			float num = m_fExRange[i][0];
			float num2 = m_fExRange[i][1];
			num2 -= num;
			fAngle -= num;
			for (; num2 < 0.0f; num2 += (float)std::numbers::pi * 2.0f)
			{
			}
			while (num2 > (float)std::numbers::pi * 2.0f)
			{
				num2 -= (float)std::numbers::pi * 2.0f;
			}
			while (fAngle < 0.0f)
			{
				fAngle += (float)std::numbers::pi * 2.0f;
			}
			while (fAngle > (float)std::numbers::pi * 2.0f)
			{
				fAngle -= (float)std::numbers::pi * 2.0f;
			}
			if (fAngle >= 0.0f && fAngle <= num2)
			{
				float num3 = num2 / 2.0f;
				fAngle = ((!(fAngle < num3)) ? (fAngle + fPower) : (fAngle - fPower));
				flag = true;
				fAngle += num;
				break;
			}
		}
	}
	while (fAngle > (float)std::numbers::pi)
	{
		fAngle -= (float)std::numbers::pi * 2.0f;
	}
	while (fAngle < -(float)std::numbers::pi)
	{
		fAngle += (float)std::numbers::pi * 2.0f;
	}
	if (flag)
	{
		pAngle = fAngle;
	}
	return flag;
}

bool cTaskCamera::CheckBGHit(float fAngle, Vector4& vCross)
{
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecSet(v, 0.0f, 0.0f, m_fDistance, 1.0f);
	::Constants::jetGeoVecRotX(v, v, 0.0f - m_fPhiAngle);
	::Constants::jetGeoVecRotY(v, v, fAngle);
	::Constants::jetGeoVecAdd(v, m_vTarget, v);
	::Constants::jetGeoVecSub(v, v, m_vTarget);
	Constants::sFace* _discard5;
	cTaskBgHit* _discard6;
	if (Constants::PACN_APP()->pTaskCollision()->CheckVectorSimple(3u, m_vTarget, v, vCross, _discard5, _discard6))
	{
		return true;
	}
	return false;
}

bool cTaskCamera::HitProcess()
{
	int pad1No = InGamePadManager::GetPad1No();
	bool flag = false;
	Vector4 v;
	::Constants::jetGeoVecSub(v, m_vPos, m_vTarget);
	float num = ::Constants::bizGeoVecLength2(v);
	float outDist;
	Vector4 outCross;
	Constants::sFace* outFace;
	cTaskBgHit* outObj;
	if (Constants::PACN_APP()->pTaskCollision()->CheckVector(3u, m_vTarget, v, outDist, outCross, outFace, outObj))
	{
		if (num >= outDist)
		{
			fEscAngle = 0.03834952f;
			m_bNowFrameCameraHit = 1u;
			Vector4 v2;
			::Constants::jetGeoVecTrans(v2, outObj->pMtxRotate(), outFace->vNorm);
			Vector4 v3;
			::Constants::jetGeoVecSub(v3, m_vTarget, m_vPos);
			Vector4 v4;
			::Constants::jetGeoVecSub(v4, m_vTarget, outCross);
			float num2 = ::Constants::jetGeoVecInnerProduct(v2, v3);
			float num3 = ::Constants::jetGeoVecInnerProduct(v2, v4);
			float num4 = num2 - num3;
			if (num4 > 1.0f)
			{
				m_bNowFrameCameraHit = 2u;
				flag = true;
				num4 = 1.0f;
			}
			if (!CustomKingdomUtil::IF(m_nEscapeDir))
			{
				::Constants::jetGeoVecScale(v2, v2, num4);
				::Constants::jetGeoVecAdd(m_vPos, m_vPos, v2);
				::Constants::jetGeoVecSub(v, m_vPos, m_vTarget);
				float num5 = Mathf::Atan2(v[0], v[2]);
				if (flag)
				{
					if (num5 - m_fThetaAngle > 0.0f)
					{
						m_nEscapeDir = 1;
						SetNextThetaLerp(m_fThetaAngle + fEscAngle, m_bPrevFrameCameraHit == 2);
					}
					else
					{
						m_nEscapeDir = -1;
						SetNextThetaLerp(m_fThetaAngle - fEscAngle, m_bPrevFrameCameraHit == 2);
					}
					m_fThetaAngle += (float)m_nEscapeDir * ::Constants::JET_DEG2RAD(0.2f);
				}
				else
				{
					m_fThetaAngle = num5;
				}
			}
			else
			{
				SetNextThetaLerp(m_fThetaAngle + (float)m_nEscapeDir * fEscAngle, m_bPrevFrameCameraHit == 2);
			}
			if (GetPadCnt(pad1No, cTaskCamera::EKeyType::eLEFT_RIGHT))
			{
				m_bCameraKeepOut = true;
			}
		}
	}
	else
	{
		m_bThetaLerp = false;
	}
	if (!CustomKingdomUtil::IF(m_bNowFrameCameraHit))
	{
		return false;
	}
	return true;
}

// DOL 0x80159428 (fRadius unused, as in the DOL)
bool cTaskCamera::HitViewVolume(Vector4 vCenter, float fRadius)
{
	float fClipFarDist = m_fClipFarDist;
	Vector4 v;
	::Constants::jetGeoVecSub(v, vCenter, m_vPos);
	float num = ::Constants::jetGeoVecInnerProduct(v, m_vClipVec);
	if (num > 0.0f && num < fClipFarDist)
	{
		Vector4 v2;
		::Constants::jetGeoVecSub(v2, vCenter, m_vTarget);
		if (::Constants::jetGeoVecInnerProduct(v2, v2) < 225.0f)
		{
			return true;
		}
		float num2 = ::Constants::jetGeoVecInnerProduct(v, m_vClipVecOut);
		if (::Constants::JET_ABS(num / num2) < m_fClipValue)
		{
			return false;
		}
		return true;
	}
	return false;
}

void cTaskCamera::FrameMoveGameClearCamera()
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::IS_NAKAJIMA_PAUSE() && Constants::PACMAN_OBJ() != nullptr)
	{
		Vector4 v;
		::Constants::jetGeoVecSub(v, Constants::PACMAN_OBJ()->vTrans(), m_vTarget);
		::Constants::jetGeoVecLength(v);
		Vector4 v2;
		::Constants::jetGeoVecCopy(v2, Constants::PACMAN_OBJ()->vTrans());
		::Constants::jetGeoVecSub(v, v2, m_vTarget);
		Vector4 v3;
		::Constants::jetGeoVecScale(v3, v, m_fAtDelay);
		::Constants::jetGeoVecAdd(m_vTarget, m_vTarget, v3);
		m_fThetaAngle += ::Constants::JET_DEG2RAD(m_NowParam->m_fGoalSpeed);
		if (m_bPhiLerp)
		{
			PhiAngleLerp();
		}
		if (m_bDistLerp)
		{
			DistanceLerp();
		}
		MakeCamPositionRT();
	}
}

void cTaskCamera::FrameMoveFreeFall()
{
	if (Constants::PACMAN_OBJ() != nullptr)
	{
		Vector4 v;
		::Constants::jetGeoVecSub(v, Constants::PACMAN_OBJ()->vTrans(), m_vTarget);
		::Constants::jetGeoVecLength(v);
		Vector4 v2;
		::Constants::jetGeoVecCopy(v2, Constants::PACMAN_OBJ()->vTrans());
		::Constants::jetGeoVecSub(v, v2, m_vTarget);
		Vector4 v3;
		::Constants::jetGeoVecScale(v3, v, m_fAtDelay);
		::Constants::jetGeoVecAdd(m_vTarget, m_vTarget, v3);
	}
}

void cTaskCamera::FrameMoveInitGame()
{
	if (Constants::PACMAN_OBJ() != nullptr)
	{
		::Constants::bizMathNearRad(m_fThetaAngle, m_fThetaAngle, ::Constants::JET_DEG2RAD(m_NowParam->m_fInitAngle), ::Constants::JET_DEG2RAD(m_NowParam->m_fStartSpeed));
		MakeCamPositionRT();
	}
}

// DOL 0x80158b08 (FrameMoveCameraOpe) and friends read get_pad1(0)->hold: bit 0x4 (LEFT) / 0x8 (RIGHT), 0xc for either. The C# read the right stick
// (eKEY_RSTICK_*), which a Wii pad does not have (and the port does not bind): the camera turns with the D-pad (arrows).
// Native addition (not in the DOL): a gamepad's right stick pushed past half travel counts as the D-pad direction it points to, so the camera
// turns with either (stick right = RIGHT, stick up = UP).
bool cTaskCamera::GetPadCnt(int padNum, cTaskCamera::EKeyType type)
{
	unsigned hold = GamePad::Get(padNum).hold;
	const Vector2 rstick = InGamePadManager::GetStick(padNum, PadDefine::scRSTICK, false);
	if (rstick.x <= -0.5f)
	{
		hold |= GamePad::PAD_LEFT;
	}
	if (rstick.x >= 0.5f)
	{
		hold |= GamePad::PAD_RIGHT;
	}
	if (rstick.y >= 0.5f)
	{
		hold |= GamePad::PAD_UP;
	}
	if (rstick.y <= -0.5f)
	{
		hold |= GamePad::PAD_DOWN;
	}
	switch (type)
	{
	case cTaskCamera::EKeyType::eUP:
		return (hold & GamePad::PAD_UP) != 0;
	case cTaskCamera::EKeyType::eDOWN:
		return (hold & GamePad::PAD_DOWN) != 0;
	case cTaskCamera::EKeyType::eLEFT:
		return (hold & GamePad::PAD_LEFT) != 0;
	case cTaskCamera::EKeyType::eRIGHT:
		return (hold & GamePad::PAD_RIGHT) != 0;
	case cTaskCamera::EKeyType::eLEFT_RIGHT:
		return (hold & (GamePad::PAD_LEFT | GamePad::PAD_RIGHT)) != 0;
	default:
		return false;
	}
}

}
