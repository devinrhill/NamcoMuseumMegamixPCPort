#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "kingdom/game/g11/cGrobdaCollision.hpp"

// Grobda Remix (g11) the model task every game object derives from: kingdom::game::g11::cTaskGrobdaModel (0xb50 bytes, vtable 0x8038c020; Init 0x80036a90,
// Execute 0x80036b00, Draw 0x80036b60 = cTaskModel's). Two bases in the DOL: cGrobdaObject (collision object, timers) and the cTaskModel sub-object at
// +0x97c; +0xb48 = the index of the model slot whose translation / rotation are the object's position / rotation (vtable +0x18 / +0x28 = FUN_80027770
// / FUN_80028084 -> the model pointer / its +0x10), +0x950 the movement vector and +0x960 the old position (vtable +0x30 / +0x20).
namespace kingdom::game::g11
{
class cTaskGrobdaModel : public kingdom::cTaskModel, public cGrobdaObject
{
public:
	cTaskGrobdaModel();                                   // DOL 0x80036a90 (name "cTaskGrobdaModel" at +0x980, +0xb48 = 0)
	bool Execute(cJetTask* pParam) override;              // DOL 0x80036b00: the object timers, then cTaskModel::Execute (the pause gate is the timers')

	float* GetPos() override;                             // the model's translation
	float* GetRot() override;                             // the model's rotation (radians, y = [1])

	// the object's position as the model holds it (the DOL objects keep it there)
	void SetPos(float x, float y, float z);
	int m_nModelIdx{0};                                   // +0xb48
};
}
