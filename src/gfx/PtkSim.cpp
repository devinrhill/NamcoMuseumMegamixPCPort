#include "kingdom/MslRand.hpp"
#include "gfx/PtkSim.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "gfx/Gfx3D.hpp"
#include "gfx/TextureCache.hpp"
#include "UnityShim.hpp"

namespace gfx
{
namespace
{
constexpr double DEG = 57.29578, PI = 3.14159265358979;
using M4 = double[16];

void Ident(double* m) { for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0); }
void Mul(const double* a, const double* b, double* o)
{
	double r[16];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) r[i * 4 + j] = a[i * 4] * b[j] + a[i * 4 + 1] * b[4 + j] + a[i * 4 + 2] * b[8 + j] + a[i * 4 + 3] * b[12 + j];
	std::memcpy(o, r, sizeof r);
}
void RotX(double d, double* m) { Ident(m); double c = cos(d / DEG), s = sin(d / DEG); m[5] = c; m[6] = -s; m[9] = s; m[10] = c; }
void RotY(double d, double* m) { Ident(m); double c = cos(d / DEG), s = sin(d / DEG); m[0] = c; m[2] = s; m[8] = -s; m[10] = c; }
void RotZ(double d, double* m) { Ident(m); double c = cos(d / DEG), s = sin(d / DEG); m[0] = c; m[1] = -s; m[4] = s; m[5] = c; }
void Trans(double x, double y, double z, double* m) { Ident(m); m[3] = x; m[7] = y; m[11] = z; }
// Quaternion.Euler(x, y, z) as a matrix: z first, then x, then y
void Euler(double x, double y, double z, double* m)
{
	M4 a, b, c;
	RotY(y, a); RotX(x, b); RotZ(z, c);
	Mul(a, b, m); Mul(m, c, m);
}
void Xform(const double* m, const double* v, double w, double* o)
{
	o[0] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3] * w;
	o[1] = m[4] * v[0] + m[5] * v[1] + m[6] * v[2] + m[7] * w;
	o[2] = m[8] * v[0] + m[9] * v[1] + m[10] * v[2] + m[11] * w;
}
void AxisAngle3(double deg, const double* a, double* r)
{
	double n = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	if (n < 1e-12) { r[0] = r[4] = r[8] = 1; r[1] = r[2] = r[3] = r[5] = r[6] = r[7] = 0; return; }
	double x = a[0] / n, y = a[1] / n, z = a[2] / n, c = cos(deg / DEG), s = sin(deg / DEG), t = 1 - c;
	double v[9] = {t * x * x + c, t * x * y - s * z, t * x * z + s * y, t * x * y + s * z, t * y * y + c, t * y * z - s * x,
	               t * x * z - s * y, t * y * z + s * x, t * z * z + c};
	std::memcpy(r, v, sizeof v);
}

// cJetPtkPtc::_Curve
float Curve(int kind, float st, float ed, float rate)
{
	if (st == ed) return st;
	const float pi = (float)PI;
	switch (kind)
	{
	case 0: rate = 0; break;
	case 2: rate *= rate; break;
	case 3: rate = 1 - rate * rate; break;
	case 4: rate = (std::sin(rate * pi * 2) + 1) / 2; break;
	case 5: rate = std::sin(rate * pi); break;
	case 6: rate = std::sin(rate * pi + pi) + 1; break;
	case 7: rate = (std::cos(rate * pi * 2) + 1) / 2; break;
	case 8: rate = (std::cos(rate * pi) + 1) / 2; break;
	case 9: rate = (std::cos(rate * pi + pi) + 1) / 2; break;
	case 10: rate = std::cos(rate * pi / 2); break;
	case 11: rate = std::sin(rate * pi / 2); break;
	case 12: rate = 1 - std::sin(rate * pi / 2); break;
	case 13: rate = 1 - std::cos(rate * pi / 2); break;
	case 14: rate = std::min(std::max((2 * std::sin(rate * pi * 2) + 1) / 2, 0.0f), 1.0f); break;
	case 15: rate = std::min(2 * std::sin(rate * pi), 1.0f); break;
	case 16: rate = std::max(2 * std::sin(rate * pi + pi) + 1, 0.0f); break;
	default: break; // 1 = linear
	}
	return st + (ed - st) * rate;
}
}

