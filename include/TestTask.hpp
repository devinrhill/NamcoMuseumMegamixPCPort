#pragma once

#include "common.hpp"
#include "cJetTask.hpp"

class TestTask : public cJetTask
{
public:
	TestTask();

private:
	void Init();

public:
	bool Execute(cJetTask* pParam) override;
};
