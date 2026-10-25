#include "kingdom/game/g07/cParamGame.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g07
{

void cParamGame::Init()
{
	m_bValid = false;
}

void cParamGame::Term()
{
}

void cParamGame::LoadValue()
{
	CustomKingdomUtil::Assert(0 <= nRow(), "csv error");
	m_bValid = true;
}

cParamGame::cParamGame()
{
	Init();
}

cParamGame::cParamGame(string key)
{
	Init();
	RegistData(key);
}

cParamGame::~cParamGame()
{
	Term();
}

bool cParamGame::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);
	CustomKingdomUtil::Assert(result, "csv error");
	LoadValue();
	return result;
}

}
