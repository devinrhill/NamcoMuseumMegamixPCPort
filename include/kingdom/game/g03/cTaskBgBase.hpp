#pragma once

#include "common.hpp"
#include "kingdom/cTaskPBModel.hpp"
#include "ResNode.hpp"
#include "kingdom/cKingdomModel.hpp"

namespace kingdom::game::g03
{

class cTaskBgBase : public cTaskPBModel
{
public:
	cTaskBgBase();

	cTaskBgBase(ResNode* const& node0, ResNode* const& node1);

private:
	void Init();

public:
	void Term() override;

protected:
	void Regist(ResNode* const& node0, ResNode* const& node1);

public:
	IEnumerator* coRegist(ResNode* node0, ResNode* node1);

protected:
	void SetUserDataAnm(cKingdomModel* model, ResNode* const& node);

public:
	void SetMaterialColor(Color color);

	void SetMaterialAlpha(byte alpha);

protected:
	void ReleaseTerm() override { cTaskBgBase::Term(); cTaskPushBack::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
