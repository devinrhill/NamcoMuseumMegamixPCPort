#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cHitChar.hpp"

namespace kingdom::game::g03
{

class cTaskCharAreaGhost : public cTaskCharGhost
{
public:
	enum EType
	{
		TYPE_NORMAL = 0,
		TYPE_IZIKE = 1,
		TYPE_MAX = 2
	};

	enum EState
	{
		STATE_NORMAL = 0,
		STATE_IZIKE = 1,
		STATE_MAX = 2
	};

	enum EGhostST
	{
		GHOST_ST_NONE = 0,
		GHOST_ST_SEARCH = 1,
		GHOST_ST_MOVEING = 2,
		GHOST_ST_SEARCH_RELAY = 3,
		GHOST_ST_CHASE = 4,
		GHOST_ST_ATTACK = 5,
		GHOST_ST_IZIKE_PRE = 6,
		GHOST_ST_IZIKE = 7,
		GHOST_ST_EATED = 8,
		GHOST_ST_DEAD = 9,
		GHOST_ST_CREATE = 10,
		GHOST_ST_END = 11
	};

	enum EMoveST
	{
		MOVE_ST_NONE = 0,
		MOVE_ST_RANDOM = 1,
		MOVE_ST_RELAY = 2,
		MOVE_ST_CHASE = 3,
		MOVE_ST_ESCAPE = 4
	};

private:
	int m_nCount{};

	float m_fEatSpeed{};

	bool m_bHydeItem{};

	int m_nMoveStatus{};

	int m_nMoveCount{};

	float m_fSpeed{};

	float m_fTargetSpeed{};

	int m_nTargetPointID{};

	Vector4 m_vTarget{};

	Vector4 m_vMoveTarget{};

	Vector4 m_vPacManTrans{};

	bool m_bCanFlying{};

	uint m_nFlyingProc{};

	bool m_bFlying{};

	bool m_bFlyingUp{};

	Vector4 m_vFlyingTarget{};

	float m_fMaxSpeed{};

	float m_fAddSpeed{};

	float m_fTurnSpeed{};

	float m_fIzikeSpeed{};

	int m_nIzikeFlashTime{};

	bool m_bPackAttack{};

	float m_fFootDist{};

public:
	float& fFootDist();

	int& nTargetPointID();

	Vector4& vTarget();

	Vector4& vMoveTarget();

	int& nMoveStatus();

	int& nMoveCount();

	virtual cTaskChar::eKIND GetKind();

	virtual string GetModelKey();

	virtual string GetAnmKey();

	virtual string GetSRTAnmKey();

	virtual uint GetPushOutLevel();

	bool IsDead() override;

	cTaskCharAreaGhost();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	void SetType(const uint& type);

	void SetAnmState(const uint& state);

	void SetAnm(string anime, uint idx = 0u);

	void Move();

	void SetModel() override;

	void SetParam();

	void SetOptionItem(int type);

	void SetHydeItem(bool on);

	void FastDead();

	void CheckStatus();

	void ChangeArea();

	void Attacked(cHitChar* pRev, cHitChar* pAtk) override;

	void HitPushOut(Vector4 vVec, cTaskChar* pChar) override;
};

}
