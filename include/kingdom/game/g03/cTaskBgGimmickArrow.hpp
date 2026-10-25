#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskBgGimmickArrow : public cTaskBgGimmick
{
private:
	static constexpr float ARROW_HIT_RADIUS = 0.6f;

	static constexpr float SOUND_ENABLE_RADIUS = 15.0f;

	uint m_nIntervalTime{};

	uint m_nIntervalCnt{};

	float m_fSpeed{};

	Vector4 m_vDir{};

	Vector4 m_vDefaultTrans{};

	bool m_bBreak{};

	uint m_nBreakTime{};

	float m_fBreakRot{};

	uint m_nLifeTime{};

	float m_fSndLength{};

	bool m_bHit{};

public:
	cTaskBgGimmickArrow(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool HitCheck();
};

}
