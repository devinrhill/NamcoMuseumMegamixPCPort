#pragma once

#include "common.hpp"
#include "kingdom/game/g03/cTaskBgHit.hpp"
#include "kingdom/game/g03/sSaveDataGate.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskPBModel.hpp"
#include "kingdom/game/g03/cTaskGateCookie.hpp"

namespace kingdom::game::g03
{

class cTaskBgGate : public cTaskBgHit
{
private:
	enum EDraw
	{
		eDRAW_NORMAL = 0,
		eDRAW_IJIKE = 1,
		eDRAW_NONE = 2
	};

	enum EStep
	{
		eSTEP_INIT = 0,
		eSTEP_WORK = 1,
		eSTEP_BREAK = 2,
		eSTEP_SLEEP = 3
	};

public:
	enum EKind
	{
		eKIND_NORMAL = 0,
		eKIND_SHORT = 1,
		eKIND_LONG = 2,
		eKIND_MAX = 3
	};

private:
	static constexpr float AREA_LENGTH_X = 5.0f;

	static constexpr float AREA_LENGTH_Y = 4.0f;

	static constexpr float AREA_LENGTH_Z = 5.0f;

	uint m_nKind{};

	bool m_bWake{};

	uint m_nCnt{};

	uint m_nPriority{};

	uint m_nNeedCookie{};

	cTaskGateCookie* m_pTaskGateCookie{};

	cTaskPBModel* m_pBreakModel{};

	cKingdomPtkHnd* m_PtkHnd[2]{};

	uint m_nDrawFlag{};

	bool m_bFirstBreak{};

	float m_fCheckRange{};

	bool m_bDrain{};

public:
	uint nKind();

	bool& IsWake();

	bool IsBreak();

	uint& nNeedCookie();

	uint GetPriority();

	bool IsDrain();

	cTaskBgGate();

private:
	void Init();

public:
	void Term() override;

	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool RegistData(const uint& kind, const uint& cookieNum, const uint& pri, const Vector4& trans, const Vector4& rotate);

	bool RegistData(sSaveDataGate& gate);

private:
	bool AreaCheck();
};

}
