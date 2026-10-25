#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::menu
{

class cParamMessage : public cKingdomCSV
{
public:
	enum ECsv
	{
		eCSV_ROW_HEAD = 0,
		eCSV_ROW_ERR_12 = 1,
		eCSV_ROW_MSG_13 = 2,
		eCSV_ROW_MSG_14 = 3,
		eCSV_ROW_MSG_18 = 4,
		eCSV_ROW_MSG_19 = 5,
		eCSV_ROW_MSG_20_1 = 6,
		eCSV_ROW_MSG_20_2 = 7,
		eCSV_ROW_MSG_20_3 = 8,
		eCSV_ROW_MSG_20_4 = 9,
		eCSV_ROW_MSG_21 = 10,
		eCSV_ROW_MSG_25 = 11,
		eCSV_ROW_MSG_26 = 12,
		eCSV_ROW_MSG_43 = 13,
		eCSV_ROW_MSG_ = 14,
		eCSV_ROW_MAX = 15,
		eCSV_COL_HEAD = eCSV_ROW_HEAD,
		eCSV_COL_COMMENT = eCSV_ROW_ERR_12,
		eCSV_COL_EN = eCSV_ROW_MSG_13,
		eCSV_COL_JA = eCSV_ROW_MSG_14,
		eCSV_COL_DE = eCSV_ROW_MSG_18,
		eCSV_COL_ES = eCSV_ROW_MSG_19,
		eCSV_COL_FR = eCSV_ROW_MSG_20_1,
		eCSV_COL_IT = eCSV_ROW_MSG_20_2,
		eCSV_COL_MAX = eCSV_ROW_MSG_20_3
	};

	static constexpr int STRING_BUFF_SIZE = 4096;

private:
	string m_Message[15]{};

	bool m_bValid{};

	void Init();

	void Term();

	void LoadValue();

public:
	cParamMessage();

	virtual ~cParamMessage();

	bool RegistData(string key);

	bool IsValid();

	string pStr(uint n);
};

}
