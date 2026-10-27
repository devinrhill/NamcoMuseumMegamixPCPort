#pragma once

#include "common.hpp"
#include "kingdom/cTaskPushBack.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkMan.hpp"

namespace kingdom
{

class cTaskPBPtkMan : public cTaskPushBack
{
private:
	cKingdomPtkMan* m_particleManager{};

	bool m_bDebugDraw{};

	GameObject* m_gazePoint{};

	Camera* m_camera{};

public:
	cKingdomPtkMan* get_KingdomPtkMan();

	cTaskPBPtkMan();

	virtual ~cTaskPBPtkMan();

	void Term();

	bool Execute(cJetTask* pParam) override;

	void PostDraw(int nStatus, cJetTask* pParam) override;

	void DrawScnRootXlu() override;

	bool IsDebugDraw();

protected:
	void ReleaseTerm() override { cTaskPBPtkMan::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
