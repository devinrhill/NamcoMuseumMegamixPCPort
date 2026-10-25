#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskBoss05.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBoss05 : public cTaskBoss05
{
private:
	static Vector4 vRING_POSITION;

	static const int datalist[11];

	static const string node_name[4];

	Transform* m_glassNode{};

	Transform* m_ghostNode[(int)std::size(node_name)]{};

public:
	static constexpr float BOSS_05_DEFAULT_HEIGHT = 0.0f;

private:
	static constexpr int FLAG_TRANS_X = 1;

	static constexpr int FLAG_TRANS_Y = 2;

	static constexpr int FLAG_TRANS_Z = 4;

	static constexpr int FLAG_TRANS_ALL = 8;

	static constexpr int FLAG_ROTATE_X = 16;

	static constexpr int FLAG_ROTATE_Y = 32;

	static constexpr int FLAG_ROTATE_Z = 64;

	static constexpr int FLAG_ROTATE_ALL = 128;

	static constexpr int FLAG_TRANS_RING = 256;

public:
	cTaskCharEnemyBoss05();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	void UpdateMatrix() override;

	void ApearUpdateMatrix();

	void GameMainUpdateMatrix();
};

}
