#pragma once

#include "common.hpp"
#include "kingdom/cKingdomSndActor.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include <ESEKind.hpp>
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickAddinSnd : public cTaskBgGimmick
{
private:
	float m_fLength{};

	ESEKind m_pSndName{};

	GameObject* m_SndHnd{};

	kingdom::cKingdomSndActor* m_pSndActor{};   // DOL +0x520

public:
	cTaskBgGimmickAddinSnd(ResNode* const& node0, ResNode* const& node1, const ESEKind& sndName);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nState, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskBgGimmickAddinSnd::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
