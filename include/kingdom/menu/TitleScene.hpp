#pragma once

#include "kingdom/cKingdomShadow.hpp"
#include <memory>
#include <random>
#include <string>

#include "gfx/ModelDraw.hpp"
#include "gfx/PtkSim.hpp"
#include "wii/Anim.hpp"

namespace kingdom::menu
{

// The 3D scene behind the title (job 137): sky, castle gate, tape flags and Pac-Man, seen through the camera of
// TITLE_CAMERA_GATE01_SCA. On the Wii these are the cTaskBg / cTaskTape / cTaskPacman / cTaskCamera children of cAppTitle (g3d
// model tasks); the PC port drives them from one object that cAppTitle updates and draws, the task tree keeps the original names.
//
// Source functions (DOL): cTaskBg::Init 0x8011f908, Execute 0x8011fa30, start animation 0x8011fad0 (gate clip TITLE_A01_CHA);
// cTaskTape::Init 0x801216e8; cTaskPacman::Init 0x8012056c, Execute 0x80120914, SetMode 0x80121308; cTaskCamera ctor 0x8011fbc4,
// FUN_8011fe14 (loads the camera from the SCN0, CSV "eCSV_FAR,40.0" = vertical fov) and FUN_8011fff8 (applies position / aim / twist).
// Model positions are the translations of the nodes of TITLE_SET_MDL (FUN_8001dd54 copies a node translation into the model holder).
//
// UNVERIFIED: animation rate (1 clip frame per game frame), fog slots not applied, the unlit shading (the scene lights are white ambient + one white
// directional light, which saturates the materials), the effect task (cTaskEffect wind particles) and the pad-B freeze of the
// original are not simulated beyond Update(frozen).
class TitleScene
{
public:
	// Needs g10_menu/g10title.lzs attached. False if a resource is missing (the scene then draws nothing).
	bool Load();

	// One game frame: advances every clip. `frozen` = the pad hold bit 0x400 (B) that every scene task checks (DOL 0x8011fa44 etc.).
	void Update(bool frozen);

	// cAppTitle state 2: FUN_8011fad0 (gate opens) and FUN_80121308(pacman, 1) (Pac-Man reacts)
	void StartPressed();

	// Draws the scene into the 4:3 area (clears depth); call before the 2D layers.
	void Draw();

	bool loaded() const { return m_loaded; }

private:
	struct Holder
	{
		gfx::ModelDraw model;
		float pos[3]{};
	};
	// Pac-Man's four modes: (texture pattern, bone clip) per mode, DOL 0x80120914 / 0x80121308
	struct PacMode { int tpa; int cha; };
	void PacSetMode(int mode);
	void PacExecute();
	void LoadAnim(const std::string& entry, wii::ChrAnim& a);

	bool m_loaded{};
	Holder m_sky, m_gate, m_tape, m_pac;
	wii::SrtAnim m_cloud;                  // TITLE_CLOUD_TSA on the sky
	wii::ChrAnim m_gateAnim, m_tapeAnim;   // TITLE_A01_CHA, TITLE_TAPE_CHA
	wii::ChrAnim m_pacCha[10];             // TITLE_PACMAN_01..10_CHA (only 01, 04, 07 are used by the title)
	wii::PatAnim m_pacTpa[6];              // TITLE_PACMAN_01..06_TPA
	wii::CameraResult m_cam;
	kingdom::cKingdomShadow m_shadow;      // the scene root's light camera (+0x1fc)
	wii::LightResult m_lightBg, m_lightMdl; // TITLE_BG_SCA (sky, gate) / TITLE_MDL_SCA (tape, Pac-Man): light set 0 / 1 of the materials (FUN_8001e1f8)

	float m_skyFrame{}, m_gateFrame{}, m_tapeFrame{};
	bool m_gateStarted{};

	// cTaskPacman state: +0x1cc mode, +0x210 frame counter, +0x218 blend weight, +0x214 Mersenne twister (seed 0x20061206)
	int m_pacMode{};
	float m_pacCounter{};
	float m_pacWeight{};
	struct Layer { int tpa{}; int cha{}; float chaFrame{}; float tpaFrame{}; };
	Layer m_pacCur, m_pacPrev;             // kind 0 (current) and kind 1 (outgoing)
	std::mt19937 m_rng{0x20061206u};
	std::unique_ptr<gfx::PtkSim> m_wind;   // TITLE_WIND_PTO leaves (cTaskEffect, DOL 0x801201c4 [audited], step 1.0; emitter at node ef_t_wind01: 0x8011fa8c)
};

}
