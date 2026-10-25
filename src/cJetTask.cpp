#include "cJetTask.hpp"

cJetTask::cJetTask()
{
	SetName("cJetTask");
	m_nStatus = 0u;
	m_nDrawPriority = 2147483648u;
	m_AutoRelease = false;
}

cJetTask::~cJetTask()
{
}

bool cJetTask::PreExecute(cJetTask* pParam)
{
	return true;
}

bool cJetTask::Execute(cJetTask* pParam)
{
	return true;
}

bool cJetTask::PostExecute(cJetTask* pParam)
{
	return true;
}

void cJetTask::PreDraw(int nStatus, cJetTask* pParam)
{
}

void cJetTask::Draw(int nStatus, cJetTask* pParam)
{
}

void cJetTask::PostDraw(int nStatus, cJetTask* pParam)
{
}

uint cJetTask::nDrawPriority()
{
	return m_nDrawPriority;
}

void cJetTask::nDrawPriority(uint n)
{
	m_nDrawPriority = n;
}

bool cJetTask::IsAutoRelease()
{
	return m_AutoRelease;
}

void cJetTask::AutoRelease(bool flag)
{
	m_AutoRelease = flag;
}

void cJetTask::PreExecuteAll(cJetTask* pRoot, cJetTask* pParam, bool bReleaseFlag)
{
	cJetTask* cJetTask3;
	cJetTask* cJetTask2 = (cJetTask3 = pRoot);
	while (cJetTask3 != nullptr)
	{
		if (!cJetTask3->PreExecute(pParam) & bReleaseFlag)
		{
			cJetTask3->Release();
			cJetTask3 = ((cJetTask3 == pRoot) ? nullptr : static_cast<cJetTask*>(cJetTask2->SearchNextNode(pRoot)));
		}
		else
		{
			cJetTask2 = cJetTask3;
			cJetTask3 = static_cast<cJetTask*>(cJetTask3->SearchNextNode(pRoot));
		}
	}
}

void cJetTask::ExecuteAll(cJetTask* pRoot, cJetTask* pParam, bool bReleaseFlag)
{
	cJetTask* cJetTask3;
	cJetTask* cJetTask2 = (cJetTask3 = pRoot);
	while (cJetTask3 != nullptr)
	{
		if (!cJetTask3->Execute(pParam) & bReleaseFlag)
		{
			cJetTask3->Release();
			cJetTask3 = ((cJetTask3 == pRoot) ? nullptr : static_cast<cJetTask*>(cJetTask2->SearchNextNode(pRoot)));
		}
		else
		{
			cJetTask2 = cJetTask3;
			cJetTask3 = static_cast<cJetTask*>(cJetTask3->SearchNextNode(pRoot));
		}
	}
}

void cJetTask::PostExecuteAll(cJetTask* pRoot, cJetTask* pParam, bool bReleaseFlag)
{
	cJetTask* cJetTask3;
	cJetTask* cJetTask2 = (cJetTask3 = pRoot);
	while (cJetTask3 != nullptr)
	{
		if (!cJetTask3->PostExecute(pParam) & bReleaseFlag)
		{
			cJetTask3->Release();
			cJetTask3 = ((cJetTask3 == pRoot) ? nullptr : static_cast<cJetTask*>(cJetTask2->SearchNextNode(pRoot)));
		}
		else
		{
			cJetTask2 = cJetTask3;
			cJetTask3 = static_cast<cJetTask*>(cJetTask3->SearchNextNode(pRoot));
		}
	}
}
