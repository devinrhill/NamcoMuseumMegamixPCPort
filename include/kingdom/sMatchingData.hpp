#pragma once

#include "common.hpp"

namespace kingdom
{

class sMatchingData
{
public:
	class InfoData
	{
	public:
		string Name{};

		uint nCtrl{};

		uint nWork{};
	};

	uint nNum{};

	sMatchingData::InfoData* Info[4]{};

	sMatchingData();

	sMatchingData* Clone();
};

}
