#include "kingdom/game/g03/cGhostParam.hpp"

#include "CustomKingdomUtil.hpp"

namespace kingdom::game::g03
{

void cGhostParam::Init()
{
	m_bValid = false;
}

void cGhostParam::Term()
{
}

cGhostParam::cGhostParam()
{
	Init();
}

cGhostParam::cGhostParam(string key)
{
	Init();
	RegistData(key);
}

cGhostParam::~cGhostParam()
{
	Term();
}

bool cGhostParam::RegistData(string key)
{
	bool result = cKingdomCSV::RegistData(key);   // the converted code called itself (overload lost)
	CustomKingdomUtil::Assert(result, "csv error");
	m_bValid = true;
	return result;
}

bool cGhostParam::IsValid()
{
	return m_bValid;
}

int cGhostParam::KindChange(int id)
{
	id = ((id == 1) ? 1 : ((id == 2) ? 2 : ((id == 3) ? 3 : ((id == 4) ? 4 : ((id == 8) ? 6 : ((id == 9) ? 7 : ((id == 10) ? 8 : ((id == 11) ? 9 : ((id == 12) ? 10 : ((id == 13) ? 11 : (0)))))))))));
	if (id == 0)
	{
		// TODO(unity): Debug.LogFormat("UNKNOWNN GHOST\n");
	}
	return id;
}

float cGhostParam::GetGhostSpeed(int id)
{
	id = KindChange(id);
	if (id == 0)
	{
		return 0.0f;
	}
	return F32(id, 1, 0.0f);
}

float cGhostParam::GetGhostAddSpeed(int id)
{
	id = KindChange(id);
	if (id == 0)
	{
		return 0.0f;
	}
	return F32(id, 2, 0.0f);
}

float cGhostParam::GetGhostTurnSpeed(int id)
{
	id = KindChange(id);
	if (id == 0)
	{
		return 0.0f;
	}
	return F32(id, 3, 0.0f);
}

float cGhostParam::GetGhostHitSize(int id)
{
	id = KindChange(id);
	if (id == 0)
	{
		return 0.0f;
	}
	return F32(id, 4, 0.0f);
}

}
