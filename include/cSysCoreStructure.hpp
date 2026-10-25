#pragma once

#include "common.hpp"
#include "NODE_COMPARE_FUNC_PTR.hpp"
#include "cJetNode.hpp"
#include "cJetTask.hpp"

class JBC_Handler;
class JBC_Element;

class cSysCoreStructure
{
private:
	JBC_Handler* job;

	cJetTask* m_pTaskRoot{};

	cJetTask* m_pTaskUsr{};

	cJetTask* m_pTaskSys{};

	// The six system task nodes under "Sys Root" (DOL core + 0x2c..0x40): Pad Sync, G Start, G Finish, G Sync,
	// EFB 2 XFB, V Sync. RegistSysTaskChild(n, task) hangs a task under node n.
	cJetTask* m_pSysTaskNode[6]{};

	cJetTask* m_pTaskSortBuf[512]{};

public:
	cJetTask* m_pExecParam{};

	cJetTask* m_pDrawParam{};

	int m_pDrawStatus{};

private:
	static const NODE_COMPARE_FUNC_PTR scTaskCompareFunc;

public:
	static bool TaskCompare(cJetNode* pNode0, cJetNode* pNode1);

	cSysCoreStructure();

	cJetTask* pTaskRoot() { return m_pTaskRoot; }

	virtual ~cSysCoreStructure();

	void Free();

	void SetJobEnviron(uint msk, uint sft, uint ntbl, JBC_Element* tbl);

	uint SetJobNo(uint prm);

	uint SetForceJobNo(uint prm);

	uint ForceSetJobNo(uint prm);

	uint GetJobNo();

	uint GetLastJobNo();

	void RegistMainTask(cJetTask* tsk, uint prio);

	void RegistMainTask(cJetTask* tsk, bool isExecGCCollect = true);

	void ReleaseMainTask();

	void RegistSysTaskChild(uint systsk_name, cJetTask* tsk);

	void ReleaseSysTaskChild(uint systsk_name);

	void Process();

protected:
	void JobProc();

	void TaskProc();

public:
	void TaskExecuteParam(cJetTask* const& pParam);

	void TaskDrawParam(cJetTask* const& pParam);

	void TaskDrawStatus(const int& pStatus);

	void TaskExecAll(bool loading = false);

	void TaskDrawAll(bool loading = false);
};
