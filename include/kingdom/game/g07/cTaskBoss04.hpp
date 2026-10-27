#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g07
{

class cTaskBoss04 : public cTaskCharBoss
{
private:
	enum EDirectionStep
	{
		DIRECTION_STEP_FADEIN = 0,
		DIRECTION_STEP_APPEAR_PILLAR = 1,
		DIRECTION_STEP_APPEAR_CENTER = 2,
		DIRECTION_STEP_APPEAR_LIGHT = 3,
		DIRECTION_STEP_START_WAIT = 4,
		DIRECTION_STEP_WAIT = 5
	};

	enum EMOHoverKind
	{
		MO_HOVER_KIND_MOVE = 0,
		MO_HOVER_KIND_STOP = 1
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

	enum EMOActionB4
	{
		MO_ACTION_B4_RANDOM01 = 0,
		MO_ACTION_B4_MOVE03 = 1,
		MO_ACTION_B4_SPAWN07 = 2,
		MO_ACTION_B4_SPAWN08 = 3,
		MO_ACTION_B4_SPAWN09 = 4,
		MO_ACTION_B4_RANDOM04 = 5,
		MO_ACTION_B4_MOVE04 = 6,
		MO_ACTION_B4_SPAWN10 = 7,
		MO_ACTION_B4_SPAWN11 = 8,
		MO_ACTION_B4_SPAWN12 = 9,
		MO_ACTION_B4_RANDOM05 = 10,
		MO_ACTION_B4_MOVE05 = 11,
		MO_ACTION_B4_SPAWN13 = 12,
		MO_ACTION_B4_SPAWN14 = 13,
		MO_ACTION_B4_SPAWN15 = 14,
		MO_ACTION_B4_SLOPE01 = 15,
		MO_ACTION_B4_SLOPE02 = 16,
		MO_ACTION_B4_SLOPE03 = 17,
		MO_ACTION_B4_MAX = 18
	};

	enum EMotosBS4
	{
		MOTOS_BS4_MDL_PILLAR = 0,
		MOTOS_BS4_MDL_CENTER = 1,
		MOTOS_BS4_MDL_RING = 2,
		MOTOS_BS4_MDL_LIGHT = 3,
		MOTOS_BS4_ANM_PILLAR = 4,
		MOTOS_BS4_ANM_CENTER = 5,
		MOTOS_BS4_ANM_RING = 6,
		MOTOS_BS4_ANM_LIGHT = 7,
		MOTOS_BS4_MAX = 8
	};

public:
	enum EMOModeB4
	{
		MO_MODE_B4_APPEAR = 1,
		MO_MODE_B4_GOCENTER = 2,
		MO_MODE_B4_RANDOM = 3,
		MO_MODE_B4_MOVE = 4,
		MO_MODE_B4_SLOPE = 5,
		MO_MODE_B4_PIYO = 6,
		MO_MODE_B4_SPAWN = 7,
		MO_MODE_B4_WAIT = 8,
		MO_MODE_B4_DEATH = 9
	};

	enum EModel
	{
		MODEL_CENT = 0,
		MODEL_HASHI_UL = 1,
		MODEL_HASHI_UR = 2,
		MODEL_HASHI_DL = 3,
		MODEL_HASHI_DR = 4,
		MODEL_RING_UL = 5,
		MODEL_RING_UR = 6,
		MODEL_RING_DL = 7,
		MODEL_RING_DR = 8,
		MODEL_GLAS = 9,
		MODEL_GHORST = 10,
		MODEL_MAX = 11,
		MODEL_HASHI_ROTATE_START = MODEL_HASHI_UL,
		MODEL_HASHI_RING_START = MODEL_RING_UL,
		MODEL_ROTATE_NUM = MODEL_HASHI_DL
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

	static constexpr int PILLAR_NUM = 4;

	static constexpr float BURIED_AVOID = 0.1f;

	static constexpr int MOVE_SE_PITCH = 25;

	static constexpr int ROTE_SE_PITCH = 110;

private:
	static Vector4 s_vPos[7];

	static int g_HitDamage[6];

	static const Constants::MO_BOSS_MODE_INFO* s_MdeTbl[18];

	Vector4 m_vSTarget{};

	float m_fNowAccel{};

	float m_fNowMaxSpeed{};

	float m_fRotSpeed{};

	int m_nRandomFrame{};

	int m_nDownFrame{};

	int m_nPiyoFrame{};

	int m_nRestTime{};

	float m_fCenterPosY{};

	float m_fPiyoDWSp{};

	int m_nRingHP{};

	sbyte m_nHitFlash{};

	float m_fPillarMot[4]{};

	Constants::MO_RING_INFO* m_RingInfo[4]{};

	cKingdomPtkHnd* m_pHover[4]{};

	float m_fCenterSpeed{};

	float m_fRandomAccel{};

	float m_fRandomMaxSpeed{};

	float m_fSlopeAccel{};

	float m_fSlopeMaxSpeed{};

	int m_nMoveSEPitch{};

	int m_nRoteSEPitch{};

	float m_fLightMot{};

	float m_fLightMotAdd{};

	float m_fLightAmb{};

	float m_fLightAmbAdd{};

	Vector4 m_vPillarScale{};

	byte m_nRingAlpha{};

	int m_nSlopeTime{};

	int m_nSlopeStopTime{};

	float m_fSlopeRadMax{};

	float m_fAddSlopeRad{};

	float m_fAccelRotSpeed{};

	float m_fMaxAccelRotSpeed{};

	float m_fAddAccelRotSpeed{};

	int m_nAccelRotTime{};

	Vector4 m_vTarget{};

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

private:
	static void SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time);

	static void SET_MODE_PIYO(cTaskCharBoss* boss, int mode);

public:
	cTaskBoss04();

	Vector4& vTarget();

	Constants::MO_RING_INFO* pRingInfo(uint n);

	float& fCenterPosY();

	Vector4& vPillarScale();

	byte& nRingAlpha();

	cKingdomPtkHnd*& pHover(int n);

	float& fNowAccel();

	float& fNowMaxSpeed();

	float& fRandomAccel();

	float& fRandomMaxSpeed();

	float& fSlopeAccel();

	float& fSlopeMaxSpeed();

	void SetApearScale(float x, float y, float z);

private:
	void Init();

public:
	void Term() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	void AdjustMaxSpeed() override;

private:
	void SetRingDamageFrame(int nRing);

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

	int AjustAccelRotate(float minrot);

	int AjustAccelRotate();

protected:
	void ReleaseTerm() override { cTaskBoss04::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
