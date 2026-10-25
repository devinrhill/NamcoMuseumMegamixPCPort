#pragma once

#include "common.hpp"
#include "cJetTask.hpp"

class cSysTaskUser : public cJetTask
{
public:
	cSysTaskUser();

	virtual ~cSysTaskUser();

	uint nDrawPriority() override;

	void nDrawPriority(uint n) override;
};