PtkSim::PtkSim(wii::PtkFile file) : m_file(std::move(file))
{
	// cJetPtkEmiAccess::fGetLifeTime
	float num = 0;
	bool done = false;
	for (auto& g : m_file.gens)
	{
		if (g.genFlag & 4)
		{
			num = std::max(num, g.genDead);
			if (g.ptcFlag & 2) { num = std::max(num, g.age + g.ageRnd); continue; }
			num = m_file.scn.loopFrame; done = true; break;
		}
		num = m_file.scn.loopFrame; done = true; break;
	}
	m_life = done ? num : num;
}

// DOL 0x80211048 [audited]: next = next * 0x41c64e6d + 0x3039; return (next >> 16) & 0x7fff
unsigned PtkSim::Rand15()
{
	return (unsigned)kingdom::MslRand();   // one global state, shared with the game code like the DOL's
}
// DOL 0x8008fd04 [audited]: equal bounds -> lo; swapped if lo > hi; rand / 32767 * (hi - lo) + lo (hi is reachable)
float PtkSim::Rand(float lo, float hi)
{
	if (lo == hi) return lo;
	if (lo > hi) std::swap(lo, hi);
	return (float)Rand15() / 32767.0f * (hi - lo) + lo;
}
// DOL 0x8008fc98 [audited]: lo + rand % (hi - lo + 1)
int PtkSim::RandUint(int lo, int hi)
{
	if (lo == hi) return lo;
	if (lo > hi) std::swap(lo, hi);
	return lo + (int)(Rand15() % (unsigned)(hi - lo + 1));
}

unsigned PtkSim::Spawn(const float* matrix, bool oneTime, bool depend, float delay)
{
	auto e = std::make_unique<Emitter>();
	Ident(e->m);
	if (matrix) for (int i = 0; i < 12; i++) e->m[i] = matrix[i];
	e->id = m_nextId++;
	e->oneTime = oneTime; e->depend = depend; e->delay = delay; e->life = m_life;
	for (size_t i = 0; i < m_file.gens.size(); i++)
		if (m_file.gens[i].genFlag & 1)
		{
			e->gens.emplace_back();
			Gen& g = e->gens.back();
			g.emi = e.get(); g.index = (int)i; g.d = &m_file.gens[i];
			Ident(g.mtxRot); Ident(g.mtxAll);
			g.axisRot = Gosa(g.d->genRot, g.d->genRotRnd);
			g.axisRotSpd = Gosa(g.d->genRotSpd, g.d->genRotSpdRnd);
		}
	const unsigned id = e->id;
	m_emis.push_back(std::move(e));
	return id;
}

bool PtkSim::Alive(unsigned id) const
{
	for (auto& e : m_emis) if (e->id == id) return e->use;
	return false;
}

bool PtkSim::Control(unsigned id, const float* m12, int active, int wait, int deadReserve, int visible)
{
	for (auto& e : m_emis)
	{
		if (e->id != id) continue;
		if (!e->use) return false;
		if (m12) for (int i = 0; i < 12; i++) e->m[i] = m12[i];
		if (active >= 0) e->active = active != 0;
		if (wait >= 0) e->wait = wait != 0;
		if (deadReserve >= 0) e->deadReserve = deadReserve != 0;
		if (visible >= 0) e->visible = visible != 0;
		return true;
	}
	return false;
}

