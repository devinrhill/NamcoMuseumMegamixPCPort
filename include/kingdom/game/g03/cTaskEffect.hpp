#pragma once

#include "common.hpp"
#include "kingdom/cTaskPBPtkMan.hpp"
#include "ResNode.hpp"
#include "cJetTask.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g03
{

class cTaskEffect : public cTaskPBPtkMan
{
public:
	static constexpr int EMITTER_MAX = 64;

	static constexpr int GENERATOR_MAX = 512;

	static constexpr int PARTICLE_MAX = 4096;

private:
	std::vector<cKingdomPtkHnd*> m_vecPtkHnd;

public:
	cTaskEffect();

	cTaskEffect(ResNode* nodeEff);

	virtual ~cTaskEffect();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Regist();

	void Regist(ResNode* nodeEff);

private:
	uint GetDelay(ResNode* node);

	void RegistPreset();

protected:
	void ReleaseTerm() override { cTaskEffect::Term(); cTaskPBPtkMan::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
