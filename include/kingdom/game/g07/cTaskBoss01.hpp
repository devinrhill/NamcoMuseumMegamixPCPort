#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskBoss01 : public cTaskCharBoss
{
private:
	enum EAppearStep
	{
		APPEAR_STEP_FADEIN = 0,
		APPEAR_STEP_EFFECT = 1,
		APPEAR_STEP_APPEAR_LEG = 2,
		APPEAR_STEP_FOG_CLEAR = 3,
		APPEAR_STEP_START_WAIT = 4
	};

	enum EMove
	{
		MOVE_CT = 0,
		MOVE_LT = 1,
		MOVE_LD = 2,
		MOVE_RD = 3,
		MOVE_RU = 4,
		MOVE_LU = 5,
		MOVE_MAX = 6
	};

	enum EMOActionB1
	{
		MO_ACTION_B1_MOVE01 = 0,
		MO_ACTION_B1_CHARGE01 = 1,
		MO_ACTION_B1_RUSH01 = 2,
		MO_ACTION_B1_ROTATE01 = 3,
		MO_ACTION_B1_SPAWN01 = 4,
		MO_ACTION_B1_ROTATE02 = 5,
		MO_ACTION_B1_MOVE02 = 6,
		MO_ACTION_B1_ROTATE03 = 7,
		MO_ACTION_B1_SPAWN02 = 8,
		MO_ACTION_B1_ROTATE04 = 9,
		MO_ACTION_B1_CHARGE02 = 10,
		MO_ACTION_B1_RUSH02 = 11,
		MO_ACTION_B1_ROTATE05 = 12,
		MO_ACTION_B1_SPAWN03 = 13,
		MO_ACTION_B1_ROTATE06 = 14,
		MO_ACTION_B1_MOVE03 = 15,
		MO_ACTION_B1_ROTATE07 = 16,
		MO_ACTION_B1_SPAWN04 = 17,
		MO_ACTION_B1_ROTMOVE01 = 18,
		MO_ACTION_B1_MAX = 19
	};

	enum EMotosBS1
	{
		MOTOS_BS1_CENTER_MDL = 0,
		MOTOS_BS1_MDL = 1,
		MOTOS_BS1_ANM = 2,
		MOTOS_BS1_CENTER_ANM = 3
	};

	enum EMOModeB1
	{
		MO_MODE_B1_APPEAR = 1,
		MO_MODE_B1_GOCENTER = 2,
		MO_MODE_B1_MOVE = 3,
		MO_MODE_B1_CHARGE = 4,
		MO_MODE_B1_RUSH = 5,
		MO_MODE_B1_ROTATE = 6,
		MO_MODE_B1_ROTCENTER = 7,
		MO_MODE_B1_SPAWN = 8,
		MO_MODE_B1_WAIT = 9,
		MO_MODE_B1_DEATH = 10
	};

	enum ECamera
	{
		eCAMERA_DOWN = 0,
		eCAMERA_CENTER = 1
	};

public:
	enum EModel
	{
		MODEL_CENT1 = 0,
		MODEL_CENT2 = 1,
		MODEL_BODY = 2,
		MODEL_GLAS = 3,
		MODEL_GHORST = 4,
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

	static constexpr float APPEAR_FAR_FOG = 40.0f;

	static float APPEAR_ADD_NEAR_FOG;

	static float APPEAR_ADD_FAR_FOG;

	static constexpr int DAMAGE_BASE_FOG_NEAR = 30;

	static constexpr int DAMAGE_BASE_FOG_FAR = 60;

	static constexpr float DAMAGE_FOG_NEAR = -25.0f;

	static constexpr float DAMAGE_FOG_FAR = 25.0f;

	static constexpr int EXPLODE_FRAME_FOG = 90;

	static constexpr float EXPLODE_ADD_NEAR_FOG = -1.0f / 18.0f;

	static constexpr float EXPLODE_ADD_FAR_FOG = 1.0f / 6.0f;

	static constexpr int STEP_WAIT = 255;

private:
	static Vector4 s_vPos[6];

	static int g_HitDamage[6];

public:
	static const Constants::MO_BOSS_MODE_INFO* s_ModeTbl[20];

private:
	Matrix4x4 m_mRoll{};

	float m_fTargetRot{};

	float m_fNowAccel{};

	float m_fNowMaxSpeed{};

	float m_fRushAccel{};

	float m_fRushSpeed{};

	float m_fRotSpeed{};

	float m_fRotMaxSpeed{};

	float m_fRotAccel{};

	sbyte m_nHitFlash{};

	bool m_nAttacked{};

	uint m_nCameraMode{};

	float m_fCenterPosY{};

public:
	static int DAMAGE_BASE_HP(cTaskCharBoss* boss);

	static float DAMAGE_ADD_NEAR_FOG(cTaskCharBoss* boss);

	static float DAMAGE_ADD_FAR_FOG(cTaskCharBoss* boss);

	static void SET_MODE_WAIT(cTaskCharBoss* boss, int mode, int time);

	cTaskBoss01();

	float& fTargetRot();

	float& fRushAccel();

	float& fRushSpeed();

	float& fRotSpeed();

	float& fNowAccel();

	float& fNowMaxSpeed();

	Matrix4x4& mRoll();

	float& fCenterPosY();

private:
	void Init();

public:
	void Term() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	bool ActionOther() override;

	void AdjustMaxSpeed() override;

private:
	void DrawStartDirection();

	void DrawInGame();

	void DrawEndDirection();

public:
	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

	void RefAttacked(int nParts, int nGrobalID) override;

	static void SET_STEP_NO_CLEAR(cTaskCharBoss* boss, ushort step);

	static void SET_STEP_WAIT_NO_CLEAR(cTaskCharBoss* boss, ushort step, int time);

	static void SET_STEP(cTaskCharBoss* boss, ushort step);

	static void SET_STEP_WAIT(cTaskCharBoss* boss, ushort step, int time);

	void ApearCameraSet();

private:
	void ModeAppear();

	void ModeDeath();
};

}
