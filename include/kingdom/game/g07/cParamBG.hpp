#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g07
{

class cParamBG : public cKingdomCSV
{
private:
	enum ECSV
	{
		eCSV_ROW_HEAD = 0,
		eCSV_ROW_BG_1 = 1,
		eCSV_ROW_BG_2 = 2,
		eCSV_ROW_BG_3 = 3,
		eCSV_ROW_BG_4 = 4,
		eCSV_ROW_MAX = 5,
		eCSV_COL_TRANS_X = eCSV_ROW_HEAD,
		eCSV_COL_TRANS_Y = eCSV_ROW_BG_1,
		eCSV_COL_TRANS_Z = eCSV_ROW_BG_2,
		eCSV_COL_ROTATE_X = eCSV_ROW_BG_3,
		eCSV_COL_ROTATE_Y = eCSV_ROW_BG_4,
		eCSV_COL_ROTATE_Z = eCSV_ROW_MAX,
		eCSV_COL_SCALE_X = 6,
		eCSV_COL_SCALE_Y = 7,
		eCSV_COL_SCALE_Z = 8,
		eCSV_COL_FRAME = 9,
		eCSV_COL_MAX = 10
	};

	enum EBG
	{
		BG_MAX = 4
	};

	struct Param
	{
	public:
		Vector4 vTrans{};

		Vector4 vRotate{};

		Vector4 vScale{};

		float fFrameRate{};
	};

	cParamBG::Param m_Param[4]{};

protected:
	void Init() override;

	void Term() override;

private:
	void LoadValue();

public:
	cParamBG();

	cParamBG(string key);

	virtual ~cParamBG();

	bool RegistData(string key);

	Vector4 vGetTrans(uint n);

	Vector4 vGetRotate(uint n);

	Vector4 vGetScale(uint n);

	float fGetFrameRate(uint n);
};

}
