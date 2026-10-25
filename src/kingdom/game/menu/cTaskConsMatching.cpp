#include "kingdom/game/menu/cTaskConsMatching.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::menu
{

void cTaskConsMatching::WaitProc()
{
}

void cTaskConsMatching::PointProc()
{
}

bool cTaskConsMatching::AppearProc()
{
	return true;
}

bool cTaskConsMatching::DecideProc()
{
	return true;
}

bool cTaskConsMatching::DeleteProc()
{
	return true;
}

cTaskConsMatching::~cTaskConsMatching()
{
}

void cTaskConsMatching::Appear()
{
	nState() = 1;
	nStep() = 0;
}

bool cTaskConsMatching::IsAppear()
{
	return nState() == 1;
}

bool cTaskConsMatching::IsAppearEnd()
{
	return nState() == 2;
}

void cTaskConsMatching::Wait()
{
	nState() = 3;
	nStep() = 0;
}

bool cTaskConsMatching::IsWait()
{
	return nState() == 3;
}

void cTaskConsMatching::Point()
{
	nState() = 4;
	nStep() = 0;
}

bool cTaskConsMatching::IsPoint()
{
	return nState() == 4;
}

void cTaskConsMatching::Decide()
{
	nState() = 5;
	nStep() = 0;
}

bool cTaskConsMatching::IsDecide()
{
	return nState() == 5;
}

bool cTaskConsMatching::IsDecideEnd()
{
	return nState() == 6;
}

void cTaskConsMatching::Delete()
{
	nState() = 7;
	nStep() = 0;
}

bool cTaskConsMatching::IsDelete()
{
	return nState() == 7;
}

bool cTaskConsMatching::IsDeleteEnd()
{
	return nState() == 8;
}

cTaskConsMatching::cTaskConsMatching(int nId)
	: cTaskConsBase(nId)
{
	nState() = 0;
	nDrawPriority(2147483648u);
}

bool cTaskConsMatching::Execute(cJetTask* pParam)
{
	switch (nState())
	{
	case 1:
		if (AppearProc())
		{
			nState() = 2;
			nStep() = 0;
		}
		break;
	case 3:
		WaitProc();
		break;
	case 4:
		PointProc();
		break;
	case 5:
		if (DecideProc())
		{
			nState() = 6;
			nStep() = 0;
		}
		break;
	case 7:
		if (DeleteProc())
		{
			nState() = 8;
			nStep() = 0;
		}
		break;
	default:
		CustomKingdomUtil::Assert("error");
		break;
	case 0:
	case 2:
	case 6:
	case 8:
		break;
	}
	return cTaskAnita::Execute(pParam);
}

}
