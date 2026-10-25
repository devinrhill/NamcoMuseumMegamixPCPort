#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickFall : public cTaskBgGimmick
{
private:
	bool m_bBoot{};

	bool m_bReturn{};

	uint m_nTimeCnt{};

	float m_fMoveY{};

	uint m_nShakeTime{};

	uint m_nFallTime{};

	float m_fShakeFall{};

	float m_fFallAccel{};

	Vector4 m_vDefaultTrans{};

protected:
	bool& IsBoot();

	bool& IsReturn();

public:
	cTaskBgGimmickFall(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void HitCallbackFloor(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW) override;
};

}