void PtkSim::Step(float dt)
{
	for (auto& e : m_emis) EmitterStep(*e, dt);
	m_emis.erase(std::remove_if(m_emis.begin(), m_emis.end(), [&](auto& e) {
		if (e->use) return false;
		for (auto& p : m_ptcs) if (p->emi == e.get()) p->emi = nullptr;
		return true;
	}), m_emis.end());
	for (auto& e : m_emis) for (auto& g : e->gens) GenStep(g, dt);
	std::vector<std::unique_ptr<Ptc>> live;
	for (auto& p : m_ptcs)
	{
		if (!p->emi) continue;
		if (p->emi->active && p->emi->delay <= 0)
		{
			if (!PtcStep(*p, dt)) continue;
		}
		live.push_back(std::move(p));
	}
	m_ptcs.swap(live);
}

// cJetPtkEmi::AnimUpdate
void PtkSim::EmitterStep(Emitter& e, float dt)
{
	if (e.deadReserve) { e.use = false; return; }
	if (!e.active) return;
	if (e.delay > 0) { e.delay -= dt; return; }
	e.delay = 0;
	if (e.wait)
	{
		if (e.activePtc <= 0) { e.deadReserve = true; return; }
	}
	else
	{
		if (e.life <= -1.0f) return;
		e.time += dt;
		if (e.life <= e.time) { e.time = 0; if (e.oneTime) e.wait = true; }
	}
	e.activePtc = 0;
}

// cJetPtkGen::AnimUpdate
void PtkSim::GenStep(Gen& g, float dt)
{
	Emitter& e = *g.emi;
	if (!e.use || !e.active || e.delay > 0) return;
	const wii::PtkGen& d = *g.d;
	float num = e.time;
	if (num == 0)
	{
		g.axisRot = Gosa(d.genRot, d.genRotRnd);
		g.axisRotSpd = Gosa(d.genRotSpd, d.genRotSpdRnd);
	}
	if ((d.genFlag & 2) && num < d.genBorn) g.ptcBorn = 0;
	else if ((d.genFlag & 4) && d.genDead < num) g.ptcBorn = 0;
	else { g.axisRot += g.axisRotSpd * dt; g.ptcBorn += d.genMaxFrm * dt; }
	if (num == 0 || std::fabs(g.ptcBorn) > 1e-6f)
	{
		M4 rot, t, tmp;
		Ident(rot);
		if (std::fabs(d.genDirY) > 1e-6f) { Euler(0, d.genDirY, 0, tmp); Mul(rot, tmp, rot); }
		if (std::fabs(d.genDirX) > 1e-6f) { Euler(d.genDirX, 0, 0, tmp); Mul(rot, tmp, rot); }
		if (std::fabs(g.axisRot) > 1e-6f) { Euler(0, g.axisRot, 0, tmp); Mul(rot, tmp, rot); }
		std::memcpy(g.mtxRot, rot, sizeof rot);
		Trans(d.genPos[0], d.genPos[1], d.genPos[2], t);
		Mul(t, rot, g.mtxAll);
		const double up[3] = {0, 1, 0};
		Xform(rot, up, 0, g.axis);
	}
	if (!e.wait)
	{
		int n = (int)g.ptcBorn;
		if (n > 0)
		{
			if ((int)(g.ptcAlive + n) >= (int)d.genMax) n = (int)(d.genMax - g.ptcAlive);
			for (int i = 0; i < std::max(n, 0); i++)
				if (m_ptcs.size() < m_maxPtcs) { Generate(g); g.ptcAlive += 1; }
			g.ptcBorn -= n;
		}
	}
	e.activePtc += (int)g.ptcAlive;
}

