#pragma once

#include <vector>

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"

// Grobda Remix (g11) collision framework, ported 1:1 from main.dol (no C# exists for Grobda).
//   cGrobdaCollisionObject (vtable 0x804490a0, ctor 0x80192174): a convex polygon on the XZ plane (up to 64 vertices, outward edge normals) with a radius,
//     two hit lists (contact hits +4, wall hits +0x10, records of 0x50 bytes) and the virtual interface the resolver calls: OnHit 0x801922f4 (+0xc),
//     OnWall 0x801926e8 (+0x10), ClearHits 0x80192adc (+0x14), GetPos (+0x18, the model's translation), GetOldPos (+0x20 -> this +0x960), GetRot
//     (+0x28, the model's rotation), GetMove (+0x30 -> this +0x950), Resolve (+0x38, 0x801936b0 trivial).
//   cGrobdaObject (vtable 0x80449110, ctor 0x80193968): the game object base: type id +0x938, flag word +0x93c, three words +0x940 / +0x944 / +0x948,
//     8 frame timers that freeze while the game is paused (0x80193b34).
//   cTaskGrobdaCollision (0x220 bytes, vtable 0x8038bc28): nine object lists (+0x1b4 + 0xc * k, elements {object, tag}) that the objects fill every frame
//     (Register 0x800307a0); Execute 0x800306d4 runs the resolver 0x80030b98 and empties the lists.
// Lists (k): 0 / 1 = the two kinds of movers (players / enemies), 2 = structures (walls), 3 = bombs / blasts (circles), 4 = pickup polygons, 5 / 6 = the two
// kinds of shots, 7 / 8 = overlap polygons.
namespace kingdom::game::g11
{
class cGrobdaCollisionObject;

// DOL record, 0x50 bytes: +0 object, +4 kind, +8 squared distance, +0x10 point, +0x20 normal, +0x30 push-out, +0x40 tag
struct sGrobdaHit
{
	cGrobdaCollisionObject* pObj{};
	int kind{};
	float dist{};
	float point[4]{};
	float normal[4]{};
	float push[4]{};
	void* tag{};   // the shooter's combo (the registering object's tag)
};

class cGrobdaCollisionObject
{
public:
	cGrobdaCollisionObject() = default;
	virtual ~cGrobdaCollisionObject() = default;

	virtual void OnHit(const sGrobdaHit& h) { m_vHit.push_back(h); }     // DOL 0x801922f4 (vtable +0xc)
	virtual void OnWall(const sGrobdaHit& h) { m_vWall.push_back(h); }   // DOL 0x801926e8 (+0x10)
	virtual void ClearHits() { m_vHit.clear(); m_vWall.clear(); }        // DOL 0x80192adc (+0x14)
	virtual float* GetPos() = 0;                                         // +0x18
	virtual float* GetOldPos() = 0;                                      // +0x20
	virtual float* GetRot() = 0;                                         // +0x28 (y = [1])
	virtual float* GetMove() = 0;                                        // +0x30
	virtual void Resolve(int a, int b) { (void)a; (void)b; }             // +0x38 (DOL 0x801936b0)

	// the polygon: local vertices / normals (+0x440 / +0x640) -> world (+0x30 / +0x230)
	void SetShape(const float (*v)[4], uint n, bool reversed);   // DOL 0x80192b08 (reversed = 0x80192f24: the vertices in reverse order)
	void UpdateShape();                                          // DOL 0x801933dc: local -> world after the object moved / turned

	std::vector<sGrobdaHit>& HitList() { return m_vHit; }         // DOL 0x80192af8 (+4)
	std::vector<sGrobdaHit>& WallList() { return m_vWall; }       // DOL 0x80192b00 (+0x10)

	float m_fRadius{};        // +0x24 the circle radius of the swept tests
	float m_fBroad{};         // +0x28 the broad phase radius (the shape's, or (radius + reach) * 1.5 for the movers, 0x80030ab0)
	float m_vPush[4]{};       // +0x850 the push-out of the wall resolution
	uint m_nMask{};           // +0x870 edges of a four-edge polygon the sweeps skip
	float m_fReach{};         // +0x874 (0x801922e4 / 0x801922ec)

	uint m_nWorld{};                 // +0x430 the world polygon (+0x30 vertices, +0x230 normals)
	float m_vWorld[64][4]{};
	float m_vWorldN[64][4]{};
	uint m_nLocal{};                 // +0x840 the local polygon (+0x440 vertices, +0x640 normals)
	float m_vLocal[64][4]{};
	float m_vLocalN[64][4]{};

private:
	std::vector<sGrobdaHit> m_vHit;    // +4
	std::vector<sGrobdaHit> m_vWall;   // +0x10
};

// one of the eight frame timers of a cGrobdaObject (DOL: u64 count at +8, enable byte at +0x10 of a 0x18 slot, 0x878 + 0x18 * k)
struct sGrobdaTimer
{
	unsigned long long count{};
	bool enable{};
};

class cGrobdaObject : public cGrobdaCollisionObject
{
public:
	cGrobdaObject() = default;

