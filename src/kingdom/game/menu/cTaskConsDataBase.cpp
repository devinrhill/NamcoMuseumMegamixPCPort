#include "kingdom/game/menu/cTaskConsDataBase.hpp"

namespace kingdom::game::menu
{

int cTaskConsDataBase::m_nSelectDataBase = -1;

int cTaskConsDataBase::nGetDataBaseAnmNo(uint nState)
{
	return 0;
}

int& cTaskConsDataBase::nSelectDataBase()
{
	return m_nSelectDataBase;
}

cTaskConsDataBase::~cTaskConsDataBase()
{
}

int cTaskConsDataBase::nGetSelectDataBase()
{
	return m_nSelectDataBase;
}

void cTaskConsDataBase::nSetSelectDataBase(int nSelect)
{
	m_nSelectDataBase = nSelect;
}

cTaskConsDataBase::cTaskConsDataBase(int nId, string Key)
	: cTaskConsMatching(nId)
{
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetState(1u, false);
}

bool cTaskConsDataBase::AppearProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetDataBaseAnmNo(0u));
		pAnita(0u)->SetState(1u);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

void cTaskConsDataBase::WaitProc()
{
	if (nSelectDataBase() == nId())
	{
		pAnita(0u)->SetAnimNo(nGetDataBaseAnmNo(2u), false);
	}
	else
	{
		pAnita(0u)->SetAnimNo(nGetDataBaseAnmNo(1u), false);
	}
}

void cTaskConsDataBase::PointProc()
{
	if (nSelectDataBase() == nId())
	{
		pAnita(0u)->SetAnimNo(nGetDataBaseAnmNo(4u), false);
	}
	else
	{
		pAnita(0u)->SetAnimNo(nGetDataBaseAnmNo(3u), false);
	}
}

bool cTaskConsDataBase::DecideProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetDataBaseAnmNo(5u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		nSelectDataBase() = nId();
		return true;
	}
	return false;
}

bool cTaskConsDataBase::DeleteProc()
{
	if (nStep() == 0)
	{
		pAnita(0u)->SetAnimNo(nGetDataBaseAnmNo(6u), false);
		nStep()++;
	}
	else if (nStep() == 1 && pAnita(0u)->IsState(65536u))
	{
		return true;
	}
	return false;
}

}
