#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "kingdom/game/g07/NET_RF_PACKET.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaRed.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyNabicon : public cTaskCharEnemy
{
public:
	class NAVICON_STAGE_DATA
	{
	public:
		ushort m_nEnemyKind{};

		ushort m_nEnemyMax{};

		ushort m_nEnemyTotal{};
	};

	static cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* s_StageNormal[38];

	static cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* s_StageEasy[38];

	static cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* s_StageHard[38];

	static cTaskCharEnemyNabicon::NAVICON_STAGE_DATA* s_StageLAN[13];

	static constexpr int MAX_SPWAN_CHAR = 16;

	static constexpr int MO_NAVICON_SCORE = 50000;

private:
	bool m_bCrushFlag{};

	int m_nEnemyInter{};

	int m_nEnemyKind{};

	int m_nEnemyMax{};

	int m_nEnemyTotal{};

	int m_nHP{};

	int m_nAcceptDir{};

	int m_nEnemyTimer{};

	bool m_bRetry{};

	int m_nVibraTimer{};

	Vector4 m_vBaseTrans{};

	Vector4 m_vTransTemp{};

	static bool m_nInitFlag;

	static cTaskCharEnemyPupaRed* m_pSpawnChar[16];

public:
	cTaskCharEnemyNabicon();

	void InitParam(MO_CHAR_PARAMS* pParam) override;

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	uint Input() override;

private:
	cTaskChar* m_SpawnEnemy(int dir);

public:
	bool Status() override;

	bool ActionBoth() override;

	bool ActionOther() override;

	bool ActionLocal() override;

	void ExecReflect(NET_RF_PACKET* pPacket) override;

	void ExecReflectSub(Vector4 pVec, float hit_power, float charge_pow) override;

private:
	cTaskChar* m_GetTaikiEnemy();

public:
	void SetMMSParam(int para1, int para2, int para3) override;
};

}
