#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBomb : public cTaskCharEnemy
{
public:
	enum EMOModeBomb
	{
		MO_MODE_BOMB_TAIKI = 1,
		MO_MODE_BOMB_SHOOT = 2,
		MO_MODE_BOMB_MOVE1 = 3,
		MO_MODE_BOMB_MOVE2 = 4,
		MO_MODE_BOMB_EXPLODE = 5
	};

private:
	Matrix4x4 m_mRoll{};

	ushort m_nRotDir{};

	float m_fGensoku{};

	float m_fShootRange{};

	float m_fNoHitRange{};

	Vector4 m_vSpawnPos{};

public:
	cTaskCharEnemyBomb();

	static cTaskCharEnemy* Construct();

	float& fShootRange();

	float& fNoHitRange();

	Vector4& vSpawnPos();

	bool IsFall(float x, float y, float z) override;

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	uint InputLaunch();

	static bool RATE(int rand, int rate);

	uint Input() override;

	bool ActionBoth() override;

	bool ActionLocal() override;

	void ChangeTaiki() override;

	bool ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene) override;

	void RefAttacked(int nParts, int nGrobalID) override;

	bool IsFall(Nullable<Vector4> pVec = nullptr) override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyBomb::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
