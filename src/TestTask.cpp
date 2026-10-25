#include "TestTask.hpp"

TestTask::TestTask()
{
	Init();
}

void TestTask::Init()
{
	// TODO(unity): Debug.Log("\t [ Task ] \t " + GetType().Name + " / " + MethodBase.GetCurrentMethod().Name);
}

bool TestTask::Execute(cJetTask* pParam)
{
	// TODO(unity): Debug.Log("\t [ Task ] \t " + GetType().Name + " / " + MethodBase.GetCurrentMethod().Name);
	return true;
}
