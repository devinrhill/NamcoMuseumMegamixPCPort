#pragma once

#include "common.hpp"
#include "CSVParam.hpp"

class cJetCSV
{
protected:
	bool m_bValid{};

	CSVParam* m_pCSV{};

public:
	cJetCSV();

	cJetCSV(CSVParam* const& csv, uint size, uint analyze = 0u);

	virtual ~cJetCSV();

protected:
	virtual void Init();

	virtual void Term();

public:
	bool SetData(CSVParam* const& csv, uint size = 0u, uint analyze = 0u);

	std::vector<string> GetData();

	bool IsValid();

	int nRow();

	int nCol();

	string Cell(int row, int col);

	bool Cell(int row, int col, string& _out);

	string Str(int row, int col, string errorValue);

	bool Str(int row, int col, string& _out, uint size);

	// The C# had `S32(row, col, errorValue)` and `S32(row, col, out value)`; the conversion lost `out`, so an lvalue argument is the out version
	// (it used to bind to the by-value overload and silently dropped the result), an rvalue is the error value.
	int S32(int row, int col, int&& errorValue);
	bool S32(int row, int col, int& _out) { return S32(row, col, &_out); }
	bool S32(int row, int col, unsigned int& _out) { int t = 0; const bool ok = S32(row, col, &t); _out = (unsigned int)t; return ok; }

	bool S32(int row, int col, int* _out);

	float F32(int row, int col, float&& errorValue);
	bool F32(int row, int col, float& _out) { return F32(row, col, &_out); }

	bool F32(int row, int col, float* _out);

	int SearchRow(int col, string searchStr, int startRow = 0, int endRow = 0);

	int SearchCol(int row, string searchStr, int startCol = 0, int endCol = 0);
};
