#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <vector>

#include "wii/Ptk.hpp"

// Jet particle effects (cJetPtkEmi / cJetPtkGen / cJetPtkPtc). Port of the user's unpac/kingdom/jetptk/sim.py, which derives from
// the converted C# jet/ptk; NOT traced against the DOL (UNVERIFIED). The random sequence differs from the original (std::mt19937).
// Units: one Step() = one game frame (60 Hz). Matrices are row-major 3x4 / 4x4 (m[row*4+col]), column vectors.
namespace gfx
{
class PtkSim
{
public:
	// the random numbers come from the one global rand() (kingdom::MslRand, DOL 0x80211048), seeded by the title
	explicit PtkSim(wii::PtkFile file);
	// Starts an emitter at `matrix` (16 floats row-major, or nullptr = identity); `delay` frames before it runs.
	// Returns the emitter's id (for Control / Alive).
	unsigned Spawn(const float* matrix, bool oneTime = false, bool depend = true, float delay = 0);
	// Pushes the owner's state into emitter `id`: 12 floats row-major 3x4 matrix (nullptr = keep), flags (-1 = keep). False if it no longer exists.
	bool Control(unsigned id, const float* matrix12, int active, int wait, int deadReserve, int visible);
	bool Alive(unsigned id) const;
	void Step(float dt = 1.0f);
	// Draws all particles with the camera of the open 3D pass (gfx::Begin3D); farthest first. Call between Begin3D / End3D.
	void Draw();
	const wii::PtkFile& File() const { return m_file; }
	size_t ParticleCount() const { return m_ptcs.size(); }

private:
	struct Emitter;
	struct Gen
	{
		Emitter* emi; int index; const wii::PtkGen* d;
		float ptcAlive{}, ptcBorn{}, axisRot{}, axisRotSpd{};
		double mtxRot[16], mtxAll[16];
		double axis[3]{0, 1, 0};
	};
	struct Emitter
	{
		double m[16];
		unsigned id{};
		bool oneTime, depend, active{true}, wait{}, deadReserve{}, use{true}, visible{true};
		float delay, time{}, life;
		int activePtc{};
		std::vector<Gen> gens;
	};
	struct Ptc
	{
		Gen* gen; Emitter* emi;
		double parent[16]; // C# cJetPtkPtc.pGetParentMtx: the emitter matrix at birth; depend emitters use their live matrix instead
		double p[3]{}, pn[3]{}, v[3]{};
		float color[4]{1, 1, 1, 1}, sx{1}, sy{1}, age{}, ageMax{1}, texFrm{}, weightS{}, weightE{};
		float scaleXs{}, scaleYs{}, scaleXe{}, scaleYe{}, rot1{}, rot1Spd{}, rot2{}, rot2Spd{}, rot3{}, rot3Spd{};
		float sizeX{}, sizeY{}, feelS{}, feelE{};
		int texPat{}, patMin{}, patMax{};
	};
	unsigned Rand15();                      // libc rand(), DOL 0x80211048
	float Rand(float lo, float hi);
	float Gosa(float v, float g) { return Rand(v - g, v + g); }
	int RandUint(int lo, int hi);
	void EmitterStep(Emitter& e, float dt);
	void GenStep(Gen& g, float dt);
	void Generate(Gen& g);
	bool PtcStep(Ptc& p, float dt);
	void PtcState(Ptc& p, float dt);

	wii::PtkFile m_file;
	float m_life{};
	std::vector<std::unique_ptr<Emitter>> m_emis;
	unsigned m_nextId{1};
	std::vector<std::unique_ptr<Ptc>> m_ptcs;
	size_t m_maxPtcs{1024};
};
}

namespace gfx
{
// Every PtkSim owned by the particle manager draws here (called by NativeScene::Flush inside the 3D pass).
void DrawParticles();
}
