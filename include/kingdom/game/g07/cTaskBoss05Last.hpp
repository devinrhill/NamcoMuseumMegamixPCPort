#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskBoss05Last : public cTaskCharBoss
{
private:
	enum EDirectionB5LStep
	{
		DIRECTION_B5L_STEP_FADEIN = 0,
		DIRECTION_B5L_STEP_APPEAR = 1,
		DIRECTION_B5L_STEP_EXPAND_INIT = 2,
		DIRECTION_B5L_STEP_EXPAND_ROTE_INIT = 3,
		DIRECTION_B5L_STEP_EXPAND_WAIT = 4,
		DIRECTION_B5L_STEP_EXPAND = 5,
		DIRECTION_B5L_STEP_EXPAND_END = 6,
		DIRECTION_B5L_STEP_FOG_CLEAR = 7,
		DIRECTION_B5L_STEP_TOP_APPEAR = 8,
		DIRECTION_B5L_STEP_START_WAIT = 9,
		DIRECTION_B5L_STEP_WAIT = 255
	};

	enum EMove
	{
		MOVE_CT = 0
	};

	enum EMOActionB5L
	{
		MO_ACTION_B5L_RANDOM01 = 0,
		MO_ACTION_B5L_HOMING01 = 1,
		MO_ACTION_B5L_MOVE01 = 2,
		MO_ACTION_B5L_STOP02 = 3,
		MO_ACTION_B5L_MAX = 4
	};

	enum EMotosBS5L
	{
		MOTOS_BS5L_MDL_CENTER = 0,
		MOTOS_BS5L_MDL_LIGHT01 = 1,
		MOTOS_BS5L_MDL_LIGHT02 = 2,
		MOTOS_BS5L_ANM_CENTER = 3,
		MOTOS_BS5L_ANM_LIGHT01 = 4,
		MOTOS_BS5L_ANM_LIGHT02 = 5
	};

public:
	enum EMOModeB5L
	{
		MO_MODE_B5L_APPEAR = 1,
		MO_MODE_B5L_GOCENTER = 2,
		MO_MODE_B5L_HOMING = 3,
		MO_MODE_B5L_RANDOM = 4,
		MO_MODE_B5L_MOVE = 5,
		MO_MODE_B5L_STOP = 6,
		MO_MODE_B5L_DEATH = 7,
		MO_MODE_B5L_WAIT = 8,
		MO_MODE_B5L_END = 9
	};

	enum EModel
	{
		MODEL_GHOST1 = 0,
		MODEL_GHOST2 = 1,
		MODEL_GHOST3 = 2,
		MODEL_GHOST4 = 3,
		MODEL_GLAS = 4,
		MODEL_LAST = 5,
		MODEL_MAX = 6
	};

private:
	static constexpr int APPEAR_ZOOM_FRAME = 40;

	static constexpr float APPEAR_ZOOM_ADD = 0.0075000003f;

	static constexpr float WHITEOUT_NEAR_FOG = -30.0f;

	static constexpr float WHITEOUT_FAR_FOG = 30.0f;

	static constexpr int APPEAR_FRAME_FOG = 120;

	static constexpr float APPEAR_ADD_NEAR_FOG = 0.25f;

	static constexpr float APPEAR_ADD_FAR_FOG = -0.25f;

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

	static constexpr int CENTER_RESTOR_UP_FRAME = 40;

	static constexpr float CENTER_PIYO_PUL2 = 0.15f;

	static constexpr float RING_DEC_SPEED = 0.05f;

	static constexpr float PILLAR_HEIGHT = 2.95f;

	static constexpr float RING_HEIGHT = 2.1f;

	static constexpr int SPAWN_TIME = 120;

	static constexpr int MOTEOR_NUM = 5;

	static constexpr int MORE_SPAWN_BOM_NUM = 5;

	static Vector4 s_vPos[1];

	static int g_HitDamage[6];

	static Constants::MO_BOSS_MODE_INFO* s_MdeTbl[4];

	Vector4 m_vSTarget{};

	Vector4 m_vDeathPoint{};

	float m_fNowAccel{};

	float m_fNowMaxSpeed{};

	float m_fRushAccel{};

	float m_fRushSpeed{};

	int m_nFrameCnt{};

	sbyte m_nHitFlash{};

	bool m_IsTakeOver{};

	int m_nAction{};

	int m_nActionNum{};

	float m_fSlopeMaxRad{};

	float m_fSlopeAccel{};

	float m_fTopAlpha{};

	float m_fScaleAcsel{};

	float m_fGoScale{};

	float m_fUpScale{};

	float m_fRotSpeed[2]{};

	int m_nHomingFrame{};

	int m_nRandomFrame{};

	int m_nStopFrame{};

	Vector4 m_vRandTarget{};

	static constexpr float SLOPE_AC_MAX = 0.0004f;

	static constexpr float SLOPE_FLOOR_AC = 0.0001f;

	static constexpr float SLOPE_REGIST_AC = 2E-05f;

	static float DAMAGE_BASE_HP(cTaskCharBoss* boss);

	static float DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss);

	static float DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss);

	static void SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time);

	static void SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step);

	static void SET_STEP(cTaskCharBoss* boss, ushort step);

public:
	cTaskBoss05Last();

	Vector4& vRandTarget();

	float& fRushAccel();

	float& fRushSpeed();

	float& fRotSpeed();

	float& fNowAccel();

	float& fNowMaxSpeed();

	bool& IsTakeOver();

private:
	void Init();

public:
	void Term() override;

	bool FallMove() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	void AdjustMaxSpeed() override;

	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

	void RefAttacked(int nParts, int nGrobalID) override;

private:
	void SlantFloor();

public:
	uint Input() override;

private:
	Vector4 vSetRandTarget();

	int NextMode();

	void SetNextMode();
};

}
