#include "kingdom/game/menu/cParamMessage.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::menu
{

void cParamMessage::Init()
{
	m_bValid = false;
	for (uint num = 0u; num < 15; num++)
	{
	}
}

void cParamMessage::Term()
{
}

// DOL: inlined in the matching RegistData 0x801cf8bc: rows 1..14 of the language column (KingSys +0xa44: English and unknown languages -> column 2)
// through FUN_801394c8 (error text "LOAD_CSV_ERROR"), row 0 = the fixed error text, then the valid byte +0xf020
void cParamMessage::LoadValue()
{
	CustomKingdomUtil::Assert(15 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(8 <= nCol(), "csv error");
	int col = eCSV_COL_EN;   // locale fixed to en (the C# read column 1, the comment)
	for (int i = 1; i < 15; i++)
	{
		if (!Str(i, col, m_Message[i], 4096u)) m_Message[i] = "LOAD_CSV_ERROR";
		// the cell keeps a literal "\n" (as kingdom::MsgTable): a line break
		for (std::size_t p = m_Message[i].find("\\n"); p != string::npos; p = m_Message[i].find("\\n", p + 1)) m_Message[i].replace(p, 2, "\n");
	}
	m_Message[0] = "ERROR !! ERROR !! ERROR !! ERROR !!";
	m_bValid = true;
}

cParamMessage::cParamMessage()
{
	Init();
}

cParamMessage::~cParamMessage()
{
	Term();
}

bool cParamMessage::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);   // the C# called itself here (endless recursion)
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

bool cParamMessage::IsValid()
{
	return m_bValid;
}

string cParamMessage::pStr(uint n)
{
	CustomKingdomUtil::Assert(n < 15, "error");
	return m_Message[n];
}

}
