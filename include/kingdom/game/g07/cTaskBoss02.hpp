#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g07
{

class cTaskBoss02 : public cTaskCharBoss
{
private:
	enum EMOHoverKind
	{
		MO_HOVER_KIND_MOVE = 0,
		MO_HOVER_KIND_STOP = 1
	};

	enum EDirectionStep
	{
		DIRECTION_STEP_FADEIN = 0,
		DIRECTION_STEP_APPEAR_PILLAR = 1,
		DIRECTION_STEP_APPEAR_CENTER = 2,
		DIRECTION_STEP_APPEAR_LIGHT = 3,
		DIRECTION_STEP_START_WAIT = 4,
		DIRECTION_STEP_WAIT = 5
	};

	enum EMove
	{
		MOVE_CT = 0,
		MOVE_LT = 1,
		MOVE_RT = 2,
		MOVE_LU = 3,
		MOVE_LD = 4,
		MOVE_RU = 5,
		MOVE_RD = 6,
		MOVE_MAX = 7
	};

	enum EMOActionB2
	{
		MO_ACTION_B2_RANDOM01 = 0,
		MO_ACTION_B2_MOVE01 = 1,
		MO_ACTION_B2_SPAWN01 = 2,
		MO_ACTION_B2_SPAWN02 = 3,
		MO_ACTION_B2_RANDOM02 = 4,
		MO_ACTION_B2_MOVE02 = 5,
		MO_ACTION_B2_SPAWN03 = 6,
		MO_ACTION_B2_SPAWN04 = 7,
		MO_ACTION_B2_RANDOM03 = 8,
		MO_ACTION_B2_MOVE03 = 9,
		MO_ACTION_B2_SPAWN05 = 10,
		MO_ACTION_B2_SPAWN06 = 11,
		MO_ACTION_B2_RANDOM04 = 12,
		MO_ACTION_B2_MOVE04 = 13,
		MO_ACTION_B2_SPAWN07 = 14,
		MO_ACTION_B2_SPAWN08 = 15,
		MO_ACTION_B2_RANDOM05 = 16,
		MO_ACTION_B2_MOVE05 = 17,
		MO_ACTION_B2_SPAWN09 = 18,
		MO_ACTION_B2_SPAWN10 = 19,
		MO_ACTION_B2_MAX = 20
	};

	enum EMotosBS2
	{
		MOTOS_BS2_MDL_PILLAR = 0,
		MOTOS_BS2_MDL_CENTER = 1,
		MOTOS_BS2_MDL_RING = 2,
		MOTOS_BS2_MDL_LIGHT = 3,
		MOTOS_BS2_ANM_PILLAR = 4,
		MOTOS_BS2_ANM_CENTER = 5,
		MOTOS_BS2_ANM_RING = 6,
		MOTOS_BS2_ANM_LIGHT = 7,
		MOTOS_BS2_MAX = 8
	};

	enum EMOModeB2
	{
		MO_MODE_B2_APPEAR = 1,
		MO_MODE_B2_GOCENTER = 2,
		MO_MODE_B2_RANDOM = 3,
		MO_MODE_B2_MOVE = 4,
		MO_MODE_B2_PIYO = 5,
		MO_MODE_B2_SPAWN = 6,
		MO_MODE_B2_WAIT = 7,
		MO_MODE_B2_DEATH = 8
	};

public:
	enum EModel
	{
		MODEL_CENT = 0,
		MODEL_HASHI_L = 1,
		MODEL_HASHI_R = 2,
		MODEL_RING_L = 3,
		MODEL_RING_R = 4,
		MODEL_GLAS = 5,
		MODEL_GHOST = 6,
		MODEL_MAX = 7
	};

	enum EAnmSlot
	{
		eANM_SLOT_HASHIRA = 0,
		eANM_SLOT_BALL = 1,
		eANM_SLOT_RING = eANM_SLOT_HASHIRA
	};

	static constexpr int APPEAR_FRAME_XZ = 8;

	static constexpr int APPEAR_FRAME_Y = 5;

	static constexpr float APPEAR_ADD_XZ = 0.125f;

	static constexpr float APPEAR_ADD_Y = 0.2f;

	static constexpr int APPEAR_FRAME_FOG = 120;

	static constexpr float APPEAR_NEAR_FOG = -30.0f;

	static constexpr float APPEAR_FAR_FOG = 30.0f;

	static constexpr int DAMAGE_BASE_FOG_NEAR = 30;

	static constexpr int DAMAGE_BASE_FOG_FAR = 60;

	static constexpr float DAMAGE_FOG_NEAR = -30.0f;

	static constexpr float DAMAGE_FOG_FAR = 30.0f;

	static constexpr int EXPLODE_FRAME_FOG = 90;

	static constexpr float EXPLODE_ADD_NEAR_FOG = 0.0f;

	static constexpr float EXPLODE_ADD_FAR_FOG = 0.0f;

	static constexpr int CENTER_DW_FRAME = 35;

	static constexpr int CENTER_UP_FRAME = 20;

	static constexpr int CENTER_RESTOR_UP_FRAME = 40;

	static constexpr float CENTER_PIYO_PUL2 = 0.15f;

	static constexpr float RING_DEC_SPEED = 0.05f;

	static constexpr float PILLAR_HEIGHT = 2.0f;

	static constexpr int LAMP_BLINK = 90;

	static constexpr int PILLAR_NUM = 2;

	static constexpr float BURIED_AVOID = 0.1f;

	static constexpr int MOVE_SE_PITCH = 25;

	static constexpr int ROTE_SE_PITCH = 110;

private:
	static Vector4 s_vPos[7];

	static int g_HitDamage[6];

	static const Constants::MO_BOSS_MODE_INFO* s_MdeTbl[20];

	Vector4 m_vSTarget{};

	float m_fNowAccel{};

	float m_fNowMaxSpeed{};

	float m_fRotSpeed{};

	int m_nRandomFrame{};

	int m_nDownFrame{};

	int m_nPiyoFrame{};

	int m_nRestTime{};

	float m_fCenterPosY{};

	int m_nFrameCnt{};

	float m_fPiyoDWSp{};

	int m_nRingHP{};

	sbyte m_nHitFlash{};

	float m_fPillarMot[2]{};

	Constants::MO_RING_INFO* m_RingInfo[2]{};

	cKingdomPtkHnd* m_pHover[2]{};

	float m_fCenterSpeed{};

	float m_fRandomAccel{};

	float m_fRandomMaxSpeed{};

	bool m_IsCenterDown{};

	int m_nMoveSEPitch{};

	int m_nRoteSEPitch{};

	float m_fLightMot{};

	float m_fLightMotAdd{};

	float m_fLightAmb{};

	float m_fLightAmbAdd{};

	Vector4 m_vPillarScale{};

	byte m_nRingAlpha{};

	float m_fCameraSin{};

	float m_fCameraCos{};

	Vector4 m_vRandTarget{};

	static constexpr float LAMP_MOT_BURN_SPEED = 0.5f;

	static constexpr float LAMP_MOT_BLINK_SPEED = 1.0f;

	static constexpr float LAMP_MOT_TUK1 = 1.0f;

	static constexpr float LAMP_MOT_KIE1 = 5.0f;

	static constexpr float LAMP_MOT_TUK2 = 10.0f;

	static constexpr float LAMP_MOT_KIE2 = 15.0f;

public:
	static float get_APPEAR_ADD_NEAR_FOG();

	static float get_APPEAR_ADD_FAR_FOG();

	static float DAMAGE_BASE_HP(cTaskCharBoss* boss);

	static float DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss);

	static float DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss);

	static void SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time);

	static void SET_MODE_PIYO(cTaskCharBoss* boss, int mode);

	cTaskBoss02();

	Vector4& vRandTarget();

	float& fCenterPosY();

	Constants::MO_RING_INFO* pRingInfo(uint n);

	Vector4& vPillarScale();

	byte& nRingAlpha();

	cKingdomPtkHnd*& pHover(int n);

	float& fNowAccel();

	float& fNowMaxSpeed();

	float& fRandomAccel();

	float& fRandomMaxSpeed();

	void SetApearScale(float x, float y, float z);

private:
	void Init();

public:
	void Term() override;

	bool ActionBoth() override;

	void SetRingDamageFrame(int nRing);

	bool ActionLocal() override;

	bool ActionOther() override;

	void AdjustMaxSpeed() override;

private:
	void DrawStartDirection();

	void DrawEndDirection();

	void DrawInGame();

	void SpawnHover();

	void MoveSE();

	void MoveSEForce();

	void RoteSE();

public:
	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

private:
	Vector4 vSetRandTarget();

	bool IsRingUP(int n);

	bool IsAllRingUP();

	void LampBurn(int n);

	void LampBurn();

	void LampBlink(int n);

	void LampBlink();

	void LampDell(int n);

	void LampDell();

	void LampForceDell(int n);

	void LampForceDell();

public:
	void RefAttacked(int nParts, int nGrobalID) override;

private:
	static void SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step);

	static void SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time);

	static void SET_STEP(cTaskCharBoss* boss, ushort step);

	static void SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time);

	void ModeAppear();

	bool CenterMove();

	int NextMode();

	void SetNextMode();
};

}
