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

void cParamMessage::LoadValue()
{
	CustomKingdomUtil::Assert(15 <= nRow(), "csv error");
	CustomKingdomUtil::Assert(8 <= nCol(), "csv error");
	int col = 1;
	for (int i = 1; i < 15; i++)
	{
		Str(i, col, m_Message[i], 4096u);
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
	bool result = RegistData(key);
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
