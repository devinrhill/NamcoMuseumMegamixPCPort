#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskCamera : public cTaskKingdom
{
public:
	enum EType
	{
		eTYPE_FREE = 0,
		eTYPE_GAMECLEAR = 1,
		eTYPE_FREEFALL = 2,
		eTYPE_INITGAME = 3,
		eTYPE_MAX = 4
	};

private:
	enum EKeyType
	{
		eUP = 0,
		eDOWN = 1,
		eLEFT = 2,
		eRIGHT = 3,
		eLEFT_RIGHT = 4
	};

public:
	static constexpr float ZOOM_MIN = 2.0f;

	static constexpr float ZOOM_ADD = 1.0f;

	static float dot;

	static constexpr float rot0 = 0.007f;

	static constexpr float rot1 = 0.028f;

	static constexpr uint wait = 180u;

	static constexpr uint cnt0 = 2u;

	static constexpr uint cnt1 = 1u;

	static constexpr uint cnt2 = 1u;

	static constexpr uint cnt3 = 100u;

	static constexpr uint waitMax = 240u;

	static constexpr float rot2 = 0.03f;

private:
	uint m_nCamIdx{};

	uint m_nAnmIdx{};

	Vector4 m_vPos{};

	Vector4 m_vPosLocal{};

	Vector4 m_vTarget{};

	float m_fTwist{};

	Vector4 m_vUp{};

	Vector4 m_vPrevPos{};

	Vector4 m_vPrevTarget{};

	uint m_nType{};

	float m_fFollowRot{};

	uint m_nBackWaitCnt{};

	float m_fFollowHeight{};

	Vector4 m_vNextFocus{};

	bool m_bInputFixed{};

	float m_fNowDistance{};

	float m_fHeight{};

	int m_nNowArea{};

	bool m_bInitParam{};

	Constants::CAM_PARAM* m_DefParam{};

	Constants::CAM_PARAM* m_NowParam{};

	float m_fDistanceOld{};

	Vector4 m_vInitPos{};

	Vector4 m_vCheckPointPos{};

	float m_fFollowVal{};

	float m_fThetaAngleOld{};

	bool m_bStopCamera{};

	Vector4 m_vNextTarget{};

	float m_fWaitDistance{};

	float m_fAtDelay{};

	float m_fEyeDelay{};

	int m_nEscapeDir{};

	uint m_bNowFrameCameraHit{};

	uint m_bPrevFrameCameraHit{};

	bool m_bCameraKeepOut{};

	float m_fParamDistanceXZ{};

	float m_fParamHeight{};

	float m_fThetaAngle{};

	float m_fOldThetaAngle{};

	float m_fPhiAngle{};

	float m_fDistance{};

	bool m_bThetaLerp{};

	bool m_bPhiLerp{};

	bool m_bDistLerp{};

	float m_fNextTheta{};

	float m_fNextPhi{};

	float m_fNextDist{};

	float m_fThetaSpeed{};

	float m_fPhiSpeed{};

	float m_fDistSpeed{};

	float m_fThetaAccel{};

	float m_fPhiAccel{};

	float m_fDistAccel{};

	bool m_bRecoAngle{};

	float m_fRecoAngle{};

	float m_fRecoPower{};

	float m_fExRange[2][2]{};

	float m_fClipFarDist{};

	Vector4 m_vClipVec{};

	Vector4 m_vClipVecOut{};

	float m_fClipValue{};
	float m_fFovy{40.0f};   // the SCN0 camera (Setup); these defaults only if BIN_CAMERA_SCA cannot be read
	float m_fNear{0.1f};
	float m_fFar{32768.0f};

	float m_fMenuZoom{};

	Vector4 m_vMenuCamera{};

	Camera* m_mainCam{};

	GameObject* m_gazePoint{};

	static float fEscAngle;

public:
	Camera* get_MainCam();

	static float FX32TOF32(float val);

	void SetNextThetaLerp(float fAngle, bool rflg);

	void SetNextPhiLerp(float fAngle);

	void SetNextDistance(float fDist);

	bool bCameraKeepOut();

	void SetTarget(Vector4 v);

	uint& nType();

	Vector4& vPos();

	float vPos(uint n);

	Vector4& vTarget();

	float vTarget(uint n);

	void SetCamIdx(uint camIdx);

	uint GetCamIdx();

	void RegistFirstPacPos(Vector4 vPos);

	float& fMenuZoom();

	Vector4& vMenuCamera();

	float vMenuCamera(uint n);

	cTaskCamera();

	virtual ~cTaskCamera();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void Setup(string key, uint camIdx = 0u, uint anmIdx = 0u, float frame = 0.0f);

	void Update();

	void ControlFree();

	void ControlFollow();

	void RegistAnmScn(string key);

	uint GetAnmNum();

	void SetAnmIdx(uint anmIdx);

	uint GetAnmIdx();

	void SetFrame(float frame);

	float GetFrame();

	float GetFrameMax();

	void SetUpdateRate(float rate);

	float GetUpdateRate();

	void UpdateFrame();

	void RegistCameraData(Constants::CAM_PARAM* pParam, bool bInit);

	void ChangeArea(int area);

	void InitAreaParam();

	void SetCameraType(uint cameraType);

	void ControlFreeMenu();

	void ThetaAngleLerp();

	void PhiAngleLerp();

	void DistanceLerp();

private:
	void MakeCamPositionRT();

public:
	void FrameMovePurSuit();

private:
	void FrameMoveCameraOpe();

	bool CheckExRange(float& pAngle, float fAngle, float fPower);

	bool CheckBGHit(float fAngle, Vector4& vCross);

public:
	bool HitProcess();

	bool HitViewVolume(Vector4 vCenter, float fRadius);

	void FrameMoveGameClearCamera();

	void FrameMoveFreeFall();

	void FrameMoveInitGame();

private:
	bool GetPadCnt(int padNum, cTaskCamera::EKeyType type);

protected:
	void ReleaseTerm() override { cTaskCamera::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
