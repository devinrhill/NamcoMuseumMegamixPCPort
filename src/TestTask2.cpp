#include "TestTask2.hpp"

TestTask2::TestTask2()
{
	Init();
}

void TestTask2::Init()
{
	// TODO(unity): Debug.Log("\t [ Task ] \t " + GetType().Name + " / " + MethodBase.GetCurrentMethod().Name);
}

bool TestTask2::Execute(cJetTask* pParam)
{
	// TODO(unity): Debug.Log("\t [ Task ] \t " + GetType().Name + " / " + MethodBase.GetCurrentMethod().Name);
	return true;
}
