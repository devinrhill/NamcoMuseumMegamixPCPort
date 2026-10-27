#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cGondolaEffect.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickGondola : public cTaskBgGimmick
{
private:
	static constexpr int EFF_MAX = 3;

	bool m_bBoot{};

	uint m_nSleepTime{};

	float m_fAnmFrameOld{};

	uint m_nEff{};

	cGondolaEffect* m_PtkHnd[3]{};

public:
	cTaskBgGimmickGondola(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW) override;

protected:
	void ReleaseTerm() override { cTaskBgGimmickGondola::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
