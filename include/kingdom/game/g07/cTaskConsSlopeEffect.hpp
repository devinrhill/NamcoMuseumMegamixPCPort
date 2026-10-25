#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsSlopeEffect : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_SLOPE = 0,
		eANM_MAX = 1
	};

	enum ESlope
	{
		eSLOPE_LEFT = 0,
		eSLOPE_RIGHT = 1,
		eSLOPE_NONE = 2
	};

	uint m_nTimer{};

	uint m_nSoundTimer{};

	uint m_nSlopePat{};

	uint m_nSlopePatOld{};

	Vector4 m_vSlopeVecOld{};

	bool m_bGo{};

public:
	cTaskConsSlopeEffect();

	virtual ~cTaskConsSlopeEffect();

	void Free() override;

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