	void SetType(uint t) { m_nType = t; }                   // DOL 0x80193a70
	uint nType() const { return m_nType; }                  // 0x80193a68
	bool IsKind(uint t) const { return (m_nType & 0xff000000u) == (t & 0xff000000u); }   // 0x80193a78
	bool IsGroupA() const { return m_nType - 0x04000006u <= 4u; }                          // 0x80193b0c
	uint nFlags() const { return m_nFlags; }                // 0x80193a94
	void SetFlags(uint f) { m_nFlags = f; }                 // 0x80193a9c
	bool HasFlag(uint f) const { return (m_nFlags & f) != 0; }   // 0x80193aa4
	void AddFlag(uint f) { m_nFlags |= f; }                 // 0x80193abc
	void ClearFlag(uint f) { m_nFlags &= ~f; }              // 0x80193acc
	void UpdateTimers(bool paused);                         // DOL 0x80193b34
	void TimerReset(int k) { m_Timer[k].enable = false; m_Timer[k].count = 0; }                       // DOL 0x80193db8
	void TimerStart(int k) { if (m_Timer[k].enable) m_Timer[k].count = 0; m_Timer[k].enable = true; }   // DOL 0x80193ddc (restarts a running timer)
	void TimerStop(int k) { m_Timer[k].enable = false; }                                             // DOL 0x80193e08
	uint TimerMs(int k) const { return (uint)((int)(100000.0f * (float)m_Timer[k].count / 6000.0f)); }   // DOL 0x80193e1c (0x8000d80c: frames -> ms)
	bool TimerRunning(int k) const { return m_Timer[k].enable; }                                     // DOL 0x80193e64

	float* GetOldPos() override { return m_vOld; }
	float* GetMove() override { return m_vMove; }

	uint m_nType{};           // +0x938
	uint m_nFlags{};          // +0x93c
	int m_n940{}, m_n944{}, m_n948{};
	float m_vMove[4]{};       // +0x950
	float m_vOld[4]{};        // +0x960
	sGrobdaTimer m_Timer[8]{};
	bool m_bTimersFrozen{};   // +0x970
	bool m_bSavedEnable[8]{}; // +0x971
};

class cTaskGrobdaCollision : public kingdom::cTaskKingdom
{
public:
	enum { LAYERS = 9 };
	cTaskGrobdaCollision();                        // DOL 0x80030484
	bool Execute(cJetTask* pParam) override;       // DOL 0x800306d4
	void Register(int layer, cGrobdaCollisionObject* obj, void* tag = nullptr);   // DOL 0x800307a0

	// DOL 0x80033854: does the move `vec` of `obj` (from its old position) run into any object of list `layer`
	bool SweepAny(cGrobdaCollisionObject* obj, const float* vec, int layer);
	static cTaskGrobdaCollision* Instance() { return s_pInstance; }
	static bool Running();   // the gate every Grobda Execute starts with: app +0x1b4 == 0 and HOME (pad 0 hold & 0x400) not held

private:
	void Resolve();                                // DOL 0x80030b98
	struct sEntry { cGrobdaCollisionObject* pObj; void* tag; };
	bool Sweep(cGrobdaCollisionObject* a, const float* p, const float* d, cGrobdaCollisionObject* b, bool cull, bool record, float* outPt, float* outDist);   // 0x800333fc
	bool ResolveWalls(cGrobdaCollisionObject* a, float out[4]);   // 0x80033ce8
	std::vector<sEntry> m_vList[LAYERS];           // +0x1b4 + 0xc * k
	static cTaskGrobdaCollision* s_pInstance;
};

namespace grobda_geom
{
bool Overlap(float r1, float r2, const float* p1, const float* p2);              // DOL 0x80033c50 (XZ)
bool PointInPolygon(const float* p, const float (*v)[4], uint n);                  // DOL 0x80194808
bool SegIntersect(const float* a, const float* b, const float* c, const float* d);   // DOL 0x801942dc
bool SegHit(const float* a, const float* b, const float* c, const float* d, float out[4]);   // DOL 0x8019443c
float PointLineDist(const float* a, const float* b, const float* p);              // DOL 0x80194224
bool PolyOverlap(cGrobdaCollisionObject* a, cGrobdaCollisionObject* b);            // DOL 0x80033b28
bool CirclePoly(cGrobdaCollisionObject* c, cGrobdaCollisionObject* poly);          // DOL 0x80033904
}
}
