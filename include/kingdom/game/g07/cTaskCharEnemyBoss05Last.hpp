#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskBoss05Last.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"

namespace kingdom::game::g07
{

class cTaskCharEnemyBoss05Last : public cTaskBoss05Last
{
private:
	static string node_name[4];

	Transform* m_glassNode{};

	Transform* m_ghostNode[(int)std::size(node_name)]{};

	static constexpr uint ModelNum = 6u;

	static constexpr uint FacterNum = 4u;

public:
	cTaskCharEnemyBoss05Last();

	static cTaskCharEnemy* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	void UpdateMatrix() override;

protected:
	void ReleaseTerm() override { cTaskCharEnemyBoss05Last::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
