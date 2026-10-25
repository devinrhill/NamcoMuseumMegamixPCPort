#include "cJetCSV.hpp"

#include "CSVParam.hpp"
#include "CSharpCompat.hpp"

cJetCSV::cJetCSV()
{
	Init();
}

cJetCSV::cJetCSV(CSVParam* const& csv, uint size, uint analyze)
{
	Init();
	SetData(csv, size, analyze);
}

cJetCSV::~cJetCSV()
{
	Term();
}

void cJetCSV::Init()
{
	m_bValid = false;
	m_pCSV = nullptr;
}

void cJetCSV::Term()
{
}

bool cJetCSV::SetData(CSVParam* const& csv, uint size, uint analyze)
{
	m_bValid = false;
	m_pCSV = csv;
	m_bValid = true;
	return true;
}

std::vector<string> cJetCSV::GetData()
{
	return std::vector<string>(m_pCSV->m_csvData.begin(), m_pCSV->m_csvData.end());
}

bool cJetCSV::IsValid()
{
	return m_bValid;
}

int cJetCSV::nRow()
{
	if (!(m_pCSV != nullptr))
	{
		return 0;
	}
	return m_pCSV->m_maxRow;
}

int cJetCSV::nCol()
{
	if (!(m_pCSV != nullptr))
	{
		return 0;
	}
	return m_pCSV->m_maxCol;
}

string cJetCSV::Cell(int row, int col)
{
	string _out;
	if (!Cell(row, col, _out))
	{
		return string();
	}
	return _out;
}

bool cJetCSV::Cell(int row, int col, string& _out)
{
	_out = string();
	int num = row * nCol() + col;
	if (row >= 0 && col >= 0 && num < (int)m_pCSV->m_csvData.size())
	{
		_out = m_pCSV->m_csvData[num];
		return true;
	}
	return false;
}

string cJetCSV::Str(int row, int col, string errorValue)
{
	string _out;
	if (!Str(row, col, _out, 0u))
	{
		return errorValue;
	}
	return _out;
}

bool cJetCSV::Str(int row, int col, string& _out, uint size)
{
	return Cell(row, col, _out);
}

int cJetCSV::S32(int row, int col, int&& errorValue)
{
	int _out;
	if (!S32(row, col, &_out))
	{
		return errorValue;
	}
	return _out;
}

bool cJetCSV::S32(int row, int col, int* _out)
{
	*_out = 0;
	string text = Cell(row, col);
	int num = CSharpCompat::LastIndexOf(text, 'f');
	if (0 <= num)
	{
		text = text.substr(0, num);
	}
	text = CSharpCompat::Replace(text, "\"", "");
	int result;
	if (CSharpCompat::TryParseInt(text, result))
	{
		*_out = result;
		return true;
	}
	float result2;
	if (CSharpCompat::TryParseFloat(text, result2))
	{
		*_out = (int)result2;
		return true;
	}
	return false;
}

float cJetCSV::F32(int row, int col, float&& errorValue)
{
	float _out;
	if (!F32(row, col, &_out))
	{
		return errorValue;
	}
	return _out;
}

bool cJetCSV::F32(int row, int col, float* _out)
{
	*_out = 0.0f;
	string text = Cell(row, col);
	int num = CSharpCompat::LastIndexOf(text, 'f');
	if (0 <= num)
	{
		text = text.substr(0, num);
	}
	text = CSharpCompat::Replace(text, "\"", "");
	float result;
	if (CSharpCompat::TryParseFloat(text, result))
	{
		*_out = result;
		return true;
	}
	int result2;
	if (CSharpCompat::TryParseInt(text, result2))
	{
		*_out = result2;
		return true;
	}
	return false;
}

int cJetCSV::SearchRow(int col, string searchStr, int startRow, int endRow)
{
	if (endRow <= 0)
	{
		endRow = m_pCSV->m_maxRow;
	}
	for (int i = startRow; i < endRow; i++)
	{
		if (Cell(i, col) == searchStr)
		{
			return i;
		}
	}
	return -1;
}

int cJetCSV::SearchCol(int row, string searchStr, int startCol, int endCol)
{
	if (endCol <= 0)
	{
		endCol = m_pCSV->m_maxCol;
	}
	for (int i = startCol; i < endCol; i++)
	{
		if (Cell(row, i) == searchStr)
		{
			return i;
		}
	}
	return -1;
}
