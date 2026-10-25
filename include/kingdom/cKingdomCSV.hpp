#pragma once

#include "common.hpp"
#include "cJetCSV.hpp"
#include "CSVParam.hpp"

namespace kingdom
{

class cKingdomCSV : public cJetCSV
{
public:
	bool SetData(CSVParam* const& csv, uint size, uint analyze = 0u);

	std::vector<string> GetData();

	virtual ~cKingdomCSV();

	bool RegistData(const string& key);

	std::vector<string> pCSV();
};

}
