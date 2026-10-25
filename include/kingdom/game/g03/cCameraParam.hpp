#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g03/Constants.hpp"

namespace kingdom::game::g03
{

class cCameraParam : public cKingdomCSV
{
public:
	enum EValue
	{
		VALUE_F32 = 0,
		VALUE_S32 = 1
	};

private:
	static const string s_pParamName[22];

	static Constants::CAM_PARAM* s_StanDard;

	uint m_nValueFlag{};

	Constants::CAM_PARAM* m_Param{};

public:
	cCameraParam(const string& key);

	virtual ~cCameraParam();

private:
	void Init();

	void Term();

	bool RegistData(const string& key);

public:
	uint CheckValue(int nKind);

	Constants::CAM_PARAM* GetParam();

	int GetCameraType();

	float GetParamHeight();

	float GetParamDistance();

	float GetInitAngle();

private:
	void LoadValue();

public:
	void GetParam(Constants::CAM_PARAM*& pParam);

private:
	bool LoadValue(int nID, float* pData, int nType, int nNum = 1);

	bool LoadValue(int nID, float& pData, int nType);

	bool LoadValue(int nID, int& pData, int nType);

public:
	static Constants::CAM_PARAM* GetStandardParam();

private:
	static void MERGE_VAL(Constants::CAM_PARAM* p1, Constants::CAM_PARAM* p2, int& val1, int val2, int def);

	static void MERGE_VAL(Constants::CAM_PARAM* p1, Constants::CAM_PARAM* p2, float& val1, float val2, int def);

public:
	static void MergeParam(Constants::CAM_PARAM*& pDst, Constants::CAM_PARAM* const& pDefault);

	static void InitStandardParam();
};

}
