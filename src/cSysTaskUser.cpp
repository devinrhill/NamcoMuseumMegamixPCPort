#include "cSysTaskUser.hpp"

cSysTaskUser::cSysTaskUser()
{
	nDrawPriority(65536u);
}

cSysTaskUser::~cSysTaskUser()
{
}

uint cSysTaskUser::nDrawPriority()
{
	return cJetTask::nDrawPriority();
}

void cSysTaskUser::nDrawPriority(uint n)
{
	cJetTask::nDrawPriority(n);
}
