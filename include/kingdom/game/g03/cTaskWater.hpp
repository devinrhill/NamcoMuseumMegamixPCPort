#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskWater : public cTaskKingdom
{
public:
	class sWaterArea
	{
	public:
		Vector4 vTrans{};

		Vector4 vWaterForce{};

		float fLengthX{};

		float fLengthZ{};

		float fForce{};
	};

	enum EWater
	{
		eWATER_AREA_NONE = 0,
		eWATER_AREA_UP = 1,
		eWATER_AREA_IN = 2
	};

private:
	std::vector<cTaskWater::sWaterArea*> m_vecTaskWater;

public:
	std::vector<cTaskWater::sWaterArea*>& GetWaterAreaList();

	cTaskWater();

	virtual ~cTaskWater();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	uint AreaCheck(Vector4 trans, float radius, Vector4& outWaterForce, float& outWaterHeight);
};

}
