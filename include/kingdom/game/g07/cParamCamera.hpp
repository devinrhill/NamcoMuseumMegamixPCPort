#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g07
{

class cParamCamera : public cKingdomCSV
{
private:
	enum ECSV
	{
		eCSV_ROW_HEAD = 0,
		eCSV_ROW_PARAM = 1,
		eCSV_ROW_MAX = 2,
		eCSV_COL_POS_X = eCSV_ROW_HEAD,
		eCSV_COL_POS_Y = eCSV_ROW_PARAM,
		eCSV_COL_POS_Z = eCSV_ROW_MAX,
		eCSV_COL_TGT_X = 3,
		eCSV_COL_TGT_Y = 4,
		eCSV_COL_TGT_Z = 5,
		eCSV_COL_FOVY = 6,
		eCSV_COL_NEAR = 7,
		eCSV_COL_FAR = 8,
		eCSV_COL_ASPECT = 9,
		eCSV_COL_MAX = 10
	};

	Vector4 m_vPosition{};

	Vector4 m_vTarget{};

	float m_fFovy{};

	float m_fNear{};

	float m_fFar{};

	float m_fAspect{};

protected:
	void Init() override;

	void Term() override;

private:
	void LoadValue();

public:
	cParamCamera();

	cParamCamera(string key);

	virtual ~cParamCamera();

	bool RegistData(string key);

	Vector4 vGetPosition();

	Vector4 vGetTarget();

	float fGetFovy();

	float fGetNear();

	float fGetFar();

	float fGetAspect();
};

}
