#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cHitChar.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"

namespace kingdom::game::g03
{

class cTaskCharPacman : public cTaskChar
{
public:
	enum EType
	{
		TYPE_NORMAL = 0,
		TYPE_WING = 1,
		TYPE_KNIGHT = 2,
		TYPE_MAX = 3
	};

	enum ELimit
	{
		eHP_LIMIT = 7,
		eLIFE_LIMIT = 99
	};

	enum EProc
	{
		ePROC_START_WAIT = 0,
		ePROC_NORMAL = 1,
		ePROC_HANG = 2,
		ePROC_GEARS = 3,
		ePROC_MISS = 4,
		ePROC_HELL = 5,
		ePROC_GOAL = 6,
		ePROC_JOY = 7,
		ePROC_RETIRE = 8,
		ePROC_MAX = 9
	};

	enum EState
	{
		STATE_NORMAL = 0,
		STATE_DASH = 1,
		STATE_DASH_FLOOR = 2,
		STATE_EAT_POWER_COOKIE = 4,
		STATE_DAMAGE = 8,
		STATE_HANG = 0x10,
		STATE_BRAKE = 0x20,
		STATE_HIT_BG = 0x40,
		STATE_HELL = 0x80,
		STATE_MISS = 0x100,
		STATE_MISS_FINISH = 0x200,
		STATE_WATER_UP = 0x400,
		STATE_WATER_IN = 0x800,
		STATE_BOUND = 0x1000,
		STATE_GEARS = 0x2000,
		STATE_JUMP = 0x4000,
		STATE_CLIMB = 0x8000
	};

	enum EAnimeKind
	{
		eANIME_KIND_PACPAC = 0,
		eANIME_KIND_CLOSE = 1,
		eANIME_KIND_PACPAC_FAST = 2,
		eANIME_KIND_JOY = 3,
		eANIME_KIND_HAGHAG = 4,
		eANIME_KIND_MISS = 5,
		eANIME_KIND_DULL_YAWN = 6,
		eANIME_KIND_DULL_SOMERSAULT = 7,
		eANIME_KIND_BRAKE_ON = 8,
		eANIME_KIND_BRAKE_OFF = 9,
		eANIME_KIND_MAX = 10
	};

	enum EAnimeState
	{
		ANIME_STATE_NORMAL = 0,
		ANIME_STATE_HIT_WALL = 1,
		ANIME_STATE_DASH = 2,
		ANIME_STATE_EAT_POWER_COOKIE = 3,
		ANIME_STATE_DAMAGE = 4,
		ANIME_STATE_JUNMPING = 5,
		ANIME_STATE_JUNP = 6,
		ANIME_STATE_HANG = 7,
		ANIME_STATE_EAT_GHOST = 8,
		ANIME_STATE_STAGE_CLEAR = 9,
		ANIME_STATE_GAME_OVER = 10,
		ANIME_STATE_WAIT_A = 11,
		ANIME_STATE_WAIT_B = 12,
		ANIME_STATE_BRAKE = 13,
		ANIME_STATE_BRAKE_OFF = 14,
		ANIME_STATE_MAX = 15
	};

	enum EFace
	{
		eFACE_NORMAL = 0,
		eFACE_WINK = 1,
		eFACE_ANGRILY = 2,
		eFACE_PAIN = 3,
		eFACE_JOY = 4,
		eFACE_DIZZY = 5
	};

	enum EDash
	{
		eDASH_PARAM_PAC = 0,
		eDASH_PARAM_FLOOR = 1,
		eDASH_PARAM_MAX = 2
	};

	enum EDamage
	{
		DAMAGE_NORMAL = 0,
		DAMAGE_KNIGHT_LV1 = 1,
		DAMAGE_KNIGHT_LV2 = 2
	};

	enum EHitHistry
	{
		eHIT_HISTRY_MAX = 8
	};

	class sParam
	{
	public:
		float fPadForce{};

		float fSpeedMax{};

		float fSpeedMaxAir{};

		float fGravityFall{};

		float fGravityUp{};

		float fGravityDown{};

		float fReflect{};

		float fFriction{};

		float fFrictionBrake{};

		float fAirResist{};

		float fAirResistDash{};

		float fDashForce{};

		float fDashSpeedMax{};

		float fDashSpeedMin{};

		float fDashBound{};

		uint nDashTime{};

		uint nDashAccelTime{};

		float fHangSpeed{};

		float fBoundSpeed{};

		float fWaterResistXZ{};

		float fWaterResistY{};

		float fWaterLift{};

		float fBoundFloor{};

		float fBoundWall{};

		uint nDashFloorTime{};

		float fDashFloorAcc{};

		float fDashFloorSpeedMax{};

