#pragma once

#include "common.hpp"
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

	bool m_isFirstSnd{};

public:
	cTaskBgGimmickPistonMan();

private:
	void Init();

public:
	void Term();

	bool Execute(cJetTask* pParam) override;

	void Draw(int nState, cJetTask* pParam) override;

	void RegistGimmick(cTaskBgGimmick* gim);
};

}
