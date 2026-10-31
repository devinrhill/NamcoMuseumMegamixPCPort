#include "kingdom/cTaskKingdom.hpp"

#include <iterator>

#include "CustomKingdomUtil.hpp"

namespace kingdom
{

void cTaskKingdom::Init()
{
	std::fill(std::begin(m_Flow), std::end(m_Flow), nullptr);
	for (int i = 0; i < (int)std::size(m_Flow); i++)
	{
		m_Flow[i] = new cTaskKingdom::sFlow();
	}
	SetName("cTaskKingdom");
	nDrawPriority(65536u);
}

void cTaskKingdom::Term()
{
}

void cTaskKingdom::TermOnce()
{
	if (m_bTermDone) return;
	m_bTermDone = true;
	ReleaseTerm();
}

// DOL 0x80009dc8 [audited] (delete this: the dtor chain = ReleaseTerm)
void cTaskKingdom::DeleteExecute()
{
	TermOnce();
}

cTaskKingdom::cTaskKingdom()
	
{
	Init();
}

cTaskKingdom::sFlow*& cTaskKingdom::pFlow(uint n)
{
	CustomKingdomUtil::Assert(8 > n, "error");
	return m_Flow[n];
}

uint& cTaskKingdom::nModeState(uint n)
{
	return pFlow(n)->m_nModeState;
}

uint& cTaskKingdom::nOldState(uint n)
{
	return pFlow(n)->m_nOldState;
}

uint& cTaskKingdom::nNewState(uint n)
{
	return pFlow(n)->m_nNewState;
}

uint& cTaskKingdom::nModeStep(uint n)
{
	return pFlow(n)->m_nModeStep;
}

uint& cTaskKingdom::nModeProc(uint n, int work)
{
	CustomKingdomUtil::Assert(8 > work, "error");
	return pFlow(n)->m_nModeProc[work];
}

void cTaskKingdom::NextMode(uint n, uint mode)
{
	pFlow(n)->m_nModeState = mode;
	for (int i = 0; i < (int)std::size(m_Flow[n]->m_nModeProc); i++)
	{
		m_Flow[n]->m_nModeProc[i] = 0u;
	}
}

// DOL 0x800077f8 [audited]
void cTaskKingdom::nDrawPriority(uint n)
{
	CustomKingdomUtil::Assert(65536 <= n && n <= 4294901760u, "fatal error");
	cSysTaskUser::nDrawPriority(n);
}

// DOL 0x80009dfc [audited]
uint cTaskKingdom::nDrawPriority()
{
	return cSysTaskUser::nDrawPriority();
}

}