		float fSeesawMoment{};

		float fSeesawAccMax{};
	};

	class sParamCommon
	{
	public:
		float fPadSwingSpeedDash{};

		float fPadSwingSpeedEscapeHang{};

		float fHangHeight{};

		uint nWaitAnimeTime{};

		uint nDamageTime{};

		float fVacuumRange{};
	};

	class BGHIT_HISTORY
	{
	public:
		int m_nIndex{};

		cTaskBgHit* m_aData[8]{};
	};

private:
	static constexpr uint MOVE_GOAL_CNT = 60u;

	static constexpr float WATE_STOP_SPEED = 0.1f;

	static constexpr uint NO_WALL_BOUND_TIME = 6u;

	static constexpr uint NO_FLOOR_BOUND_TIME = 20u;

	static constexpr uint NO_DASH_TIME = 20u;

	static constexpr float BRAKE_VALUE = 0.1f;

	static constexpr float KNIGHT_SINK_SPEED = 0.06f;

	static constexpr float MISS_FALL_SPEED_LIMIT = 0.6f;

	static constexpr float MISS_FALL_TRANS_LIMIT = 2000.0f;

	static constexpr uint MISS_FREE_FALL_TIME = 40u;

	static constexpr float PTK_WATER_IN_BUBBLE_VALUE = 0.7f;

	string s_AnmChrList[10][3]{};

	static constexpr float PACMAN_RADIUS = 0.38f;

	float m_fStickX{};

	float m_fStickY{};

	Matrix4x4 m_mRoll{};

	Matrix4x4 m_mAnm{};

	Vector4 m_vHangPos{};

	Vector4 m_vHangVec{};

	Vector4 m_vTransOld2{};

	Vector4 m_vForceMove{};

	Vector4 m_vDashDir{};

	Vector4 m_vLastFloorPos{};

	Vector4 m_vJumpVec{};

	Vector4 m_vFloorDashVec{};

	Vector4 m_vAfterMove{};

	uint m_nAloneCnt{};

	cTaskCharPacman::sParam* m_Param[3]{};

	cTaskCharPacman::sParamCommon* m_ParamCommon{};

	uint m_nDashCnt{};

	uint m_nNoBoundCnt{};

	uint m_nNoDashCnt{};

	uint m_nWaterCount{};

	Vector4 m_vWaterForce{};

	float m_fWaterHeight{};

	bool m_bGameOver{};

	uint m_nEatComboCnt{};

	bool m_bGoal{};

	cTaskBgHit* m_pHitObj[4]{};

	Constants::sFace* m_pHitFace[4]{};

	std::vector<Constants::sHitParam*> m_vecHitParam[4];

	uint m_nDamageCnt{};

	uint m_nHitKindFace{};

	uint m_nEffWingCnt{};

	uint m_nFreeFallCnt{};

	uint m_nFloorDashCnt{};

	cTaskCharPacman::BGHIT_HISTORY* m_sHitObjHistry[4]{};

	uint m_nJumpCnt{};

	Constants::sFace* m_pHitFaceFloorOld{};

	cTaskBgHit* m_pHitObjFloorOld{};

	bool m_bRollCheck{};

	Vector4 m_vForceMoveStock{};

	bool m_bBrink{};

	uint m_nLife{};

	uint m_nHP{};

	uint m_nMaxHP{};

	uint m_nEatEnemy{};

	uint m_nAnmState{};

	uint m_nAnmCuurentIdx{};

	uint m_nAnmStateNext{};

	uint m_nAnmChangeTime{};

	uint m_nAnmKeepTime{};

	uint m_nTpaAnmKind{};

	AnimationClip* m_AnmChrMiss[3]{};

	AnimationClip* m_AnmChrBrakeOn[3]{};

	AnimationClip* m_AnmChrBrakeOff[3]{};

	float m_fAnmCharMissFrame{};

	float m_fAnmCharBrakeOnFrame{};

	float m_fAnmCharBrakeOffFrame{};

	bool m_bBrakeAnmMax{};

	cKingdomPtkHnd* m_PtkHndFlame{};

	cKingdomPtkHnd* m_PtkHndChange{};

	cKingdomPtkHnd* m_PtkHndSweat{};

	cKingdomPtkHnd* m_PtkHndBrake{};

	cKingdomPtkHnd* m_PtkHndPoson{};

	cKingdomPtkHnd* m_PtkHndDash{};

	cKingdomPtkHnd* m_PtkHndRipple{};

	cKingdomPtkHnd* m_PtkHndWing{};

	cKingdomPtkHnd* m_PtkHndPowerCookie{};

	cKingdomPtkHnd* m_PtkHndWaterIn{};

