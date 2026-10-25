#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskChar.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g03/cTaskCharRouteGhost.hpp"

namespace kingdom::game::g03
{

class cTaskCharGhostBomb : public cTaskChar
{
public:
	enum EProc
	{
		PROC_WAIT = 0,
		PROC_SET = 1,
		PROC_LOCK_ON = 2,
		PROC_THROW = 3,
		PROC_BURST = 4,
		PROC_END = 5,
		PROC_MAX = 6
	};

private:
	uint m_nProc{};

	uint m_nSnaipeTime{};

	uint m_nCursorId{};

	uint m_nTimer{};

	uint m_nFlyTime{};

	float m_fRadiusMin{};

	float m_fRadiusMax{};

	float m_fRadius{};

	Vector4 m_vTarget{};

	Vector4 m_vCenter{};

	Vector4 m_vStart{};

	Vector4 m_vCursor{};

	cTaskCharRouteGhost* m_pGhostTask{};

	cKingdomPtkHnd* m_PtkHndBurst{};

	GameObject* m_SndHnd{};

	float m_burstWaitTime{};

public:
	bool IsWait();

	cTaskCharGhostBomb(uint cursor);

	void Term() override;

private:
	void Init();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	virtual void SetStart(cTaskCharRouteGhost* pParent);

	virtual void SetParam(uint nFlyTime, float fRadius, float max);

	virtual void Shoot();
};

}
