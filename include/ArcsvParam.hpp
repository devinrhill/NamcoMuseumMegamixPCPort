#pragma once

#include <vector>
#include "common.hpp"

class ArcsvParam {
public:
	class ArcsvData
	{
	public:
		string key{};

		string path{};
	};

	std::vector<ArcsvData*> list;

	string Get(string key);
};