// cJetPtkPtcAccess::Generate (InitWork + InitPos)
void PtkSim::Generate(Gen& g)
{
	const wii::PtkGen& d = *g.d;
	auto p = std::make_unique<Ptc>();
	p->gen = &g; p->emi = g.emi;
	p->age = 0;
	p->ageMax = Gosa(d.age, d.ageRnd);
	if (p->ageMax >= 1.0f) // else the game keeps the particle with its other values at 0
	{
		p->weightS = Gosa(d.weightS, d.weightSRnd);
		p->weightE = d.weightE;
		p->texFrm = d.texPatFrm;
		p->patMin = 0;
		p->patMax = std::max(d.texDivX * d.texDivY - 1, 0);
		if (d.texFlag & 0x1000) p->patMin = std::max(d.texPatRangeS, 0);
		if (d.texFlag & 0x2000) p->patMax = std::max(d.texPatRangeE, 0);
		p->texPat = (d.texFlag & 0x20) ? RandUint(p->patMin, p->patMax) : p->patMin;
		if (!(d.ptcFlag & 0x80000)) { p->sizeX = Gosa(d.sizeX, d.sizeXRnd); p->sizeY = Gosa(d.sizeY, d.sizeYRnd); }
		else { float s = Gosa(0, d.sizeXRnd); p->sizeX = d.sizeX + s; p->sizeY = d.sizeY + s; }
		if (d.ptcFlag & 0x400000) { p->feelS = Gosa(d.feelRotGenS, d.feelRotGenRnd); p->feelE = d.feelRotGenE; }
		if (d.ptcFlag & 0x10000)
		{
			float a = Gosa(0, d.scaleXsRnd), b = Gosa(0, d.scaleYsRnd), c, e;
			if (!(d.ptcFlag & 0x40000)) { c = Gosa(0, d.scaleXeRnd); e = Gosa(0, d.scaleYeRnd); }
			else { c = a; e = b; }
			p->scaleXs = d.scaleXs + a; p->scaleYs = d.scaleYs + b; p->scaleXe = d.scaleXe + c; p->scaleYe = d.scaleYe + e;
		}
		if (d.ptcFlag & 0x200000)
		{
			p->rot1 = Gosa(d.rot[0], d.rotRnd[0]); p->rot1Spd = Gosa(d.rotSpd[0], d.rotSpdRnd[0]);
			p->rot2 = Gosa(d.rot[1], d.rotRnd[1]); p->rot2Spd = Gosa(d.rotSpd[1], d.rotSpdRnd[1]);
			p->rot3 = Gosa(d.rot[2], d.rotRnd[2]); p->rot3Spd = Gosa(d.rotSpd[2], d.rotSpdRnd[2]);
		}
	}
	// InitPos
	const int style = d.genStyle;
	double x = 0, y = 0, z = 0;
	if (style == 1)
	{
		double ang = Rand(0, (float)(PI * 2));
		x = d.genSize[0]; z = d.genSize[2];
		if (d.genFlag & 8) { x = Rand(0.1f, (float)x); z = Rand(0.1f, (float)z); }
		x = std::cos(ang) * x; y = 0; z = std::sin(ang) * z;
	}
	else if (style == 2) { x = Rand(-d.genSize[0], d.genSize[0]); z = Rand(-d.genSize[2], d.genSize[2]); }
	else if (style == 3 || style == 4)
	{
		double v[3] = {Rand(-1, 1), Rand(-1, 1), Rand(-1, 1)};
		double n = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		if (n > 0) for (double& c : v) c /= n;
		double k = (d.genFlag & 8) ? Rand(0, 1) : 1.0;
		x = v[0] * d.genSize[0] * k; y = v[1] * d.genSize[1] * k; z = v[2] * d.genSize[2] * k;
	}
	else if (style == 5) x = Rand(-d.genSize[0], d.genSize[0]);
	else if (style == 6) x = d.genSize[0];
	double pos[3] = {x, y, z}, vel[3];
	if (style == 0) { vel[0] = 0; vel[1] = 1; vel[2] = 0; }
	else if (style == 1 || style == 2 || style == 5 || style == 6)
	{
		M4 m;
		Euler(0, (-std::atan2(pos[2], pos[0])) * DEG, Gosa(d.genAngle, d.genAngleRnd), m);
		const double ax[3] = {1, 0, 0};
		Xform(m, ax, 0, vel);
	}
	else
	{
		double s = style == 3 ? 1.0 : -1.0;
		double n = std::sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
		for (int i = 0; i < 3; i++) vel[i] = n > 0 ? s * pos[i] / n : 0;
	}
	double sp = Gosa(d.genPower, d.genPowerRnd);
	for (double& c : vel) c *= sp;
	double wp[3], wv[3];
	Xform(g.mtxAll, pos, 1, wp);
	Xform(g.mtxRot, vel, 0, wv);
	for (int i = 0; i < 3; i++) { p->p[i] = p->pn[i] = wp[i]; p->v[i] = wv[i]; }
	std::memcpy(p->parent, g.emi->m, sizeof p->parent);
	m_ptcs.push_back(std::move(p));
}

