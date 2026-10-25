#pragma once

#include "common.hpp"
#include "cJetNode.hpp"

class cJetTask : public cJetNode
{
public:
	static constexpr uint JET_TASK_PRIO_DEFAULT = 2147483648u;

private:
	uint m_nStatus{};

public:
	uint m_nDrawPriority{};

private:
	bool m_AutoRelease{};

public:
	cJetTask();

	virtual ~cJetTask();

	virtual bool PreExecute(cJetTask* pParam);

	virtual bool Execute(cJetTask* pParam);

	virtual bool PostExecute(cJetTask* pParam);

	virtual void PreDraw(int nStatus, cJetTask* pParam);

	virtual void Draw(int nStatus, cJetTask* pParam);

	virtual void PostDraw(int nStatus, cJetTask* pParam);

	virtual uint nDrawPriority();

	virtual void nDrawPriority(uint n);

	virtual bool IsAutoRelease();

	virtual void AutoRelease(bool flag);

	static void PreExecuteAll(cJetTask* pRoot, cJetTask* pParam = nullptr, bool bReleaseFlag = true);

	static void ExecuteAll(cJetTask* pRoot, cJetTask* pParam = nullptr, bool bReleaseFlag = true);

	static void PostExecuteAll(cJetTask* pRoot, cJetTask* pParam = nullptr, bool bReleaseFlag = true);
};
