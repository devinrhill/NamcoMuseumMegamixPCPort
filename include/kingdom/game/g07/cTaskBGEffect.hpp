#pragma once

#include "common.hpp"
#include "kingdom/cTaskPBPtkMan.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskScnRoot.hpp"

namespace kingdom::game::g07
{

class cTaskBGEffect : public cTaskPBPtkMan
{
private:
	static const string ptk_tbl[8];

	static constexpr int EMITTER_MAX = 16;

	static constexpr int GENERATOR_MAX = 32;

	static constexpr int PARTICLE_MAX = 512;

public:
	cTaskBGEffect();

	virtual ~cTaskBGEffect();

	virtual void DrawScnRootOpa(cTaskScnRoot* pTaskScnRoot);

	virtual void DrawScnRootXlu(cTaskScnRoot* pTaskScnRoot);

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
