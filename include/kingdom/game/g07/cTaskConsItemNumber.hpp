#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskConsItemNumber : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_X = 0,
		eANM_NUMBER1 = 1,
		eANM_NUMBER2 = 2,
		eANM_NUMBER3 = 3,
		eANM_NUMBER4 = 4,
		eANM_NUMBER5 = 5,
		eANM_NUMBER6 = 6,
		eANM_NUMBER7 = 7,
		eANM_TEMP = 8,
		eANM_MAX = 9
	};

	uint m_nNumber[9]{};

	bool m_bChange[9]{};

	float m_fOffsetX{};

	int m_nDrawFigure{};

	Ref<int>* m_pExecNumber{};

	bool m_bExec{};

	uint m_nFlag{};

	float m_fFrame{};

public:
	static float X_OFFSET;

private:
	int AnmList_d[10] = {0, 6, 12, 18, 24, 30, 36, 42, 48, 54};

	int AnmList_t[10] = {1, 7, 13, 19, 25, 31, 37, 43, 49, 55};

public:
	cTaskConsItemNumber(int nFigure);

	virtual ~cTaskConsItemNumber();

	void Free() override;

	bool& bChange(uint set);

	void pExecNumber(Ref<int>* set);

	bool& bExec();

	float& fOffsetX();

	void SetFlag(uint flag, bool sw);

	uint GetFlag();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
