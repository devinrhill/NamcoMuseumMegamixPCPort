#pragma once

#include "common.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskGimmickSwitch : public cTaskGimmickBase
{
private:
	Constants::PANEL_INFO* m_apPanels[256]{};

public:
	cTaskGimmickSwitch();

	static cTaskGimmickBase* Construct();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	bool Execute(cJetTask* pParam) override;

	void BootSwitch();

	void PanelExplode();

	bool ToSwitchLength(Constants::PANEL_INFO* p1, Constants::PANEL_INFO* p2);

protected:
	void ReleaseTerm() override { cTaskGimmickSwitch::Term(); cTaskModel::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
