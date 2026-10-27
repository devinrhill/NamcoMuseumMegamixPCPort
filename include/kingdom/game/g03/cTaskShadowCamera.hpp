#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskShadowCamera : public cTaskKingdom
{
private:
	float m_fNear{};

	float m_fFar{};

	float m_fFacter{};

public:
	cTaskShadowCamera();

	virtual ~cTaskShadowCamera();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskShadowCamera::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
