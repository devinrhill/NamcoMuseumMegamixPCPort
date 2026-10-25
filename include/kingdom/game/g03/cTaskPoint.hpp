#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/sCheckPoint.hpp"

namespace kingdom::game::g03
{

class cTaskPoint : public cTaskModel
{
private:
	enum EState
	{
		eSTATE_IJIKE = 0,
		eSTATE_PASS = 1
	};

	static constexpr float POINT_RADIUS = 2.25f;

	int m_nRetryIdx{};

	uint m_nNum{};

	std::vector<sCheckPoint*> m_pCheckPoint;

	bool m_bFont{};

public:
	int& nRetryIdx();

	Vector4& vRetryPos();

	float vRetryPos(uint n);

	Vector4& vRetryRot();

	float vRetryRot(uint n);

	bool IsFontDraw();

	cTaskPoint();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

private:
	bool HitCheck(const Vector4& trans);

public:
	void SetRetryData();
};

}
