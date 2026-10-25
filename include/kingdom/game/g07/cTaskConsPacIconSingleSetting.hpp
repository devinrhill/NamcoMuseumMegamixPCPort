#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsPacIconSingleSetting : public cTaskConsoleBase
{
private:
	enum EPacIcon
	{
		ePAC_ICON1_1 = 0,
		ePAC_ICON1_2 = 1,
		ePAC_ICON1_3 = 2,
		ePAC_ICON1_4 = 3,
		ePAC_ICON1_5 = 4,
		ePAC_ICON2_1 = 5,
		ePAC_ICON2_2 = 6,
		ePAC_ICON2_3 = 7,
		ePAC_ICON2_4 = 8,
		ePAC_ICON2_5 = 9,
		ePAC_ICON3_1 = 10,
		ePAC_ICON3_2 = 11,
		ePAC_ICON3_3 = 12,
		ePAC_ICON3_4 = 13,
		ePAC_ICON3_5 = 14,
		ePAC_ICON_MAX = 15,
		ePAC_ICON_WIDTH_NUM = ePAC_ICON2_1,
		ePAC_ICON_HEIGHT_NUM = ePAC_ICON1_4
	};

	uint m_nPlayerID{};

public:
	static float get_PAC_ICON_POS_X_RIVISE();

	static float get_PAC_ICON_POS_Y_RIVISE();

	static float get_PAC_ICON_POS_X_FIRST();

	static float get_PAC_ICON_POS_Y_FIRST();

	cTaskConsPacIconSingleSetting();

	virtual ~cTaskConsPacIconSingleSetting();

	void Free() override;

	static cTaskAnita* Construct();

	uint& nPlayerID();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
