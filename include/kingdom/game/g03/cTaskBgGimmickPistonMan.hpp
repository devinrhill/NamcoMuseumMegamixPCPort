#pragma once

#include "common.hpp"
#include "kingdom/cKingdomSndActor.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickPistonMan : public cTaskKingdom
{
private:
	class AreaGimmick
	{
	public:
		cTaskBgGimmick* pTaskBgGimmick{};

		float length{};
	};

	std::vector<cTaskBgGimmickPistonMan::AreaGimmick*> m_vecGimmick;

	int m_nSyncSndNo{};

	kingdom::cKingdomSndActor* m_pSndActor{};   // DOL +0x1c0

	GameObject* m_SndHnd{};                     // DOL +0x1b4 (loop sound handle)

public:
	cTaskBgGimmickPistonMan();

private:
	void Init();

public:
	void Term();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nState, cJetTask* pParam) override;

	void RegistGimmick(cTaskBgGimmick* gim);

protected:
	void ReleaseTerm() override { cTaskBgGimmickPistonMan::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
