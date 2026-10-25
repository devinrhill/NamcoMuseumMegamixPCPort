#include "cSysCoreStructure.hpp"

#include "Constants.hpp"
#include "InGamePadManager.hpp"
#include "JBC_Handler.hpp"
#include "JetNodeUtil.hpp"
#include "cJetNode.hpp"
#include "cJetTask.hpp"

const NODE_COMPARE_FUNC_PTR cSysCoreStructure::scTaskCompareFunc = TaskCompare;

bool cSysCoreStructure::TaskCompare(cJetNode* pNode0, cJetNode* pNode1)
{
	cJetTask* obj = static_cast<cJetTask*>(pNode0);
	cJetTask* cJetTask2 = static_cast<cJetTask*>(pNode1);
	if (obj->m_nDrawPriority > cJetTask2->m_nDrawPriority)
	{
		return true;
	}
	return false;
}

cSysCoreStructure::cSysCoreStructure()
	: job(new JBC_Handler())
{
	m_pTaskRoot = new cJetTask();
	m_pTaskRoot->SetName("ROOT");
	m_pTaskRoot->nDrawPriority(0u);
	m_pTaskSys = new cJetTask();
	m_pTaskSys->SetName("Sys Root");
	m_pTaskSys->nDrawPriority(0u);
	m_pTaskUsr = new cJetTask();
	m_pTaskUsr->SetName("Usr Root");
	m_pTaskUsr->nDrawPriority(65536u);
	m_pTaskRoot->ConnectChild(m_pTaskSys);
	m_pTaskRoot->ConnectChild(m_pTaskUsr);
	// DOL 0x80133000: after ROOT / Sys Root / Usr Root the constructor builds the six system nodes and
	// connects them under Sys Root; names from 0x804420f8, 0x80477f28, 0x804420ec, 0x80477f20, 0x804420e0, 0x80477f18.
	static const char* const kSysNodeNames[6] = {"Pad Sync", "G Start", "G Finish", "G Sync", "EFB 2 XFB", "V Sync"};
	for (int i = 0; i < 6; i++)
	{
		m_pSysTaskNode[i] = new cJetTask();
		m_pSysTaskNode[i]->SetName(kSysNodeNames[i]);
		m_pTaskSys->ConnectChild(m_pSysTaskNode[i]);
	}
	int pDrawStatus = 0;
	m_pExecParam = nullptr;
	m_pDrawParam = nullptr;
	m_pDrawStatus = pDrawStatus;
}

cSysCoreStructure::~cSysCoreStructure()
{
	Free();
}

void cSysCoreStructure::Free()
{
	m_pTaskRoot->Release();
}

void cSysCoreStructure::SetJobEnviron(uint msk, uint sft, uint ntbl, JBC_Element* tbl)
{
	Constants::Jbc_Init(*job);
	Constants::Jbc_SetConfig(*job, msk, sft, ntbl, tbl);
}

uint cSysCoreStructure::SetJobNo(uint prm)
{
	uint result = Constants::Jbc_GetProcNo(*job);
	Constants::Jbc_SetProcNo(*job, prm);
	return result;
}

uint cSysCoreStructure::SetForceJobNo(uint prm)
{
	uint result = Constants::Jbc_GetProcNo(*job);
	Constants::Jbc_SetForceProcNo(*job, prm);
	return result;
}

uint cSysCoreStructure::ForceSetJobNo(uint prm)
{
	return SetForceJobNo(prm);
}

uint cSysCoreStructure::GetJobNo()
{
	return Constants::Jbc_GetProcNo(*job);
}

uint cSysCoreStructure::GetLastJobNo()
{
	return Constants::Jbc_GetProcNoOld(*job);
}

void cSysCoreStructure::RegistMainTask(cJetTask* tsk, uint prio)
{
	tsk->nDrawPriority(prio);
	RegistMainTask(tsk);
}

void cSysCoreStructure::RegistMainTask(cJetTask* tsk, bool isExecGCCollect)
{
	if (isExecGCCollect)
	{
		// TODO(unity): Resources.UnloadUnusedAssets();
		// TODO(unity): GC.Collect();
	}
	m_pTaskUsr->ConnectChild(tsk);
}

void cSysCoreStructure::ReleaseMainTask()
{
	if (m_pTaskUsr->pChild() != nullptr)
	{
		m_pTaskUsr->pChild()->Release();
	}
}

// DOL 0x80133a08: switch over the six nodes (any other id is ignored); ConnectChild(node, tsk, 0)
void cSysCoreStructure::RegistSysTaskChild(uint systsk_name, cJetTask* tsk)
{
	if (systsk_name < 6)
	{
		m_pSysTaskNode[systsk_name]->ConnectChild(tsk);
	}
}

// DOL 0x80133aa0: releases the node's first child, if any
void cSysCoreStructure::ReleaseSysTaskChild(uint systsk_name)
{
	if (systsk_name < 6 && m_pSysTaskNode[systsk_name]->pChild() != nullptr)
	{
		m_pSysTaskNode[systsk_name]->pChild()->Release();
	}
}

void cSysCoreStructure::Process()
{
	JobProc();
	TaskProc();
}

void cSysCoreStructure::JobProc()
{
	Constants::Jbc_ExecProc(*job);
}

void cSysCoreStructure::TaskProc()
{
	if (!InGamePadManager::get_Disable())
	{
		TaskExecAll();
	}
	TaskDrawAll();
}

void cSysCoreStructure::TaskExecuteParam(cJetTask* const& pParam)
{
	m_pExecParam = pParam;
}

void cSysCoreStructure::TaskDrawParam(cJetTask* const& pParam)
{
	m_pDrawParam = pParam;
}

void cSysCoreStructure::TaskDrawStatus(const int& pStatus)
{
	m_pDrawStatus = pStatus;
}

void cSysCoreStructure::TaskExecAll(bool loading)
{
	cJetTask* pTaskRoot = m_pTaskRoot;
	cJetTask::PreExecuteAll(pTaskRoot, m_pExecParam);
	cJetTask::ExecuteAll(pTaskRoot, m_pExecParam);
	cJetTask::PostExecuteAll(pTaskRoot, m_pExecParam);
}

void cSysCoreStructure::TaskDrawAll(bool loading)
{
	cJetNode** pTaskSortBuf = (cJetNode**)m_pTaskSortBuf;
	int num = JetNodeUtil::NodeUtilListUpNodeSort(pTaskSortBuf, m_pTaskRoot, 512, scTaskCompareFunc);
	for (int i = 0; i < num; i++)
	{
		m_pTaskSortBuf[i]->PreDraw(m_pDrawStatus, m_pDrawParam);
	}
	for (int j = 0; j < num; j++)
	{
		m_pTaskSortBuf[j]->Draw(m_pDrawStatus, m_pDrawParam);
	}
	for (int k = 0; k < num; k++)
	{
		m_pTaskSortBuf[k]->PostDraw(m_pDrawStatus, m_pDrawParam);
	}
}
