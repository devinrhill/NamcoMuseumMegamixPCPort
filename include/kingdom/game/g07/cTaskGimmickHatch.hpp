#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskGimmickHatch : public cTaskGimmickBase
{
private:
	float m_fHatchComa{};

public:
	static constexpr float HATCH_SIZE_MAX = 1.0f;

	static constexpr float HATCH_SIZE_MIN = 0.3f;

	uint get_HATCH_CLOSE_TIME();

	uint get_HATCH_2OPEN_TIME();

	uint get_HATCH_OPEN_TIME();

	uint get_HATCH_2CLOSE_TIME();

	uint get_HATCHI_TIME_TOTAL();

	uint get_HATCH_CLOSE_MODE_START();

	uint get_HATCH_CLOSE_MODE_END();

	uint get_HATCH_2OPEN_MODE_START();

	uint get_HATCH_2OPEN_MODE_END();

	uint get_HATCH_OPEN_MODE_START();

	uint get_HATCH_OPEN_MODE_END();

	uint get_HATCH_2CLOSE_MODE_START();

	uint get_HATCH_2CLOSE_MODE_END();

	static cTaskGimmickBase* Construct();

	cTaskGimmickHatch();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no) override;

	bool Execute(cJetTask* pParam) override;

	bool Exec();

	bool IsStepOnCntW(Vector4 pWTrans) override;

	bool IsFall(Vector4 pMTrans) override;

protected:
	void ReleaseTerm() override { cTaskGimmickHatch::Term(); cTaskModel::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
