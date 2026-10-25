#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickPillar : public cTaskBgGimmick
{
private:
	static constexpr float PILLAR_SIZE_Y = 1.0f;

	static constexpr float PILLAR_SIZE_Z = 1.0f;

	uint m_nEff{};

	Vector4 m_vEffectTransL[2]{};

	float m_fWidth{};

	bool m_playEffect{};

public:
	cTaskBgGimmickPillar(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	bool HitCheck();
};

}
