#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsPacIcon : public cTaskConsoleBase
{
public:
	enum EPacIcon
	{
		ePAC_ICON1 = 0,
		ePAC_ICON2 = 1,
		ePAC_ICON3 = 2,
		ePAC_ICON4 = 3,
		ePAC_ICON5 = 4,
		ePAC_ICON_MAX = 5
	};

private:
	uint m_nPlayerID{};

	Ref<int>* m_pExecNumber{};

	float m_aConsoleX[5]{};

	float m_aConsoleY[5]{};

public:
	static float get_PAC_ICON_POS_RIVISE();

	cTaskConsPacIcon();

	virtual ~cTaskConsPacIcon();

	void Free() override;

	float& fConsoleX(uint set);

	static cTaskAnita* Construct();

	virtual void SetpExecNumber(Ref<int>* set);

	uint& nPlayerID();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Visible(uint set = std::numeric_limits<uint>::max()) override;

	void Invisible(uint set = std::numeric_limits<uint>::max()) override;
};

}
