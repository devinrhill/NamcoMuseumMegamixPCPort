#include "kingdom/cAppKingdom.hpp"

#include "WiiSystem.hpp"

#include "cJetTask.hpp"
#include "cSysCoreStructure.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskPBPtkMan.hpp"
#include "kingdom/cTaskScnRoot.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom
{

cAppKingdom::cAppKingdom()
{
	Init();
}

void cAppKingdom::Free()
{
	Term();
}

void cAppKingdom::DeleteExecute()
{
	if (m_bFreed) return;
	m_bFreed = true;
	Free();
}

cTaskScnRoot*& cAppKingdom::pTaskScnRoot()
{
	return m_pTaskScnRoot;
}

cTaskPBPtkMan*& cAppKingdom::pTaskPtkMan()
{
	return m_pTaskPtkMan;
}

bool& cAppKingdom::IsPause()
{
	return m_bPause;
}

void cAppKingdom::Init()
{
	m_bPause = false;
	m_pTaskPtkMan = nullptr;
	for (uint num = 0u; (long)num < (long)::Constants::ARRAY_SIZE(m_StopWatchs); num++)
	{
		m_StopWatchs[num].nStart = 0L;
		m_StopWatchs[num].nTotal = 0L;
		m_StopWatchs[num].bRunning = false;
	}
	SetName("cAppKingdom");
	KingdomGlobal::KingSys->pAppMain() = this;
	cSysCoreStructure* sysCore = KingdomGlobal::SysCore;
	cJetTask* pParam;
	sysCore->TaskExecuteParam(pParam);
	cSysCoreStructure* sysCore2 = KingdomGlobal::SysCore;
	pParam = this;
	sysCore2->TaskDrawParam(pParam);
}

void cAppKingdom::Term()
{
	KingdomGlobal::KingSys->pAppMain() = nullptr;
	KingdomGlobal::SysCore->TaskExecuteParam(nullptr);
	KingdomGlobal::SysCore->TaskDrawParam(nullptr);
}

void cAppKingdom::StopWatchReset(uint n)
{
	CustomKingdomUtil::Assert((long)n < (long)::Constants::ARRAY_SIZE(m_StopWatchs), "idx error");
	m_StopWatchs[n].bRunning = false;
	m_StopWatchs[n].nStart = 0L;
	m_StopWatchs[n].nTotal = 0L;
}

// DOL 0x80096b6c. The fields keep the DOL's meaning: nStart = time base value the elapsed time is measured from,
// nTotal = last sampled time (while stopped, elapsed = nTotal - nStart). Both in OSGetTime() ticks.
void cAppKingdom::StopWatchStart(uint n)
{
	CustomKingdomUtil::Assert((long)n < (long)::Constants::ARRAY_SIZE(m_StopWatchs), "idx error");
	const long now = (long)WiiSystem::OSGetTime();
	if (m_StopWatchs[n].bRunning)
	{
		m_StopWatchs[n].nStart = now;
		m_StopWatchs[n].nTotal = now;
	}
	else
	{
		m_StopWatchs[n].nStart = now - (m_StopWatchs[n].nTotal - m_StopWatchs[n].nStart);
		m_StopWatchs[n].nTotal = now;
	}
	m_StopWatchs[n].bRunning = true;
}

// DOL 0x80096c0c
void cAppKingdom::StopWatchStop(uint n)
{
	CustomKingdomUtil::Assert((long)n < (long)::Constants::ARRAY_SIZE(m_StopWatchs), "idx error");
	if (m_StopWatchs[n].bRunning)
	{
		m_StopWatchs[n].nTotal = (long)WiiSystem::OSGetTime();
	}
	m_StopWatchs[n].bRunning = false;
}

// DOL 0x80096c6c: elapsed milliseconds
uint cAppKingdom::StopWatchCheck(uint n)
{
	CustomKingdomUtil::Assert((long)n < (long)::Constants::ARRAY_SIZE(m_StopWatchs), "idx error");
	const long end = m_StopWatchs[n].bRunning ? (long)WiiSystem::OSGetTime() : m_StopWatchs[n].nTotal;
	return (uint)((end - m_StopWatchs[n].nStart) / WiiSystem::TicksPerMsec);
}

bool cAppKingdom::StopWatchState(uint n)
{
	CustomKingdomUtil::Assert((long)n < (long)::Constants::ARRAY_SIZE(m_StopWatchs), "idx error");
	return m_StopWatchs[n].bRunning;
}

}
