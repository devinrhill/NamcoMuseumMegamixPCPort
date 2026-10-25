#include "kingdom/cKingdomCSV.hpp"

#include "CSVParam.hpp"
#include "CustomKingdomUtil.hpp"
#include "wii/Archive.hpp"

#include <map>
#include <memory>
#include <sstream>

namespace kingdom
{

bool cKingdomCSV::SetData(CSVParam* const& csv, uint size, uint analyze)
{
	CustomKingdomUtil::Assert("issue a prohibition");
	return true;
}

std::vector<string> cKingdomCSV::GetData()
{
	CustomKingdomUtil::Assert("issue a prohibition");
	return {};
}

cKingdomCSV::~cKingdomCSV()
{
}

// The CSV parameter files are plain text entries of the attached archives ("BIN_CSV_...", "BIN_COOKIE_NUM_CSV"); the Unity build had
// them as CSVParam assets. Rows are padded to the widest row, cells are row * nCol + col (cJetCSV::Cell). Quoted cells lose their quotes.
bool cKingdomCSV::RegistData(const string& key)
{
	static std::map<std::string, std::unique_ptr<CSVParam>> cache;
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(key, size);
	if (d == nullptr)
	{
		CustomKingdomUtil::Assert(false, "could not load " + key);
		return false;
	}
	auto param = std::make_unique<CSVParam>();
	std::vector<std::vector<string>> rows;
	std::istringstream in(std::string((const char*)d, size));
	std::string line;
	int maxCol = 0;
	while (std::getline(in, line))
	{
		if (!line.empty() && line.back() == '\r') line.pop_back();
		std::vector<string> cells;
		std::stringstream ls(line);
		std::string tok;
		while (std::getline(ls, tok, ','))
		{
			// a quoted cell ("e_elevator01" in g01 Demo_s1.csv) is found by the DOL under its bare text: the quotes go (UNVERIFIED: the
			// exact rules of the DOL's cJetCSV parser 0x80139428, e.g. commas inside quotes, are not read yet)
			if (tok.size() >= 2 && tok.front() == '"' && tok.back() == '"') tok = tok.substr(1, tok.size() - 2);
			cells.push_back(tok);
		}
		if (!line.empty() && line.back() == ',') cells.push_back(string());
		maxCol = std::max(maxCol, (int)cells.size());
		rows.push_back(std::move(cells));
	}
	param->m_maxRow = (int)rows.size();
	param->m_maxCol = maxCol;
	for (auto& r : rows)
	{
		r.resize(maxCol);
		for (auto& c : r) param->m_csvData.push_back(c);
	}
	CSVParam* raw = param.get();
	cache[key] = std::move(param); // replaced data stays valid for the lifetime of the process
	return cJetCSV::SetData(raw);
}

std::vector<string> cKingdomCSV::pCSV()
{
	return cJetCSV::GetData();
}

}
