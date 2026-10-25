#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskBoss03 : public cTaskCharBoss
{
private:
	enum EDirectionStep
	{
		DIRECTION_STEP_FADEIN = 0,
		DERECTION_STEP_EFFECT = 1,
		DIRECTION_STEP_APPEAR_LEG = 2,
		DIRECTION_STEP_FOG_CLEAR = 3,
		DIRECTION_STEP_START_WAIT = 4,
		DIRECTION_STEP_WAIT = 5
	};

	enum EMove
	{
		MOVE_CT = 0,
		MOVE_LU = 1,
		MOVE_RRU = 2,
		MOVE_RRD = 3,
		MOVE_LLD = 4,
		MOVE_LLU = 5,
		MOVE_LUU = 6,
		MOVE_RDD = 7,
		MOVE_MAX = 8
	};

	enum EMOActionB3
	{
		MO_ACTION_B3_MOVE01 = 0,
		MO_ACTION_B3_CHARGE01 = 1,
		MO_ACTION_B3_RUSH01 = 2,
		MO_ACTION_B3_ROTATE01 = 3,
		MO_ACTION_B3_SPAWN01 = 4,
		MO_ACTION_B3_MOVE02 = 5,
		MO_ACTION_B3_ROTATE02 = 6,
		MO_ACTION_B3_SPAWN02 = 7,
		MO_ACTION_B3_CHARGE02 = 8,
		MO_ACTION_B3_MOVE03 = 9,
		MO_ACTION_B3_ROTATE03 = 10,
		MO_ACTION_B3_SPAWN03 = 11,
		MO_ACTION_B3_MOVE04 = 12,
		MO_ACTION_B3_ROTATE04 = 13,
		MO_ACTION_B3_SPAWN04 = 14,
		MO_ACTION_B3_MOVE05 = 15,
		MO_ACTION_B3_CHARGE03 = 16,
		MO_ACTION_B3_MOVE06 = 17,
		MO_ACTION_B3_ROTMOVE01 = 18,
		MO_ACTION_B3_ROTACCEL = 19,
		MO_ACTION_B3_MAX = 20
	};

	enum EMotosBS3
	{
		MOTOS_BS3_MDL = 0,
		MOTOS_BS3_CENTER_MDL = 1,
		MOTOS_BS3_ANM = 2,
		MOTOS_BS3_CENTER_ANM = 3
	};

	enum EMOModeB3
	{
		MO_MODE_B3_APPEAR = 1,
		MO_MODE_B3_GOCENTER = 2,
		MO_MODE_B3_MOVE = 3,
		MO_MODE_B3_CHARGE = 4,
		MO_MODE_B3_RUSH = 5,
		MO_MODE_B3_ROTATE = 6,
		MO_MODE_B3_ROTMOVE = 7,
		MO_MODE_B3_SPAWN = 8,
		MO_MODE_B3_ROTACCEL = 9,
		MO_MODE_B3_WAIT = 10,
		MO_MODE_B3_DEATH = 11
	};

public:
	enum EModel
	{
		MODEL_CENT1 = 0,
		MODEL_CENT2 = 1,
		MODEL_BODY = 2,
		MODEL_GLAS = 3,
		MODEL_GHOST = 4,
		MODEL_MAX = 5
	};

private:
	enum EEndStep
	{
		END_STEP_LEG_2WHITE = 0,
		END_STEP_LEG_EXPLOD = 1,
		END_STEP_CORE_2WHITE = 2,
		END_STEP_CORE_EXPLOD = 3,
		END_STEP_TAIKI = 4
	};

public:
	static constexpr int APPEAR_FRAME_XZ = 8;

	static constexpr int APPEAR_FRAME_Y = 5;

	static constexpr float APPEAR_ADD_XZ = 0.125f;

	static constexpr float APPEAR_ADD_Y = 0.2f;

	static constexpr int APPEAR_FRAME_FOG = 60;

	static constexpr float APPEAR_NEAR_FOG = -30.0f;

	static constexpr float APPEAR_FAR_FOG = 30.0f;

	static constexpr int DAMAGE_BASE_FOG_NEAR = 30;

	static constexpr int DAMAGE_BASE_FOG_FAR = 60;

	static constexpr float DAMAGE_FOG_NEAR = -25.0f;

	static constexpr float DAMAGE_FOG_FAR = 25.0f;

	static constexpr int EXPLODE_FRAME_FOG = 90;

	static constexpr float EXPLODE_ADD_NEAR_FOG = -1.0f / 18.0f;

	static constexpr float EXPLODE_ADD_FAR_FOG = 1.0f / 18.0f;

	static constexpr float ENSYU = (float)std::numbers::pi * 2.0f;

private:
	static Vector4 s_vPos[8];

	static int g_HitDamage[6];

	static const Constants::MO_BOSS_MODE_INFO* s_ModeTbl[20];

	Matrix4x4 m_mRoll{};

	float m_fTargetRot{};

	float m_fNowAccel{};

	float m_fNowMaxSpeed{};

	float m_fRushAccel{};

	float m_fRushSpeed{};

	float m_fRotSpeed{};

	float m_fRotMaxSpeed{};

	float m_fRotSpeedAccel{};

	sbyte m_nHitFlash{};

	float m_fRotAccel{};

	bool m_nAttacked{};

	Vector4 m_vApearScale{};

	float m_fCenterPosY{};

	float m_fAccelRotSpeed{};

	float m_fMaxAccelRotSpeed{};

	float m_fMinAccelRotSpeed{};

	int m_nAccelRotTime{};

	int m_nSlowDownRotTime{};

	int m_nStopRotTime{};

public:
	static float get_APPEAR_ADD_NEAR_FOG();

	static float get_APPEAR_ADD_FAR_FOG();

	static float DAMAGE_BASE_HP(cTaskCharBoss* boss);

	static float DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss);

	static float DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss);

	static void SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time);

	cTaskBoss03();

	float& fTargetRot();

	float& fRushAccel();

	float& fRushSpeed();

	float& fRotSpeed();

	float& fNowAccel();

	float& fNowMaxSpeed();

	Matrix4x4& mRoll();

	float& fCenterPosY();

	void SetApearScale(float x, float y, float z);

private:
	void Init();

public:
	void Term() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	void AdjustMaxSpeed() override;

	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

	void RefAttacked(int nParts, int nGrobalID) override;

	static void SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step);

	static void SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time);

	static void SET_STEP(cTaskCharBoss* boss, ushort step);

	static void SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time);

private:
	void ModeAppear();

	void ModeDeath();
};

}
