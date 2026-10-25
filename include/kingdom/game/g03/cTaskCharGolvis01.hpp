#pragma once

#include "common.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskCharGolvis.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskCharGolvis01 : public cTaskCharGolvis
{
public:
	enum gmEMonsterStateGolvisRail
	{
		STATE_GOLVISRAIL_PURSUIT = 0,
		STATE_GOLVISRAIL_ATTACK = 1,
		STATE_GOLVISRAIL_MAX = 2
	};

protected:
	class LINE_POINT
	{
	public:
		Vector4 m_vPos{};

		float m_fDist{};

		cTaskCharGolvis01::LINE_POINT* Clone();
	};

	cTaskCharGolvis01::gmEMonsterStateGolvisRail m_iState{};

	Vector4 m_vRailCenter{};

	int m_iForwardDir{};

	int m_iDirRefreshTime{};

	int m_iMoveStopTime{};

	std::vector<cTaskCharGolvis01::LINE_POINT*> m_vecLinePoint;

	float m_fLinePosition{};

private:
	static constexpr float MOVE_SPEED_DELTA = 0.01f;

public:
	cTaskCharGolvis01(cJetTask* pParent);

private:
	void Init();

public:
	void Term() override;

protected:
	virtual bool IsNearAttack();

	bool IsForceAttack() override;

	bool ChangeToAttack(int iAttackKind) override;

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void Setting(bool bRelocate) override;

	void InitStateFirst() override;

	void InitStateStart() override;

	void ChangeToRevive() override;

protected:
	void SelectForwardDir();

	void MonsterFrameMove_Free() override;

	void MonsterFrameMove_Escape() override;

	virtual float LineGetPosRot(float& pOut, Vector4& vOut, float fDistance);

	virtual float LineCalcPosition(const Vector4& vTarget);

	virtual uint LineSearchNearPOINT(const Vector4& vTarget);

	virtual void LineMove(const int& dir, const float& fSpeed);

public:
	Constants::gmSGolvisAttack* GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum) override;
};

}