// cJetPtkPtc::AnimUpdate
bool PtkSim::PtcStep(Ptc& p, float dt)
{
	const wii::PtkGen& d = *p.gen->d;
	p.age += dt;
	p.texFrm -= dt;
	if (p.age > p.ageMax)
	{
		if (d.ptcFlag & 2) { p.gen->ptcAlive -= 1; return false; }
		p.age = 0;
	}
	std::memcpy(p.p, p.pn, sizeof p.p);
	PtcState(p, dt);
	return true;
}

// cJetPtkPtc::_UpdateState
void PtkSim::PtcState(Ptc& p, float dt)
{
	const wii::PtkGen& d = *p.gen->d;
	const wii::PtkScene& scn = m_file.scn;
	const float num = p.ageMax ? p.age / p.ageMax : 0;
	const unsigned pf = d.ptcFlag;
	double px = p.pn[0], py = p.pn[1], pz = p.pn[2], vx = p.v[0], vy = p.v[1], vz = p.v[2];
	if (pf & 0x400000) // swirls around the generator's axis
	{
		float deg = Curve(d.feelRotGenCurve, p.feelS, p.feelE, num);
		if (std::fabs(deg) > 1e-6f)
		{
			double r[9];
			AxisAngle3(deg, p.gen->axis, r);
			double qx = p.p[0] - d.genPos[0], qy = p.p[1] - d.genPos[1], qz = p.p[2] - d.genPos[2];
			px = r[0] * qx + r[1] * qy + r[2] * qz + d.genPos[0];
			py = r[3] * qx + r[4] * qy + r[5] * qz + d.genPos[1];
			pz = r[6] * qx + r[7] * qy + r[8] * qz + d.genPos[2];
			double nx = r[0] * vx + r[1] * vy + r[2] * vz, ny = r[3] * vx + r[4] * vy + r[5] * vz, nz = r[6] * vx + r[7] * vy + r[8] * vz;
			vx = nx; vy = ny; vz = nz;
		}
		else { px = p.p[0]; py = p.p[1]; pz = p.p[2]; }
	}
	if ((pf & 0x20000000) && (scn.flag & 0x100)) // feels the scene's air
	{
		M4 m;
		Euler(scn.airDirX, scn.airDirY, 0, m);
		float k = Curve(d.feelAirCurve, d.feelAirS, d.feelAirE, num);
		const double a[3] = {0, scn.airPower * k, 0};
		double o[3];
		Xform(m, a, 0, o);
		vx += o[0]; vy += o[1]; vz += o[2];
	}
	vy -= Curve(d.weightCurve, p.weightS, p.weightE, num);
	vx *= d.genAccele; vy *= d.genAccele; vz *= d.genAccele;
	p.v[0] = vx; p.v[1] = vy; p.v[2] = vz;
	p.pn[0] = px + vx * dt; p.pn[1] = py + vy * dt; p.pn[2] = pz + vz * dt;
	float r = 1, gr = 1, b = 1, a = 1;
	if (pf & 0x10)
	{
		r = Curve(d.rgbaCurve[0], d.rgbaS[0], d.rgbaE[0], num);
		gr = Curve(d.rgbaCurve[1], d.rgbaS[1], d.rgbaE[1], num);
		b = Curve(d.rgbaCurve[2], d.rgbaS[2], d.rgbaE[2], num);
	}
	if (pf & 0x100)
	{
		float rate = num;
		if ((pf & 0x200) && d.alphaFrm != 0) rate = std::fmod(p.age, d.alphaFrm) / d.alphaFrm;
		a = Curve(d.rgbaCurve[3], d.rgbaS[3], d.rgbaE[3], rate);
	}
	p.color[0] = r; p.color[1] = gr; p.color[2] = b; p.color[3] = a;
	if (pf & 0x10000)
	{
		p.sx = Curve(d.scaleXCurve, p.scaleXs, p.scaleXe, num);
		p.sy = (pf & 0x20000) ? p.sx : Curve(d.scaleYCurve, p.scaleYs, p.scaleYe, num);
	}
	else p.sx = p.sy = 1;
	const unsigned tf = d.texFlag;
	if (tf & 1)
	{
		if ((tf & 0x10) && d.texPatFrm != 0)
		{
			if (!(p.texFrm > 0))
			{
				for (int guard = 0; p.texFrm < 0 && guard < 1000; guard++)
				{
					p.texFrm += d.texPatFrm;
					if (tf & 0x20) { p.texPat = RandUint(p.patMin, p.patMax); continue; }
					if (++p.texPat > p.patMax) p.texPat = p.patMin;
				}
			}
		}
		else p.texPat = p.patMin + (int)((p.patMax - p.patMin + 1) * Curve(d.texPatCurve, 0, 0.99f, num));
	}
	if (pf & 0x200000) { p.rot1 += p.rot1Spd * dt; p.rot2 += p.rot2Spd * dt; p.rot3 += p.rot3Spd * dt; }
}

