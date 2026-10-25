#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game
{

class cTaskConsMenuBase : public cTaskAnita
{
protected:
	enum EState
	{
		eSTATE_FADE_IN = 0,
		eSTATE_WAIT = 1,
		eSTATE_APPEAR = 2,
		eSTATE_APPEAR_END = 3,
		eSTATE_POINT = 4,
		eSTATE_DECIDE = 5,
		eSTATE_DECIDE_END = 6,
		eSTATE_DELETE = 7,
		eSTATE_DELETE_END = 8,
		eSTATE_MAX = 9
	};

private:
	cTaskConsMenuManBase* m_pTaskConsMenuMan{};

	int m_nId{};

	int m_nState{};

	int m_nStep{};

protected:
	cTaskConsMenuManBase*& pTaskConsMenuMan();

	int& nId();

	int& nState();

	int& nStep();

	// Common body of the IsDpdHit overrides (DOL g03 0x8005e344 Rule, 0x8005a7f0 Back, 0x8005a258 Arrow, 0x8005e8d0 Stage, 0x8005dda0 Map):
	// pointer (-1..1, y down) -> layout pixels (x * 320, -y * 240, truncated to int), then cKingdomAnita::IsHit on anita 0 and pattern `pat`.
	bool DpdHitPat(float x, float y, int pat);

public:
	int nGetId();

	virtual void WaitState();

	// DOL 0x801456e4 (vtable +0x60): anita 0 restarts its current animation (reset, frame 0)
	void RestartAnim();

	virtual bool IsWaitState();

	virtual void AppearState();

	virtual void AppearEndState();

	virtual bool IsAppearEndState();

	virtual void PointState();

	virtual void DecideState();

	virtual void DecideEndState();

	virtual bool IsDecideEndState();

	virtual void DeleteState();

	virtual void DeleteEndState();

	virtual bool IsDeleteEndState();

	cTaskConsMenuBase(cTaskConsMenuManBase* const& pConsMan);

	virtual ~cTaskConsMenuBase();

	virtual bool IsDpdHit(float x, float y);

	virtual int IsDpdHitId(float x, float y);

protected:
	virtual void WaitExec();

	virtual bool AppearExec();

	virtual void PointExec();

	virtual bool DecideExec();

	virtual bool DeleteExec();

public:
	void ResetAnm();

	bool Execute(cJetTask* pParam) override;
};

}
