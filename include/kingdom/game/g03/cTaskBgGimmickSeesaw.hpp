#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickSeesaw : public cTaskBgGimmick
{
private:
	bool m_bLevel{};

	Vector4 m_vFulcrumL{};

	Vector4 m_vApplication{};

	float m_fRotInit{};

	float m_fRotMax{};

	float m_fRot{};

	float m_fRotAcc{};

	float m_fRotAccMax{};

	float m_fLength{};

	float m_fMoment{};

	float m_fNoRiddenRot{};

public:
	cTaskBgGimmickSeesaw(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	bool CheckBottom();

protected:
	void ReleaseTerm() override { cTaskBgGimmickSeesaw::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