void PtkSim::Draw()
{
	if (m_ptcs.empty()) return;
	const float* v = CurrentView();
	const float right[3] = {v[0], v[4], v[8]}, up[3] = {v[1], v[5], v[9]};
	const float eye[3] = {-(v[0] * v[12] + v[1] * v[13] + v[2] * v[14]), -(v[4] * v[12] + v[5] * v[13] + v[6] * v[14]),
	                      -(v[8] * v[12] + v[9] * v[13] + v[10] * v[14])};
	struct Q { const Ptc* p; float c[4][3]; float dist; };
	std::vector<Q> quads;
	const float ds = m_file.scn.drawScale;
	for (auto& pp : m_ptcs)
	{
		const Ptc& p = *pp;
		const wii::PtkGen& d = *p.gen->d;
		if (p.color[3] == 0 || !p.emi || !p.emi->visible) continue;
		const Emitter& e = *p.emi;
		const double* pm = e.depend ? e.m : p.parent;
		double wc[3];
		Xform(pm, p.p, 1, wc);
		double sc = 0;
		for (int c = 0; c < 3; c++) sc += std::sqrt(pm[c] * pm[c] + pm[4 + c] * pm[4 + c] + pm[8 + c] * pm[8 + c]);
		const double k = ds * sc / 3;
		const double sx = (d.ptcFlag & 0x10000) ? p.sx : 1, sy = (d.ptcFlag & 0x10000) ? p.sy : 1;
		const double hx = p.sizeX * 0.5, hy = p.sizeY * 0.5;
		double qd[4][2];
		if (d.ptcStyle == 3) { double t[4][2] = {{-hx, p.sizeY}, {-hx, 0}, {hx, p.sizeY}, {hx, 0}}; std::memcpy(qd, t, sizeof t); }
		else if (d.ptcStyle == 4) { double t[4][2] = {{-hx, 0}, {-hx, -p.sizeY}, {hx, 0}, {hx, -p.sizeY}}; std::memcpy(qd, t, sizeof t); }
		else { double t[4][2] = {{-hx, hy}, {-hx, -hy}, {hx, hy}, {hx, -hy}}; std::memcpy(qd, t, sizeof t); }
		Q q; q.p = &p;
		if (d.ptcFlag & 1) // billboard
		{
			double ang = 0;
			if (d.ptcFlag & 0x100000) ang = DEG * std::atan2(-(p.pn[0] - p.p[0]), p.pn[1] - p.p[1]);
			else if (d.ptcFlag & 0x200000) ang = p.rot1;
			double ca = std::cos(ang / DEG), sa = std::sin(ang / DEG);
			for (int i = 0; i < 4; i++)
			{
				double qx = qd[i][0] * k * sx, qy = qd[i][1] * k * sy, rx = qx * ca - qy * sa, ry = qx * sa + qy * ca;
				for (int c = 0; c < 3; c++) q.c[i][c] = (float)(wc[c] + right[c] * rx + up[c] * ry);
			}
		}
		else // flat quad oriented by its travel direction
		{
			M4 m, t, tmp;
			std::memcpy(m, pm, sizeof m);
			Ident(tmp); tmp[0] = tmp[5] = tmp[10] = ds; Mul(m, tmp, m);
			Trans(p.p[0], p.p[1], p.p[2], t); Mul(m, t, m);
			double dv[3] = {p.v[0], p.v[1], p.v[2]};
			if (d.ptcFlag & 0x100000) for (int i = 0; i < 3; i++) dv[i] = p.pn[i] - p.p[i];
			double yaw = DEG * std::atan2(-dv[0], -dv[2]);
			if (std::fabs(yaw) > 1e-6) { RotY(yaw, tmp); Mul(m, tmp, m); }
			double pitch = DEG * std::atan2(std::sqrt(dv[0] * dv[0] + dv[2] * dv[2]), dv[1]);
			if (std::fabs(pitch) > 1e-6) { const double ax[3] = {-1, 0, 0}; double r[9]; AxisAngle3(pitch, ax, r);
				Ident(tmp); for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) tmp[i * 4 + j] = r[i * 3 + j]; Mul(m, tmp, m); }
			if (p.rot3 || p.rot2 || p.rot1) { Euler(p.rot3, p.rot2, p.rot1, tmp); Mul(m, tmp, m); }
			if (d.ptcFlag & 0x10000) { Ident(tmp); tmp[0] = p.sx; tmp[5] = p.sy; Mul(m, tmp, m); }
			for (int i = 0; i < 4; i++)
			{
				const double in[3] = {qd[i][0], qd[i][1], 0};
				double o[3];
				Xform(m, in, 1, o);
				for (int c = 0; c < 3; c++) q.c[i][c] = (float)o[c];
			}
			wc[0] = m[3]; wc[1] = m[7]; wc[2] = m[11];
		}
		q.dist = (float)((wc[0] - eye[0]) * (wc[0] - eye[0]) + (wc[1] - eye[1]) * (wc[1] - eye[1]) + (wc[2] - eye[2]) * (wc[2] - eye[2]));
		quads.push_back(q);
	}
	std::sort(quads.begin(), quads.end(), [](const Q& a, const Q& b) { return a.dist > b.dist; });
	// Back to front as sorted; consecutive quads with the same state (normally the same generator) go out in ONE call. One call per
	// quad cost ~1000 draw calls a frame in the hub (fountain etc.), which made it CPU / driver bound. The order is unchanged.
	std::vector<Vertex3D> bv;
	std::vector<std::uint32_t> bi;
	DrawState bst;
	auto same = [](const DrawState& a, const DrawState& b)
	{
		return a.tex == b.tex && a.cull == b.cull && a.blend == b.blend && a.blended == b.blended && a.depthTest == b.depthTest &&
		       a.depthWrite == b.depthWrite && a.alphaRef == b.alphaRef && a.alphaFunc == b.alphaFunc && a.linear == b.linear &&
		       a.wrapS == b.wrapS && a.wrapT == b.wrapT && std::memcmp(a.color, b.color, sizeof a.color) == 0;
	};
	auto flush = [&]()
	{
		if (!bi.empty()) DrawTriangles(bv.data(), bv.size(), bi.data(), bi.size(), bst);
		bv.clear();
		bi.clear();
	};
	for (const Q& q : quads)
	{
		const Ptc& p = *q.p;
		const wii::PtkGen& d = *p.gen->d;
		DrawState st;
		st.cull = CULL_NONE;
		if (d.texFlag & 1)
			if (Texture* t = FindTexture(wii::PtkTextureName(d.tex))) st.tex = t->gfxId;
		const int blend = (d.ptcFlag & 0x100) ? d.blendStyle : 0;
		st.blended = blend != 0;
		// DOL 0x8008c1c4 [audited: blend style/alpha test mapping taken from the DOL's GX calls; quad/UV flip rules not re-diffed]: style 0 none, 1 (BLEND, SRCALPHA, INVSRCALPHA), 2 (BLEND, SRCALPHA, ONE), 3 GX_BM_SUBTRACT
		st.blend = blend == 2 ? BLEND_ADD : blend == 3 ? BLEND_SUBTRACT : BLEND_ALPHA;
		st.depthTest = (d.ptcFlag & 8) != 0;
		st.depthWrite = (d.ptcFlag & 4) != 0;
		st.linear = d.texFilter != 0;
		st.wrapS = st.wrapT = WRAP_CLAMP;
		// DOL 0x8008c1c4 (only with ptc_flag & 0x100): func 0 / 8 always, 1 never, 2..7 = LESS EQUAL LEQUAL GREATER NEQUAL GEQUAL vs the reference
		if ((d.ptcFlag & 0x100) && d.alphaTestFunc >= 1 && d.alphaTestFunc <= 7)
		{
			st.alphaFunc = d.alphaTestFunc == 1 ? 0 : d.alphaTestFunc - 1;
			st.alphaRef = d.alphaTestRef; // (char)(255 * ref)
		}
		const int tw = std::max(d.texDivX, 1), th = std::max(d.texDivY, 1), cell = p.texPat;
		float u0 = (float)(cell % tw) / tw, u1 = (float)(cell % tw + 1) / tw, v0 = (float)(cell / tw) / th, v1 = (float)(cell / tw + 1) / th;
		if (d.texFlag & 0x100) std::swap(u0, u1);
		if (d.texFlag & 0x200) std::swap(v0, v1);
		const float uv[4][2] = {{u0, v0}, {u0, v1}, {u1, v0}, {u1, v1}};
		Vertex3D vs[4]{};
		auto cb = [](float f) { return (std::uint8_t)std::lround(std::clamp(f, 0.0f, 1.0f) * 255); };
		for (int i = 0; i < 4; i++)
		{
			std::memcpy(vs[i].pos, q.c[i], sizeof vs[i].pos);
			vs[i].nrm[2] = 1;
			vs[i].uv[0][0] = uv[i][0]; vs[i].uv[0][1] = uv[i][1];
			vs[i].rgba[0] = cb(p.color[0]); vs[i].rgba[1] = cb(p.color[1]); vs[i].rgba[2] = cb(p.color[2]); vs[i].rgba[3] = cb(p.color[3]);
		}
		// corners are TL BL TR BR; double sided
		if (!bv.empty() && (!same(st, bst) || bv.size() + 4 > 60000)) flush();
		bst = st;
		const std::uint32_t b = (std::uint32_t)bv.size();
		bv.insert(bv.end(), vs, vs + 4);
		for (std::uint32_t k : {0u, 1u, 2u, 2u, 1u, 3u}) bi.push_back(b + k);
	}
	flush();
}
}
