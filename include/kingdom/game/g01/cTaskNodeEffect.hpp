#pragma once

#include <functional>

#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cTaskKingdom.hpp"

// Galaga Remix (g01) an effect that follows a model node: kingdom::game::g01::cTaskEffectExpand (0x320 bytes, vtable 0x80439f18; a
// cKingdomPtkHnd at +0 and a task at +0x48). Made unbound (0x800e06dc), bound to a node of a model (0x800e0820, or later 0x800e0ecc), or bound
// and started (0x800e0a38); started by 0x800e0fe4 (slot, one-time, dependent, clipping). Init 0x800e0ca4: local SRT +0x210 / +0x220 / +0x230
// = 0 / 0 / 1. Execute 0x800e11fc (not paused): while the handle lives, the node's matrix (+0x250: 0x800e13d0 from the bind pose, or 0x800e17c0
// through the CHR result when the model has one) times the model's world matrix (+0x290), times the local SRT (+0x2d0) is the handle's world
// matrix (0x8008a324); once the handle is gone, +0x240 = 0. Term 0x800e0e44 frees its handle list.
// Natively the node comes from a callback: SetMtx gives the node's world matrix (DOL behaviour: rotation and scale follow the node); a
// position-only callback (the constructor's) is the older stand-in still used where no matrix is at hand.
namespace kingdom { class cKingdomModel; }

namespace kingdom::game::g01
{
class cTaskNodeEffect : public kingdom::cTaskKingdom
{
public:
	using PosFn = std::function<bool(float out[3])>;
	using MtxFn = std::function<bool(float out[16])>;   // the node's world matrix, column major
	explicit cTaskNodeEffect(PosFn fn);
	void Start(uint nSlot, bool bOnetime, bool bDepend, bool bClipping);   // DOL 0x800e0fe4
	void SetPos(PosFn fn) { m_Pos = std::move(fn); }                       // DOL 0x800e0ecc: bound to another node
	void SetMtx(MtxFn fn) { m_Mtx = std::move(fn); }                       // the same, with the node's full matrix
	// helpers: column-major products; a node matrix (cKingdomModel::NodeMatrix, 3x4 row major) as a column-major 4x4
	static void FromNode(const float n[12], float out[16]);
	static void Mul(const float a[16], const float b[16], float out[16]);
	// the world matrix of a node of a model: the model's world (CalcWorld) * the node's model-space matrix
	static bool ModelNodeMtx(kingdom::cKingdomModel* m, const char* node, float out[16]);
	void Stop(bool bKill = true);                                          // DOL 0x8008a508(handle, bKill) on the effect
	bool IsValid() { return m_bStarted && m_Hnd.IsValid(); }
	bool Pos(float out[3]) const { return m_Pos && m_Pos(out); }
	bool Execute(cJetTask* pParam) override;                              // DOL 0x800e11fc

private:
	PosFn m_Pos;
	MtxFn m_Mtx;
	kingdom::cKingdomPtkHnd m_Hnd;
	bool m_bStarted{};
	void ApplyMtx();
};
}
