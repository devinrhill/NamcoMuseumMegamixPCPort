#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g07
{

class cTaskBoss05 : public cTaskCharBoss
{
private:
	enum EMOExHit
	{
		MO_EXHIT_YOKOBOU = 0,
		MO_EXHIT_TATEBOU = 1,
		MO_EXHIT_DODAI = 2
	};

	enum EDirectionB5Step
	{
		DIRECTION_B5_STEP_FADEIN = 0,
		DIRECTION_B5_STEP_APPEAR = 1,
		DIRECTION_B5_STEP_EXPAND_INIT = 2,
		DIRECTION_B5_STEP_EXPAND_ROTE_INIT = 3,
		DIRECTION_B5_STEP_EXPAND_WAIT = 4,
		DIRECTION_B5_STEP_EXPAND = 5,
		DIRECTION_B5_STEP_EXPAND_END = 6,
		DIRECTION_B5_STEP_FOG_CLEAR = 7,
		DIRECTION_B5_STEP_TOP_APPEAR = 8,
		DIRECTION_B5_STEP_START_WAIT = 9,
		DIRECTION_B5_STEP_WAIT = 255
	};

	enum EDirectionB52LastStep
	{
		DIRECTION_B5_2LAST_STEP_SHAKE = 0,
		DIRECTION_B5_2LAST_STEP_WHITEOUT = 1,
		DIRECTION_B5_2LAST_STEP_EXPLOD = 2,
		DIRECTION_B5_2LAST_STEP_FADEOUT = 3,
		DIRECTION_B5_2LAST_STEP_FADEIN = 4,
		DIRECTION_B5_2LAST_STEP_WAIT = 255
	};

	enum EBouAnim
	{
		BOU_ANIM_LT2RT = 0,
		BOU_ANIM_RT2LT = 1,
		BOU_ANIM_DW2UP = 2,
		BOU_ANIM_UP2DW = 3,
		BOU_ANIM_EXPAND_TATE_YOKO = 4,
		BOU_ANIM_EXPAND_CENTER = 5,
		BOU_ANIM_CONTRACT_TATE_YOKO = 6,
		BOU_ANIM_CONTRACT_CENTER = 7
	};

	enum EMove
	{
		MOVE_CT = 0,
		MOVE_LD = 1,
		MOVE_LU = 2,
		MOVE_RU = 3,
		MOVE_RD = 4,
		MOVE_LD2 = 5,
		MOVE_LU2 = 6,
		MOVE_RU2 = 7,
		MOVE_RD2 = 8,
		MOVE_MAX = 9
	};

	enum EMOActionB5
	{
		MO_ACTION_B5_SEEK01 = 0,
		MO_ACTION_B5_SPAWN01 = 1,
		MO_ACTION_B5_MOVE01 = 2,
		MO_ACTION_B5_SEEK02 = 3,
		MO_ACTION_B5_SPAWN02 = 4,
		MO_ACTION_B5_MOVE02 = 5,
		MO_ACTION_B5_RUSH01 = 6,
		MO_ACTION_B5_RUSH02 = 7,
		MO_ACTION_B5_RUSH03 = 8,
		MO_ACTION_B5_MOVE03 = 9,
		MO_ACTION_B5_MORE_SPAWN = 10,
		MO_ACTION_B5_METEOR = 11,
		MO_ACTION_B5_MAX = 12
	};

	enum EMotosBS5
	{
		MOTOS_BS5_MDL_TATEBOU01 = 0,
		MOTOS_BS5_MDL_TATEBOU02 = 1,
		MOTOS_BS5_MDL_YOKOBOU01 = 2,
		MOTOS_BS5_MDL_YOKOBOU02 = 3,
		MOTOS_BS5_MDL_CENTER = 4,
		MOTOS_BS5_MDL_CENTERBOU01 = 5,
		MOTOS_BS5_MDL_CENTERBOU02 = 6,
		MOTOS_BS5_MDL_DAI = 7,
		MOTOS_BS5_MDL_RING = 8,
		MOTOS_BS5_MDL_TOP = 9,
		MOTOS_BS5_MDL_LIGHT = 10,
		MOTOS_BS5_ANM_TATEBOU01 = 11,
		MOTOS_BS5_ANM_TATEBOU02 = 12,
		MOTOS_BS5_ANM_YOKOBOU01 = 13,
		MOTOS_BS5_ANM_YOKOBOU02 = 14,
		MOTOS_BS5_ANM_CENTER = 15,
		MOTOS_BS5_ANM_RING = 16,
		MOTOS_BS5_ANM_CENTERBOU01 = 17,
		MOTOS_BS5_ANM_CENTERBOU02 = 18,
		MOTOS_BS5_ANM_DAI = 19,
		MOTOS_BS5_ANM_TOP = 20,
		MOTOS_BS5_ANM_LIGHT = 21
	};

public:
	enum EMOModeB5
	{
		MO_MODE_B5_APPEAR = 1,
		MO_MODE_B5_GOCENTER = 2,
		MO_MODE_B5_SEEK = 3,
		MO_MODE_B5_MOVE = 4,
		MO_MODE_B5_RUSH = 5,
		MO_MODE_B5_PIYO = 6,
		MO_MODE_B5_SPAWN = 7,
		MO_MODE_B5_MORE_SPAWN = 8,
		MO_MODE_B5_METEOR = 9,
		MO_MODE_B5_DEATH = 10,
		MO_MODE_B5_WAIT = 11
	};

private:
	class MO_BOU_MOT_INFO
	{
	public:
		float m_fMot{};

		float m_fRate{};

		int m_nModel{};

		int m_nDelay{};
	};

public:
	enum EPtkSpark
	{
		ePTK_SPARK_1 = 0,
		ePTK_SPARK_2 = 1,
		ePTK_MAX = 2
	};

	enum EModel
	{
		MODEL_CENT = 0,
		MODEL_DDAY = 1,
		MODEL_BODY = 2,
		MODEL_ZBOU = 3,
		MODEL_XBOU = 4,
		MODEL_RING = 5,
		MODEL_GLAS = 6,
		MODEL_GHOST1 = 7,
		MODEL_GHOST2 = 8,
		MODEL_GHOST3 = 9,
		MODEL_GHOST4 = 10,
		MODEL_MAX = 11
	};

	enum EAnmSlot
	{
		eANM_SLOT_HASHIRA = 0,
		eANM_SLOT_BALL = 1,
		eANM_SLOT_RING = eANM_SLOT_HASHIRA
	};

private:
	static constexpr int APPEAR_ZOOM_FRAME = 40;

	static constexpr float APPEAR_ZOOM_ADD = 0.0075000003f;

	static constexpr float WHITEOUT_NEAR_FOG = -30.0f;

	static constexpr float WHITEOUT_FAR_FOG = 30.0f;

	static constexpr int APPEAR_FRAME_FOG = 120;

	static constexpr int APPEAR_ADD_NEAR_FOG = 0;

	static constexpr int APPEAR_ADD_FAR_FOG = 0;

	static constexpr int DEATH_FRAME_FOG = 90;

	static constexpr float DEATH_ADD_NEAR_FOG = 1.0f / 3.0f;

	static constexpr float DEATH_ADD_FAR_FOG = -1.0f / 3.0f;

	static constexpr int DAMAGE_BASE_FOG_NEAR = 30;

	static constexpr int DAMAGE_BASE_FOG_FAR = 60;

	static constexpr float DAMAGE_FOG_NEAR = -30.0f;

	static constexpr float DAMAGE_FOG_FAR = 30.0f;

	static constexpr int EXPLODE_FRAME_FOG = 90;

	static constexpr float EXPLODE_ADD_NEAR_FOG = 0.0f;

	static constexpr float EXPLODE_ADD_FAR_FOG = 0.0f;

	static constexpr int SCALEUP_NUM = 3;

	static constexpr int PANEL_COMA_SPEED = 3;

	static constexpr int CENTER_DW_FRAME = 35;

	static constexpr int CENTER_UP_FRAME = 35;

	static constexpr int CENTER_UP_FRAME2 = 30;

	static constexpr int CENTER_RESTOR_UP_FRAME = 40;

	static constexpr float CENTER_PIYO_PUL2 = 0.15f;

	static constexpr float RING_DEC_SPEED = 0.05f;

	static constexpr float PILLAR_HEIGHT = 2.95f;

	static constexpr float RING_HEIGHT = 2.1f;

	static constexpr int SPAWN_TIME = 120;

	static constexpr int MOTEOR_NUM = 5;

	static constexpr int MORE_SPAWN_BOM_NUM = 5;

	static constexpr float BURIED_AVOID = 0.1f;

	static Vector4 s_vPos[9];

	static int g_HitDamage[6];

	static const Constants::MO_BOSS_MODE_INFO* s_MdeTbl[12];

	static constexpr int BOU_NUM = 3;

public:
	static constexpr int APEARDEMO_MODEL_START_Y = 60;

private:
	float m_fNowAccel{};

	float m_fNowMaxSpeed{};

	float m_fRushAccel{};

	float m_fRushSpeed{};

	int m_nPiyoFrame{};

	float m_fCenterPosY{};

	float m_fCenterSpeed{};

	int m_nFrameCnt{};

	float m_fPiyoDWSp{};

	int m_nRingHP{};

	float m_fPillarMot{};

	sbyte m_nHitFlash{};

	bool m_IsDaiAnim{};

	Constants::MO_RING_INFO* m_RingInfo{};

	cTaskBoss05::MO_BOU_MOT_INFO* m_BouMotInfo[3]{};

	cKingdomPtkHnd* m_pBossPtk[46]{};

	bool m_IsLastForm{};

	int m_nDeathCnt{};

	Vector4 m_vApearScale{};

	Vector4 m_vTempTrans{};

	float m_fLightMot{};

	float m_fLightMotAdd{};

	float m_fLightAmb{};

	float m_fLightAmbAdd{};

	float m_fBouHitPower{};

	float m_fDodaiHitPower{};

	float m_fDodaiHitSize{};

	float m_fSlopeMaxRad{};

	float m_fSlopeAccel{};

	float m_fTopAlpha{};

	float m_fScaleAcsel{};

	float m_fGoScale{};

	float m_fUpScale{};

	static constexpr float SLOPE_AC_MAX = 0.0004f;

	static constexpr float SLOPE_FLOOR_AC = 0.0001f;

	static constexpr float SLOPE_REGIST_AC = 2E-05f;

	static float fRotatePow;

	static float fMovePoints;

	static float fMovePoints2;

	static float DAMAGE_BASE_HP(cTaskCharBoss* boss);

	static float DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss);

	static float DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss);

	static void SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time);

	static void SET_MODE_PIYO(cTaskCharBoss* boss, int mode);

public:
	cTaskBoss05();

	float& fRushAccel();

	float& fRushSpeed();

	float& fRotSpeed();

	float& fNowAccel();

	float& fNowMaxSpeed();

	float& fCenterPosY();

	Constants::MO_RING_INFO*& pRingInfo();

	bool IsLastForm();

	void SetApearvPos(Vector4 v);

	void SetApearScale(float x, float y, float z);

	void HeadRotate(Vector4 v);

private:
	void Init();

public:
	void Term() override;

	void SparkFree();

	void SetRingDamageFrame();

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	void AdjustMaxSpeed() override;

	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

private:
	bool IsAllRingUP();

	bool CenterMove();

	void SetBouAnime(int kind, float rate, int delay = 0, bool force = false);

	ushort SearchPlayer();

public:
	float GetHitPower(int nPart, int nOtherID) override;

	void RefAttacked(int nParts, int nGrobalID) override;

private:
	void SlantFloor();

	static void SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step);

	static void SET_STEP(cTaskCharBoss* boss, ushort step);

	static void SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time);

	static void SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time);

	void ModeAppear();

	void ModeFoamChange();

	void ModeDeath();

	IEnumerator* WaitOneFramePlayBGM();
};

}
