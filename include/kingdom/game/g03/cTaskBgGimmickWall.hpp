#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickWall : public cTaskBgGimmick
{
private:
	Vector4 m_vAreaSize{};

	float m_fAddRot{};

	uint m_nEff{};

	Vector4 m_vEffectTransL[2]{};

	uint m_nCnt{};

public:
	cTaskBgGimmickWall(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void HitCallbackWall(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW) override;

	bool AreaCheck();
};

}
