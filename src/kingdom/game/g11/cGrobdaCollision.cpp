#include "kingdom/game/g11/cGrobdaCollision.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom::game::g11
{

cTaskGrobdaCollision* cTaskGrobdaCollision::s_pInstance = nullptr;

namespace
{
constexpr float TINY = FLT_MIN;   // FLOAT_8047c9ec / FLOAT_8047c9b4 = 0x00800000

void Copy4(float* d, const float* s) { if (d != s) std::memcpy(d, s, 16); }   // FUN_80032fec
float Cross(const float* u, const float* v) { return u[0] * v[2] - u[2] * v[0]; }
void Sub(const float* a, const float* b, float* o) { o[0] = a[0] - b[0]; o[1] = a[1] - b[1]; o[2] = a[2] - b[2]; }
float Dot3(const float* a, const float* b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
float Dot2(const float* a, const float* b) { return a[0] * b[0] + a[2] * b[2]; }   // FUN_80194208

// MTXRotRad(.., 'Y', a) applied to a vector (the same convention as cTaskGrobdaPlayer's RotY)
void RotY(float* v, float a)
{
	const float c = std::cos(a), s = std::sin(a);
	const float x = v[0] * c + v[2] * s, z = -v[0] * s + v[2] * c;
	v[0] = x;
	v[2] = z;
}
}

// ---- geometry -------------------------------------------------------------------------------------------------------------------------

namespace grobda_geom
{

// DOL 0x80033c50 [audited]: squared distance on the XZ plane <= (r1 + r2)^2
bool Overlap(float r1, float r2, const float* p1, const float* p2)
{
	float d[3];
	Sub(p2, p1, d);
	d[1] = 0.0f;
	return Dot3(d, d) <= (r1 + r2) * (r1 + r2);
}

// DOL 0x80194808 [audited]: the signed areas of the triangles (p, v_i, v_i+1) must not have both signs
bool PointInPolygon(const float* p, const float (*v)[4], uint n)
{
	if (n < 3) return false;
	std::vector<float> c(n);
	for (uint i = 0; i < n; i++)
	{
		float a[3], b[3];
		Sub(v[i], p, a);
		Sub(v[(i + 1) % n], p, b);
		c[i] = a[0] * b[2] - a[2] * b[0];
	}
	for (uint i = 0; i < n; i++)
		for (uint j = i + 1; j < n; j++)
			if ((c[i] < 0.0f || c[j] < 0.0f) && (0.0f < c[i] || 0.0f < c[j])) return false;
	return true;
}

// DOL 0x801942dc [audited]: do the segments a-b and c-d cross (XZ)
bool SegIntersect(const float* a, const float* b, const float* c, const float* d)
{
	float ab[3], ac[3], ad[3], cd[3], ca[3], cb[3];
	Sub(b, a, ab);
	Sub(c, a, ac);
	Sub(d, a, ad);
	Sub(d, c, cd);
	Sub(a, c, ca);
	Sub(b, c, cb);
	return Cross(ab, ac) * Cross(ab, ad) < TINY && Cross(cd, ca) * Cross(cd, cb) < TINY;
}

// DOL 0x8019443c [audited]: the crossing point of a-b with c-d (not for parallel segments)
bool SegHit(const float* a, const float* b, const float* c, const float* d, float out[4])
{
	float d1[3], d2[3];
	Sub(a, b, d1);
	Sub(c, d, d2);
	if (std::fabs(Cross(d1, d2)) < TINY) return false;
	if (!SegIntersect(a, b, c, d)) return false;
	float cd[3], ca[3], cb[3], ab[3];
	Sub(d, c, cd);
	Sub(a, c, ca);
	Sub(b, c, cb);
	const float s5 = std::fabs(Cross(cd, cb));
	const float s4 = std::fabs(Cross(cd, ca));
	const float t = s4 / (s4 + s5);
	Sub(b, a, ab);
	out[0] = a[0] + ab[0] * t;
	out[1] = a[1] + ab[1] * t;
	out[2] = a[2] + ab[2] * t;
	return true;
}

// DOL 0x80194224 [audited]: distance of p from the line a-b (XZ), 0 for a degenerate line
float PointLineDist(const float* a, const float* b, const float* p)
{
	float u[3], w[3];
	Sub(b, a, u);
	Sub(p, a, w);
	const float len = std::sqrt(Dot3(u, u));
	if (TINY < std::fabs(len)) return std::fabs(u[0] * w[2] - u[2] * w[0]) / std::fabs(len);
	return 0.0f;
}

// DOL 0x80033b28 [audited]: two polygons overlap: a centre inside the other polygon or a vertex inside the other polygon
bool PolyOverlap(cGrobdaCollisionObject* a, cGrobdaCollisionObject* b)
{
	if (PointInPolygon(a->GetPos(), b->m_vWorld, b->m_nWorld)) return true;
	if (PointInPolygon(b->GetPos(), a->m_vWorld, a->m_nWorld)) return true;
	for (uint i = 0; i < a->m_nWorld; i++)
		if (PointInPolygon(a->m_vWorld[i], b->m_vWorld, b->m_nWorld)) return true;
	for (uint i = 0; i < b->m_nWorld; i++)
		if (PointInPolygon(b->m_vWorld[i], a->m_vWorld, a->m_nWorld)) return true;
	return false;
}

// DOL 0x80033904 [audited]: the circle of c (radius +0x24) against the polygon of `poly`: a vertex inside the circle, an edge crossing the segment from the centre
// along the inward normal, or the centre inside
bool CirclePoly(cGrobdaCollisionObject* c, cGrobdaCollisionObject* poly)
{
	const uint n = poly->m_nWorld;
	const float r = c->m_fRadius;
	const float* pos = c->GetPos();
	for (uint i = 0; i < n; i++)
	{
		float d[3];
		Sub(pos, poly->m_vWorld[i], d);
		d[1] = 0.0f;
		if (Dot3(d, d) <= r * r) return true;
	}
	for (uint i = 0; i < n; i++)
	{
		const float* nrm = poly->m_vWorldN[i];
		const float off[3] = {nrm[0] * r, nrm[1] * r, nrm[2] * r};
		float q[3];
		Sub(pos, off, q);
		if (SegIntersect(poly->m_vWorld[i], poly->m_vWorld[(i + 1) % n], pos, q)) return true;
	}
	return PointInPolygon(pos, poly->m_vWorld, n);
}

}

// ---- cGrobdaCollisionObject -----------------------------------------------------------------------------------------------------------

// DOL 0x80192b08 [audited] / 0x80192f24: the local polygon, its outward edge normals (-ez, 0, ex), the bounding radius, then the world polygon
void cGrobdaCollisionObject::SetShape(const float (*v)[4], uint n, bool reversed)
{
	if (n > 64) n = 64;
	float maxDot = 0.0f;
	for (uint i = 0; i < n; i++)
	{
		const float* s = reversed ? v[n - 1 - i] : v[i];
		Copy4(m_vLocal[i], s);
		const float d = Dot3(m_vLocal[i], m_vLocal[i]);
		if (i == 0 || maxDot < d) maxDot = d;
	}
	m_nLocal = n;
	for (uint i = 0; i < n; i++)
	{
		float e[3];
		Sub(m_vLocal[(i + 1) % n], m_vLocal[i], e);
		const float l = std::sqrt(Dot3(e, e));
		if (l > 0.0f) { e[0] /= l; e[1] /= l; e[2] /= l; }   // PSVECNormalize
		m_vLocalN[i][0] = -e[2];
		m_vLocalN[i][1] = 0.0f;
		m_vLocalN[i][2] = e[0];
		m_vLocalN[i][3] = 0.0f;
	}
	UpdateShape();
	m_fBroad = maxDot < TINY ? 0.5f : std::sqrt(maxDot);   // FLOAT_8047c9b8
}

// DOL 0x801933dc [audited]
void cGrobdaCollisionObject::UpdateShape()
{
	const float* rot = GetRot();
	const float* pos = GetPos();
	for (uint i = 0; i < m_nLocal; i++)
	{
		float v[4], nrm[4];
		Copy4(v, m_vLocal[i]);
		Copy4(nrm, m_vLocalN[i]);
		if (rot[1] != 0.0f)
		{
			RotY(v, rot[1]);
			RotY(nrm, rot[1]);
		}
		v[0] += pos[0];
		v[1] += pos[1];
		v[2] += pos[2];
		Copy4(m_vWorld[i], v);
		Copy4(m_vWorldN[i], nrm);
	}
	m_nWorld = m_nLocal;
}

// ---- cGrobdaObject --------------------------------------------------------------------------------------------------------------------

// DOL 0x80193b34 [audited]: the timers count while the game runs; on the first paused frame their enable flags are saved and cleared, on resume they are
// restored (a timer that was started meanwhile restarts from 0)
void cGrobdaObject::UpdateTimers(bool paused)
{
	if (!paused)
	{
		if (m_bTimersFrozen)
		{
			m_bTimersFrozen = false;
			for (int i = 0; i < 8; i++)
				if (m_bSavedEnable[i])
				{
					if (m_Timer[i].enable) m_Timer[i].count = 0;
					m_Timer[i].enable = true;
				}
		}
		for (sGrobdaTimer& t : m_Timer)
			if (t.enable) t.count++;
	}
	else if (!m_bTimersFrozen)
	{
		m_bTimersFrozen = true;
		for (int i = 0; i < 8; i++)
		{
			m_bSavedEnable[i] = m_Timer[i].enable;
			if (m_Timer[i].enable) m_Timer[i].enable = false;
		}
	}
}

// ---- cTaskGrobdaCollision -------------------------------------------------------------------------------------------------------------

bool cTaskGrobdaCollision::Running()
{
	kingdom::cAppKingdom* app = KingdomGlobal::KingSys->pAppMain();
	return !(app != nullptr && app->IsPause()) && (GamePad::Get(0).hold & 0x400u) == 0;
}

// DOL 0x80030484 [audited]
cTaskGrobdaCollision::cTaskGrobdaCollision()
{
	SetName("cTaskGrobdaCollision");
	s_pInstance = this;
}

// DOL 0x800307a0 [audited]
void cTaskGrobdaCollision::Register(int layer, cGrobdaCollisionObject* obj, void* tag)
{
	m_vList[layer].push_back({obj, tag});
}

// DOL 0x800306d4 [audited]
bool cTaskGrobdaCollision::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (Running())
	{
		Resolve();
		for (auto& l : m_vList) l.clear();   // the nine counts (+0x1b8 ...) = 0
	}
	return true;
}

// DOL 0x80033854 [audited]
bool cTaskGrobdaCollision::SweepAny(cGrobdaCollisionObject* obj, const float* vec, int layer)
{
	for (const sEntry& e : m_vList[layer])
		if (Sweep(obj, obj->GetOldPos(), vec, e.pObj, true, false, nullptr, nullptr)) return true;
	return false;
}

// DOL 0x800333fc [audited]. The circle of `a` (radius +0x24) moves from p along d; each edge of b's polygon (skipped by the mask on four-edge polygons) is widened
// by the radius at both ends, the leading point is p - r * normal, started 1.5 r behind along the move; a crossing gives a hit. cull: edges the move
// leaves are ignored; record: a wall record goes to a (OnWall); the nearest crossing point / squared distance are returned.
bool cTaskGrobdaCollision::Sweep(cGrobdaCollisionObject* a, const float* p, const float* d, cGrobdaCollisionObject* b, bool cull, bool record, float* outPt, float* outDist)
{
	using namespace grobda_geom;
	const uint n = b->m_nWorld;
	const float r = a->m_fRadius;
	uint count = 0;
	float best = 0.0f;
	float bestPt[4] = {};
	for (uint i = 0; i < n; i++)
	{
		if (n == 4 && ((1u << i) & b->m_nMask) != 0) continue;
		float v0[4], v1[4], nrm[4];
		Copy4(v0, b->m_vWorld[i]);
		Copy4(v1, b->m_vWorld[(i + 1) % n]);
		Copy4(nrm, b->m_vWorldN[i]);
		const float off[3] = {nrm[2] * r, 0.0f, -nrm[0] * r};   // the tangent * radius
		v0[0] -= off[0]; v0[1] -= off[1]; v0[2] -= off[2];
		v1[0] += off[0]; v1[1] += off[1]; v1[2] += off[2];
		if (cull && Dot2(d, nrm) >= 0.0f) continue;
		float s0[4], e0[4], st[4];
		for (int k = 0; k < 3; k++) s0[k] = p[k] - r * nrm[k];
		s0[3] = 0.0f;
		for (int k = 0; k < 3; k++) e0[k] = s0[k] + d[k];
		e0[3] = 0.0f;
		if (Dot3(d, d) >= 0x1p-125f)   // FUN_80194194: |d|^2 >= FLOAT_8047c9e8 (0x01000000)
		{
			float dn[3] = {d[0], d[1], d[2]};
			const float l = std::sqrt(Dot3(dn, dn));
			if (l > 0.0f) { dn[0] /= l; dn[1] /= l; dn[2] /= l; }
			const float k = 1.5f * -r;
			for (int c = 0; c < 3; c++) st[c] = s0[c] + dn[c] * k;
		}
		else
			for (int c = 0; c < 3; c++) st[c] = s0[c];
		st[3] = 0.0f;
		float hit[4] = {};
		if (!SegHit(st, e0, v0, v1, hit)) continue;
		if (record || outPt != nullptr || outDist != nullptr)
		{
			float dv[3];
			Sub(hit, s0, dv);
			dv[1] = 0.0f;
			const float dist2 = Dot3(dv, dv);
			if (record)
			{
				sGrobdaHit h;
				h.pObj = b;
				Copy4(h.normal, nrm);
				Copy4(h.point, hit);
				h.dist = dist2;
				const float pd = PointLineDist(v0, v1, e0);
				h.push[0] = nrm[0] * pd;
				h.push[1] = 0.0f;
				h.push[2] = nrm[2] * pd;
				h.push[3] = 0.0f;
				a->OnWall(h);
			}
			if ((outPt != nullptr || outDist != nullptr) && (count == 0 || dist2 < best))
			{
				best = dist2;
				Copy4(bestPt, hit);
			}
		}
		count++;
	}
	if (count != 0)
	{
		if (outPt != nullptr) Copy4(outPt, bestPt);
		if (outDist != nullptr) *outDist = best;
	}
	return count != 0;
}

// DOL 0x80033ce8 [audited]: the wall records of a combine into one push-out; false when walls and mover contacts are mixed, the pushes pull apart or the pushed
// position is occupied
bool cTaskGrobdaCollision::ResolveWalls(cGrobdaCollisionObject* a, float out[4])
{
	std::vector<sGrobdaHit>& w = a->WallList();
	bool k2 = false, k01 = false;
	for (const sGrobdaHit& h : w)
	{
		if (h.kind == 2) k2 = true;
		else if (h.kind == 0 || h.kind == 1) k01 = true;
	}
	if (k2 && k01) return false;
	if (w.empty()) return false;
	for (size_t i = 0; i < w.size(); i++)   // the same direction twice: the later one goes
		for (size_t j = i + 1; j < w.size();)
		{
			if (Dot3(w[i].normal, w[j].normal) > 0.98f)   // FLOAT_8047a008
				w.erase(w.begin() + (long)j);
			else
				j++;
		}
	bool ok = true;
	for (size_t i = 0; i < w.size() && ok; i++)
		for (size_t j = i + 1; j < w.size(); j++)
			if (!(0.0f <= Dot3(w[i].normal, w[j].normal))) { ok = false; break; }
	if (!ok) return false;
	float sum[4] = {};
	for (const sGrobdaHit& h : w)
	{
		sum[0] += h.push[0]; sum[1] += h.push[1]; sum[2] += h.push[2];
	}
	const float* mv = a->GetMove();
	const float mvLen = std::sqrt(Dot3(mv, mv));
	const float sumLen = std::sqrt(Dot3(sum, sum));
	if (mvLen < sumLen)
	{
		sum[0] /= sumLen; sum[1] /= sumLen; sum[2] /= sumLen;
		sum[0] *= mvLen; sum[1] *= mvLen; sum[2] *= mvLen;
	}
	const float* pos = a->GetPos();
	const float np[4] = {pos[0] + sum[0], pos[1] + sum[1], pos[2] + sum[2], 0.0f};
	bool blocked = false;
	for (int k : {1, 0})   // the lists +0x1c0 then +0x1b4
	{
		for (const sEntry& e : m_vList[k])
		{
			if (e.pObj == a) continue;
			const float rr = e.pObj->m_fRadius + a->m_fRadius;
			float dv[3];
			Sub(e.pObj->GetPos(), np, dv);
			dv[1] = 0.0f;
			if (Dot3(dv, dv) <= rr * rr) { blocked = true; break; }
		}
		if (blocked) break;
	}
	if (!blocked)
		for (const sEntry& e : m_vList[2])
			if (Sweep(a, a->GetPos(), sum, e.pObj, true, false, nullptr, nullptr)) { blocked = true; break; }
	if (blocked) return false;
	Copy4(out, sum);
	return true;
}

// DOL 0x80030b98 [audited]: the passes, in the DOL's order
void cTaskGrobdaCollision::Resolve()
{
	using namespace grobda_geom;
	auto& L = m_vList;
	// 0x80030ab0 for the lists 0 / 1 / 5 / 6: the broad radius (radius + reach) * 1.5
	for (int k : {0, 1, 5, 6})
		for (sEntry& e : L[k]) e.pObj->m_fBroad = (e.pObj->m_fRadius + e.pObj->m_fReach) * 1.5f;

	// movers that run into each other are stopped
	std::vector<cGrobdaCollisionObject*> stop;
	for (sEntry& ea : L[0])
		for (sEntry& eb : L[1])
		{
			cGrobdaCollisionObject *a = ea.pObj, *b = eb.pObj;
			if (!Overlap(a->m_fBroad, b->m_fBroad, a->GetOldPos(), b->GetOldPos())) continue;
			if (!Sweep(a, a->GetOldPos(), a->GetMove(), b, true, false, nullptr, nullptr))
			{
				if (Sweep(b, b->GetOldPos(), b->GetMove(), a, true, false, nullptr, nullptr)) stop.push_back(b);
			}
			else if (Dot3(b->GetMove(), a->GetMove()) < 0.0f)
				stop.push_back(b);
		}
	for (size_t i = 0; i < L[1].size(); i++)
		for (size_t j = 0; j < L[1].size(); j++)
		{
			if (i == j) continue;
			cGrobdaCollisionObject *a = L[1][i].pObj, *b = L[1][j].pObj;
			if (!Overlap(a->m_fBroad, b->m_fBroad, a->GetOldPos(), b->GetOldPos())) continue;
			if (Sweep(a, a->GetOldPos(), a->GetMove(), b, true, false, nullptr, nullptr)) stop.push_back(a);
			if (Sweep(b, b->GetOldPos(), b->GetMove(), a, true, false, nullptr, nullptr)) stop.push_back(b);
		}
	for (cGrobdaCollisionObject* o : stop)
	{
		Copy4(o->GetPos(), o->GetOldPos());
		std::memset(o->GetMove(), 0, 16);   // FUN_80033000
	}

	// the contact records
	auto contact = [&](cGrobdaCollisionObject* a, cGrobdaCollisionObject* b, int kindOnA)
	{
		sGrobdaHit h;
		h.pObj = b;
		h.kind = kindOnA;
		a->OnHit(h);
	};
	for (sEntry& ea : L[0])
		for (sEntry& eb : L[1])
		{
			cGrobdaCollisionObject *a = ea.pObj, *b = eb.pObj;
			if (!Overlap(a->m_fBroad, b->m_fBroad, a->GetOldPos(), b->GetOldPos())) continue;
			if (Sweep(a, a->GetOldPos(), a->GetMove(), b, true, true, nullptr, nullptr)) contact(a, b, 1);
			if (Sweep(b, b->GetOldPos(), b->GetMove(), a, true, true, nullptr, nullptr)) contact(b, a, 0);
		}
	for (int k : {0, 1})
		for (size_t i = 0; i < L[k].size(); i++)
			for (size_t j = i + 1; j < L[k].size(); j++)
			{
				cGrobdaCollisionObject *a = L[k][i].pObj, *b = L[k][j].pObj;
				if (!Overlap(a->m_fBroad, b->m_fBroad, a->GetOldPos(), b->GetOldPos())) continue;
				if (Sweep(a, a->GetOldPos(), a->GetMove(), b, true, true, nullptr, nullptr)) contact(a, b, k);
				if (Sweep(b, b->GetOldPos(), b->GetMove(), a, true, true, nullptr, nullptr)) contact(b, a, k);
			}
	for (int k : {0, 1})   // against the structures: kind 2
		for (sEntry& ea : L[k])
			for (sEntry& eb : L[2])
			{
				cGrobdaCollisionObject *a = ea.pObj, *b = eb.pObj;
				if (!Overlap(a->m_fBroad, b->m_fBroad, a->GetOldPos(), b->GetOldPos())) continue;
				if (Sweep(a, a->GetOldPos(), a->GetMove(), b, true, true, nullptr, nullptr)) contact(a, b, 2);
			}

	// the wall records: push out, or go back to the old position
	std::vector<cGrobdaCollisionObject*> pushed, back;
	for (int k : {0, 1})
		for (sEntry& e : L[k])
		{
			cGrobdaCollisionObject* a = e.pObj;
			if (a->WallList().empty()) continue;
			float out[4] = {};
			if (ResolveWalls(a, out))
			{
				Copy4(a->m_vPush, out);
				pushed.push_back(a);
			}
			else
				back.push_back(a);
		}
	for (cGrobdaCollisionObject* a : pushed)
	{
		float* pos = a->GetPos();
		pos[0] += a->m_vPush[0]; pos[1] += a->m_vPush[1]; pos[2] += a->m_vPush[2];
		a->UpdateShape();
		std::memset(a->m_vPush, 0, sizeof(a->m_vPush));
	}
	for (cGrobdaCollisionObject* a : back) Copy4(a->GetPos(), a->GetOldPos());

	// overlap polygons: kind 8
	auto overlapKind8 = [&](int polyList, int moverList, bool sphereFirst)
	{
		for (sEntry& ep : L[polyList])
			for (sEntry& em : L[moverList])
			{
				cGrobdaCollisionObject *p = ep.pObj, *m = em.pObj;
				if (p == m) continue;
				if (sphereFirst && !Overlap(p->m_fBroad, m->m_fBroad, p->GetPos(), m->GetPos())) continue;
				if (!PolyOverlap(p, m)) continue;
				sGrobdaHit h;
				h.pObj = p;
				h.kind = 8;
				float dv[3];
				Sub(p->GetPos(), m->GetPos(), dv);
				dv[1] = 0.0f;
				h.dist = Dot3(dv, dv);
				m->OnHit(h);
			}
	};
	overlapKind8(8, 0, false);
	overlapKind8(7, 0, true);
	overlapKind8(7, 1, true);

	// shots against movers / structures
	auto shot = [&](int shotList, int targetList, bool excludeOwn, int kindOnShot, int kindOnTarget)
	{
		for (sEntry& es : L[shotList])
			for (sEntry& et : L[targetList])
			{
				cGrobdaCollisionObject *s = es.pObj, *t = et.pObj;
				if (!Overlap(s->m_fBroad, t->m_fBroad, s->GetOldPos(), t->GetPos())) continue;
				if (excludeOwn)   // a shot of player n does not hit player n: types 0x0100000n vs 0x1000000n
				{
					const uint ts = static_cast<cGrobdaObject*>(s)->nType(), tt = static_cast<cGrobdaObject*>(t)->nType();
					if ((ts == 0x1000001u && tt == 0x10000001u) || (ts == 0x1000002u && tt == 0x10000002u) || (ts == 0x1000003u && tt == 0x10000003u) ||
					    (ts == 0x1000004u && tt == 0x10000004u))
						continue;
				}
				float pt[4] = {}, dist = 0.0f;
				bool hit = false;
				if (!PointInPolygon(s->GetPos(), t->m_vWorld, t->m_nWorld))
					hit = Sweep(s, s->GetOldPos(), s->GetMove(), t, true, false, pt, &dist);
				else
				{
					hit = true;
					Copy4(pt, s->GetPos());
					dist = Dot3(s->GetMove(), s->GetMove());
				}
				if (!hit) continue;
				sGrobdaHit h1;
				h1.pObj = t;
				h1.kind = kindOnShot;
				h1.dist = dist;
				Copy4(h1.point, pt);
				s->OnHit(h1);
				sGrobdaHit h2;
				h2.pObj = shotList == 6 ? t : s;   // kind 6: the DOL records the target itself
				h2.kind = kindOnTarget;
				h2.dist = dist;
				t->OnHit(h2);
			}
	};
	shot(5, 1, false, 1, 5);
	shot(6, 0, false, 0, 6);
	shot(5, 0, true, 0, 5);

	// bombs: circles against polygons
	for (sEntry& eb : L[3])
		for (sEntry& em : L[1])
		{
			cGrobdaCollisionObject *b = eb.pObj, *m = em.pObj;
			const bool groupA = static_cast<cGrobdaObject*>(b)->IsGroupA();
			if (!(groupA || static_cast<cGrobdaObject*>(m)->nType() != 0x20000006u)) continue;
			if (!CirclePoly(b, m)) continue;
			sGrobdaHit h1;
			h1.pObj = m;
			h1.kind = 1;
			b->OnHit(h1);
			sGrobdaHit h2;
			h2.pObj = b;
			h2.kind = 3;
			h2.tag = eb.tag;
			float dv[3];
			Sub(b->GetPos(), m->GetPos(), dv);
			dv[1] = 0.0f;
			h2.dist = Dot3(dv, dv);
			m->OnHit(h2);
		}
	for (sEntry& eb : L[3])
		for (sEntry& em : L[0])
		{
			cGrobdaCollisionObject *b = eb.pObj, *m = em.pObj;
			if (!CirclePoly(b, m)) continue;
			sGrobdaHit h1;
			h1.pObj = m;
			h1.kind = 0;
			b->OnHit(h1);
			sGrobdaHit h2;
			h2.pObj = b;
			h2.kind = 3;
			h2.tag = eb.tag;
			float dv[3];
			Sub(b->GetPos(), m->GetPos(), dv);
			dv[1] = 0.0f;
			h2.dist = Dot3(dv, dv);
			m->OnHit(h2);
		}
	// shots against the structures: kind 2 (the type 0x30000003 ones are hit from inside)
	for (int shotList : {5, 6})
		for (sEntry& es : L[shotList])
			for (sEntry& et : L[2])
			{
				cGrobdaCollisionObject *s = es.pObj, *t = et.pObj;
				if (!Overlap(s->m_fBroad, t->m_fBroad, s->GetOldPos(), t->GetPos())) continue;
				const bool inside = PointInPolygon(s->GetPos(), t->m_vWorld, t->m_nWorld);
				const bool t3 = static_cast<cGrobdaObject*>(t)->nType() == 0x30000003u;
				float pt[4] = {}, dist = 0.0f;
				bool hit = false;
				if ((t3 || !inside) && (!t3 || inside))
					hit = Sweep(s, s->GetOldPos(), s->GetMove(), t, true, false, pt, &dist);
				else
				{
					hit = true;
					Copy4(pt, s->GetPos());
					dist = Dot3(s->GetMove(), s->GetMove());
				}
				if (!hit) continue;
				sGrobdaHit h;
				h.pObj = t;
				h.kind = 2;
				h.dist = dist;
				Copy4(h.point, pt);
				s->OnHit(h);
			}
	// pickups: kind 4
	for (int k : {0, 1})
		for (sEntry& em : L[k])
			for (sEntry& ep : L[4])
				if (PolyOverlap(em.pObj, ep.pObj))
				{
					sGrobdaHit h;
					h.pObj = ep.pObj;
					h.kind = 4;
					em.pObj->OnHit(h);
				}

	// every object processes its records, then the lists of records are emptied
	for (auto& l : L)
		for (sEntry& e : l) e.pObj->Resolve(0, 0);
	for (auto& l : L)
		for (sEntry& e : l) e.pObj->ClearHits();
}

}
