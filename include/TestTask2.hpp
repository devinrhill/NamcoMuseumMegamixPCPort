#pragma once

#include "common.hpp"
#include "cJetTask.hpp"

class TestTask2 : public cJetTask
{
public:
	TestTask2();

private:
	void Init();

public:
	bool Execute(cJetTask* pParam) override;
};
