#pragma once

#include "common.hpp"

// [Serializable]
class CSVParam : public ScriptableObject
{
public:
	int m_maxRow{};

	int m_maxCol{};

	std::vector<string> m_csvData;
};
