#pragma once

#include <vector>

#include "common.hpp"

// Galaga demo (cut-scene) manager, owned by cG01GalagaSys (+0xc; made by cAppG01GalagaGame::Init, deleted by its Term). DOL vtable 0x80396400.
// The DOL keeps a vector of cDemoInterface pointers (+0 data, +4 count, +8 capacity) and a frame timer (+0xc). Owners (cTaskScene, cTaskBg,
// cTaskPacman) register their demo objects (cDemoSceneW1S1, cDemoBgW1S1, cDemoPacmanW1S1 ... created by the owners, 0x800f6354 /
// 0x800c89dc / 0x800f3eec). Natively the owner implements the interface itself: a demo object's state machine (0x801bce00: state 0 Init
// (+0x14), state 1 until its check (+8) is false, then done (+8 byte)) runs in the owner's Execute; Term (+0x18) restores the owner.
// The demo objects themselves are not separate classes natively (their logic runs on the owners). Their destructors: cDemoBgBase 0x800c8b1c,
// cDemoBgBoss 0x801bcfd4, cDemoBgW1S1 0x801bd034, cDemoBossBase 0x801bd23c, cDemoBossBoss 0x801bd544, cDemoGoalBase 0x801bd81c, cDemoGoalW1S3
// 0x801bd878, cDemoGoalW3S3 0x801be2f0, cDemoPacmanBase 0x800f4094, cDemoPacmanBoss 0x801c0410, cDemoPacmanW1S1 0x801c05bc, W1S3 0x801c055c,
// W2S3 0x801c04d0, W3S3 0x801c0470, cDemoSceneBase 0x800f64fc, cDemoSceneBoss 0x801c0950, W1S1 0x801c0aa4, W1S3 0x801c0a64, W2S3 0x801c0a08,
// W3S3 0x801c09ac, cDemoStartBase 0x801c0cfc, cDemoStartW1S1 0x801c0d58. The opening Inits: cDemoBgW1S1 0x801bcf28 (bg +0x7c), cDemoPacmanW1S1
// 0x801beeb4 (Pac-Man +0xc8), cDemoSceneW1S1 0x801c061c (scene +0x84). The End slot (+0x20: cDemoGoalW1S3 0x801bddcc -> goal +0x78,
// cDemoGoalW3S3 0x801be940) has no caller found.
namespace kingdom::game::g01
{
class cDemoInterface
{
public:
	virtual ~cDemoInterface() = default;
	virtual void DemoStart() = 0;              // the interface slot the manager's Start calls (owner state 0, e.g. cTaskScene 0x800d201c)
	virtual bool IsDemoDone() const = 0;       // the demo object's +8 byte
	virtual void DemoFinish() = 0;             // the demo object's Term (+0x18)
	// the goal demos (stages 3 / 6 / 9): the manager's StartGoal calls the interface's +0x10 (owner state 1), whose Execute runs the demo
	// object's state-1 path (0x801bce94: +0x1c init, then until the +0xc check fails; done = +9). Owners without a goal demo are done at once.
	virtual void DemoGoalStart() {}
	virtual bool IsGoalDone() const { return true; }
};

class cDemoMan
{
public:
	cDemoMan() = default;                      // DOL 0x801b9828: all fields 0 (+0xc = 0.0f)
	virtual ~cDemoMan();                       // DOL 0x800b0650
	void Regist(cDemoInterface* p);            // DOL 0x801be964
	bool IsEmpty() const { return m_Demo.empty(); }   // DOL 0x801bee38
	void Start();                              // DOL 0x801becd8: every demo starts, timer 0
	void Reset() { m_fTimer = 0.0f; }          // DOL 0x801beccc
	void Tick() { m_fTimer += 1.0f; }          // DOL 0x801becb8
	bool IsAllDone() const;                    // DOL 0x801bedb8
	bool IsSkip() const;                       // DOL 0x801bec4c: after 60 frames (FLOAT_8047cec8) any pad's A / B / Z (0x860) skips
	void Finish();                             // DOL 0x801bee48: every demo's Term
	void StartGoal();                          // DOL 0x801bed48: every owner's goal demo starts (+0x10), timer 0
	bool IsAllGoalDone() const;                // DOL 0x801bedf8: every demo object's +9

private:
	std::vector<cDemoInterface*> m_Demo;       // +0x00 .. +0x08
	float m_fTimer{};                          // +0x0c
};
}
