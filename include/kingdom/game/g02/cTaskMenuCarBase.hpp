#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"

// Rally-X Remix (g02) stage select 3D: Pac-Man's car on the world map. Ported from main.dol (no C# exists for Rally-X).
// cTaskMenuCarBase (DOL vtable 0x8043e658, parent kingdom::cTaskModel; dtor 0x80110c54; its ctor is inlined in the subclasses) and
// cTaskMenuPacman (vtable 0x8043e5c8, 0x220 bytes, ctor 0x80110bac). The multiplayer subclass cTaskMultiMenuCar (0x8043e530) is NOT PORTED.
namespace kingdom::game::g02
{

// the transform of node `node` of model resource `model` (a placement "set" model): ResNodeData translate (FUN_8001dd54), rotate in degrees
// (FUN_8001dbd4) and scale (FUN_8001da54). False (and the vectors untouched) when the model / node is missing.
bool MenuSetNode(const char* model, const char* node, Vector4& trans, Vector4& rot, Vector4& scale);

class cTaskMenuCarBase : public kingdom::cTaskModel
{
public:
	cTaskMenuCarBase();               // inlined in 0x80110bac / 0x80111050
	~cTaskMenuCarBase() override;     // DOL 0x80110c54

protected:
	int m_nState{};                   // +0x1cc (1 appear, 2 appeared)
	int m_nStep{};                    // +0x1d0
	Vector4 m_vTrans{0.0f, 0.0f, 0.0f, 1.0f};   // +0x1e0
	Vector4 m_vRotate{0.0f, 0.0f, 0.0f, 0.0f};  // +0x1f0 (radians after Init)
	Vector4 m_vScale{1.0f, 1.0f, 1.0f, 1.0f};   // +0x200 (FUN_8013e968 = ONE)
};

class cTaskMenuPacman : public cTaskMenuCarBase
{
public:
	cTaskMenuPacman();                // DOL 0x80110bac (priority 0x10004)
	~cTaskMenuPacman() override;      // DOL 0x80111148

	virtual void Init();              // DOL 0x80110cac (vtable +0x44): the models, placed at node "set_pac" of MENU_NULL_MODEL, scale 0
	virtual void AppearState();       // DOL 0x80110b74 (+0x48)
	virtual bool IsAppearEnd();       // DOL 0x80110b88 (+0x4c)
	virtual void Reset();             // DOL 0x80110b9c (+0x50)
	bool Execute(cJetTask* pParam) override;   // DOL 0x80110ef4

private:
	float m_fScale{};                 // +0x210 0 -> 1 while appearing
};

}
