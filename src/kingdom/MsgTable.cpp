#include "kingdom/MsgTable.hpp"

#include "wii/Archive.hpp"

namespace kingdom
{
namespace
{
constexpr int ENGLISH_COLUMN = 2;
}

bool MsgTable::Load(const char* entry)
{
	m_rows.clear();
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(entry, size);
	if (d == nullptr) return false;
	std::vector<std::string> row(1);
	bool quoted = false;
	for (std::size_t i = 0; i < size; i++)
	{
		const char c = (char)d[i];
		if (quoted)
		{
			if (c == '"')
			{
				if (i + 1 < size && d[i + 1] == '"') { row.back() += '"'; i++; }
				else quoted = false;
			}
			else row.back() += c;
		}
		else if (c == '"') quoted = true;
		else if (c == ',') row.emplace_back();
		else if (c == '\r') continue;
		else if (c == '\n') { m_rows.push_back(std::move(row)); row.assign(1, std::string()); }
		else row.back() += c;
	}
	if (row.size() > 1 || !row[0].empty()) m_rows.push_back(std::move(row));
	// literal "\n" -> line break
	for (auto& r : m_rows)
		for (auto& cell : r)
			for (std::size_t p = cell.find("\\n"); p != std::string::npos; p = cell.find("\\n", p + 1)) cell.replace(p, 2, "\n");
	return true;
}

std::string MsgTable::Get(int index) const
{
	const int row = index + 1;
	if (row < 1 || row >= (int)m_rows.size() || (int)m_rows[row].size() <= ENGLISH_COLUMN) return {};
	return m_rows[row][ENGLISH_COLUMN];
}

std::string MsgTable::Cell(int row, int col) const
{
	if (row < 0 || row >= (int)m_rows.size() || col < 0 || col >= (int)m_rows[row].size()) return {};
	return m_rows[row][col];
}
}