	int m_splashPtcCnt{};

	GameObject* m_SndHndMove{};

	GameObject* m_SndHndDash{};

	GameObject* m_SndHndDash2{};

	GameObject* m_SndHndInWater{};

	GameObject* m_SndHndInWater2{};

	GameObject* m_SndHndOnField{};

	GameObject* m_SndHndSwim{};

	GameObject* m_SndHndGrass{};

	GameObject* m_dummyAnimObj[3]{};

	Animation* m_dummyAnimation[3]{};

	Transform* m_dummyAnimObjRootNode[3]{};

	Transform* m_dummyAnimObjBaseNode[3]{};

	static void GET_PARAM_FLT(cKingdomCSV* csv, string name, int col, float& data);

	static void GET_PARAM_INT(cKingdomCSV* csv, string name, int col, int& data);

	static void GET_PARAM_UINT(cKingdomCSV* csv, string name, int col, uint& data);

public:
	Vector4& vTransOld2();

	float vTransOld2(int n);

	Vector4& vForceMove();

	float vForceMove(int n);

	Vector4& vLastFloorPos();

	float vLastFloorPos(int n);

	cTaskBgHit*& pHitObj(int n);

	Constants::sFace*& pHitFace(int n);

	uint& nLife();

	uint& nHP();

	uint& nMaxHP();

	uint& nEatEnemy();

	uint& nAloneCnt();

	uint& nEatComboCnt();

	cTaskCharPacman::sParam* GetParam();

	cTaskCharPacman::sParamCommon* GetParamCommon();

	bool IsGoal();

	std::vector<Constants::sHitParam*>* pHitList();

	cTaskCharPacman();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	void InitParam();

public:
	void CollisionFoceMove();

	void PostCollisionWall();

	void PostCollisionFloor();

	void PostCollisionWallFinish();

	void PostCollisionFloorFinish();

private:
	void SetBoundWall(Constants::sHitParam* const& hitParam, const Vector4& vNorm);

	void SetBoundFloor(Constants::sHitParam* const& hitParam, const Vector4& vNorm);

	void WaterForce();

	bool IsHang(Constants::sHitParam* hitParam);

	bool Hanging(bool flg = true);

	bool Hanging(bool flg, cTaskBgHit* hitObjFloor);

public:
	void SetType(const uint& type);

	void SetAnmState(const uint& state, const uint& time = 0u, const uint& keep = 0u);

	void SetAnm(string anime, const uint& idx = 0u);

	void SetAnmChr(const uint& kind, string anime, const uint& time = 0u, const uint& keep = 0u);

	bool Control();

private:
	void AdjustSpeed();

public:
	bool IsOverSpeed(const float& sp, const Nullable<Vector4>& move = nullptr);

	float CalcSpeed();

	void ChangePacn(uint kind);

private:
	bool MoveGoalCenter();

	void UpdateRollMatrix();

	bool ResetRollMatrix();

	void LookCameraRollMatrix();

	float Get2GameraRot();

public:
	void RegistHitParam(uint idx, Constants::sHitParam* param);

	void ClearHitParam();

	void SetRetryData();

	bool IsMiss();

	bool IsMissFinish();

	bool IsGameOver();

	bool IsGround();

	void Attacked(cHitChar* pRev, cHitChar* pAtk) override;

	virtual bool DamageProcess(uint nDamage, Vector4 vVec, float fSpeed, int nKnightDef = 0);

private:
	void BrakeAnime();

public:
	void RecoverHp(uint n = 0u);

	void GainHpLimit(uint n);

	void GainLife(uint n);

private:
	bool CheckFallArea();

	void SetMaterialAlpha(byte alpha);

public:
	void IjikeStart() override;

	void IjikeEnd() override;

	void RotateYPacMan(float rot);

	void DisposeEffect();

	void HitObjRegist(int nKind, cTaskBgHit* pOBJ);

	cTaskBgHit* HitObjGet(int nKind, int nIndex);

	void HitObjTick();

private:
	Vector2 GetPadLStickOrKey(int padNum);
	// Execute split by the port (one per EProc mode, NormalPlay = the play step of ePROC_NORMAL in blocks)
	void ProcStartWait();
	void ProcNormal();
	void ProcHang();
	void ProcGears();
	void ProcMiss();
	void ProcHell();
	void ProcGoal();
	void ProcJoy();
	void ProcRetire();
	void NormalPlay();
	bool NormalImpulse();
	void NormalCounters();
	void NormalIdle();
	void NormalWater();
	void NormalAirSound();
	void NormalDash(bool jumped);
	void NormalDashAccel();
	void NormalMoveEnd();


protected:
	void ReleaseTerm() override { cTaskCharPacman::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
