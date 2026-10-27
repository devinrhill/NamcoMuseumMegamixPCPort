#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickTruck : public cTaskBgGimmick
{
private:
	uint m_nStep{};

	GameObject* m_SndHnd{};

	Transform* m_moveRoot{};

public:
	cTaskBgGimmickTruck(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskBgGimmickTruck::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
