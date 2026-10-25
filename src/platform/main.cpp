// Native PC entry point: a window plus the game's own task system.
#include "platform/platform.hpp"
#include "common.hpp"
#include "cJetTask.hpp"
#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include "platform/boot.hpp"
#include "GamePad.hpp"
#include "gfx/Gfx.hpp"
#include "WiiSystem.hpp"
#include "WiiPad.hpp"
#include "platform/devtools.hpp"
#include "InGamePadManager.hpp"
#include "gfx/Gfx3D.hpp"
#include "snd/Mixer.hpp"
#include "gfx/ModelDraw.hpp"
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include <cmath>
#include <vector>

// Recursively prints a node and its children, one indented line per task.
static void PrintTaskTree(cJetNode* node, int depth = 0)
{
	for (; node != nullptr; node = node->pNextSibling())
	{
		std::string name = node->szName();
		std::printf("%*s%s (kind %u)\n", depth * 2, "", name.empty() ? "<unnamed>" : name.c_str(), node->nNodeKind());
		PrintTaskTree(node->pChild(), depth + 1);
	}
}

// Debug aid: print every Wiimote/Nunchuk button press on channel 0 so bindings can be checked.
static void LogWiiPresses()
{
	static const struct { std::uint32_t bit; const char* name; } kNames[] = {
		{WiiPad::BTN_LEFT, "Left"}, {WiiPad::BTN_RIGHT, "Right"}, {WiiPad::BTN_DOWN, "Down"}, {WiiPad::BTN_UP, "Up"},
		{WiiPad::BTN_PLUS, "+"}, {WiiPad::BTN_MINUS, "-"}, {WiiPad::BTN_HOME, "Home"}, {WiiPad::BTN_1, "1"},
		{WiiPad::BTN_2, "2"}, {WiiPad::BTN_A, "A"}, {WiiPad::BTN_B, "B"}, {WiiPad::BTN_Z, "Z"}, {WiiPad::BTN_C, "C"},
	};
	for (const auto& n : kNames)
	{
		if (WiiPad::Pressed(0) & n.bit) std::printf("wiimote: %s\n", n.name);
	}
}

// --view-model ARCHIVE ENTRY [MODEL]: orbit viewer for one BRRES model (left drag = orbit, wheel = zoom). Used to check the MDL0 renderer.
static int ViewModel(const std::string& archive, const std::string& entry, const std::string& modelName, const std::map<int, std::string>& shots, int argc, char** argv)
{
	// --chr ENTRY / --srt ENTRY / --pat ENTRY: animation entries of the same archive; --at FRAME: freeze them at that frame (else they play at 1 frame per frame)
	std::string chrEntry, srtEntry, patEntry;
	float atFrame = -1;
	for (int i = 1; i + 1 < argc; i++)
	{
		const std::string a = argv[i];
		if (a == "--chr") chrEntry = argv[i + 1];
		else if (a == "--srt") srtEntry = argv[i + 1];
		else if (a == "--pat") patEntry = argv[i + 1];
		else if (a == "--at") atFrame = (float)std::atof(argv[i + 1]);
	}
	if (!wii::ArchiveStore::Attach(archive)) { std::fprintf(stderr, "cannot attach %s\n", archive.c_str()); return 1; }
	std::size_t size = 0;
	const std::uint8_t* data = wii::ArchiveStore::Find(entry, size);
	if (!data) data = wii::ArchiveStore::Find(entry + ".brres", size);
	if (!data) { std::fprintf(stderr, "no entry %s\n", entry.c_str()); return 1; }
	wii::Brres brres;
	if (!brres.Load(data, size) || brres.ModelNames().empty()) { std::fprintf(stderr, "%s: no models\n", entry.c_str()); return 1; }
	for (const auto& n : brres.ModelNames()) std::printf("model: %s\n", n.c_str());
	gfx::ModelDraw md;
	if (!md.Load(brres, modelName.empty() ? brres.ModelNames().front() : modelName)) { std::fprintf(stderr, "model load failed\n"); return 1; }
	auto loadAnim = [&](const std::string& e, auto& anim) {
		if (e.empty()) return false;
		std::size_t sz = 0;
		const std::uint8_t* d = wii::ArchiveStore::Find(e, sz);
		wii::Brres b;
		if (!d || !b.Load(d, sz) || !anim.Load(b)) { std::fprintf(stderr, "cannot load animation %s\n", e.c_str()); return false; }
		return true;
	};
	wii::ChrAnim chr; wii::SrtAnim srt; wii::PatAnim pat;
	const bool hasChr = loadAnim(chrEntry, chr), hasSrt = loadAnim(srtEntry, srt), hasPat = loadAnim(patEntry, pat);
	if (hasSrt) md.SetTexSrt(&srt, 0);
	{
		// default light = TITLE_MDL_SCA light0 (white directional from (4,4,4), white specular, shininess 16); VIEW_NOLIGHT=1 for ambient only
		const float pos[3] = {4, 4, 4}, aim[3] = {0, 0, 0}, white[3] = {1, 1, 1};
		if (!std::getenv("VIEW_NOLIGHT")) md.SetLight(pos, aim, white, white, 16.0f);
	}
	const wii::Mdl0& m = md.Model();
	// bounds from the bind-pose vertices (the MDL0 volume box is not reliable)
	float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f}, c[3], r = 0;
	for (const auto& mesh : m.meshes)
		for (const auto& v : mesh.verts)
			for (int i = 0; i < 3; i++) { lo[i] = std::min(lo[i], v.pos[i]); hi[i] = std::max(hi[i], v.pos[i]); }
	for (int i = 0; i < 3; i++) { c[i] = (lo[i] + hi[i]) * 0.5f; r = std::max(r, hi[i] - lo[i]); }
	std::printf("bounds (%g %g %g) .. (%g %g %g); volume (%g %g %g) .. (%g %g %g)\n", lo[0], lo[1], lo[2], hi[0], hi[1], hi[2],
	            m.volumeMin[0], m.volumeMin[1], m.volumeMin[2], m.volumeMax[0], m.volumeMax[1], m.volumeMax[2]);
	if (r <= 0) r = 100;
	float yaw = std::getenv("VIEW_YAW") ? (float)std::atof(std::getenv("VIEW_YAW")) : 0.5f, pitch = 0.3f, dist = r * 1.5f;
	int frame = 0;
	while (!platform::ShouldClose())
	{
		float dx, dy;
		platform::MouseDelta(dx, dy);
		if (platform::MouseDown(0)) { yaw -= dx * 0.01f; pitch += dy * 0.01f; }
		dist *= std::pow(0.9f, platform::MouseWheel());
		pitch = std::max(-1.5f, std::min(1.5f, pitch));
		platform::BeginFrame();
		const float eye[3] = {c[0] + dist * std::cos(pitch) * std::sin(yaw), c[1] + dist * std::sin(pitch), c[2] + dist * std::cos(pitch) * std::cos(yaw)};
		// lookAt (column-major), up = +Y
		float f[3] = {c[0] - eye[0], c[1] - eye[1], c[2] - eye[2]};
		float fl = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
		for (float& v : f) v /= fl;
		float s[3] = {f[1] * 0 - f[2] * 1, f[2] * 0 - f[0] * 0, f[0] * 1 - f[1] * 0}; // f x up
		float sl = std::sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
		for (float& v : s) v /= sl;
		const float u[3] = {s[1] * f[2] - s[2] * f[1], s[2] * f[0] - s[0] * f[2], s[0] * f[1] - s[1] * f[0]};
		const float view[16] = {s[0], u[0], -f[0], 0, s[1], u[1], -f[1], 0, s[2], u[2], -f[2], 0,
		                        -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]), -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]),
		                        f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2], 1};
		const float fov = 1.0f / std::tan(0.5f * 0.7f), zn = r * 0.01f, zf = r * 20.0f, asp = 4.0f / 3.0f;
		const float proj[16] = {fov / asp, 0, 0, 0, 0, fov, 0, 0, 0, 0, (zf + zn) / (zn - zf), -1, 0, 0, 2 * zf * zn / (zn - zf), 0};
		const float world[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		const float t = atFrame >= 0 ? atFrame : (float)frame;
		if (hasChr) md.SetPose({{&chr, wii::ClipFrame(t, chr.numFrame, chr.loops), 1.0f}});
		if (hasSrt) md.SetTexSrt(&srt, t);
		if (hasPat) md.SetTexPat(&pat, wii::ClipFrame(t, pat.numFrame, pat.loops));
		gfx::Begin3D(view, proj, true);
		md.Draw(world);
		gfx::End3D();
		frame++;
		auto it = shots.find(frame);
		if (it != shots.end()) { std::printf("shot %d -> %s %s\n", frame, it->second.c_str(), gfx::SaveScreenshot(it->second) ? "ok" : "FAILED"); if (shots.rbegin()->first == frame) { platform::EndFrame(); break; } }
		platform::EndFrame();
	}
	return 0;
}


// --help / -h. Keep in sync with docs/OPTIONS.md (the full reference).
static void PrintHelp()
{
	std::puts(
	    "Namco Museum Megamix (native port)\n"
	    "\n"
	    "usage: megamix [options]\n"
	    "\n"
	    "  -h, --help                  print this text and exit\n"
	    "  --job HEX                   start at a job instead of the boot chain (0x08300000 debug menu, 0x08900000 title, 0x08800000 attract)\n"
	    "  --round N                   Pac-N-Roll stage 0..25 (W1S1 = 0) when the game job is entered directly (--job 0x0d300000)\n"
	    "  --g03-start X Y Z           dev: Pac-N-Roll starts Pac-Man at this stage position (also MEGAMIX_G03_START=x,y,z)\n"
	    "  --g03-life N                dev: Pac-N-Roll starts with N extra lives instead of the save's value (also MEGAMIX_G03_LIFE=N)\n"
	    "  --g01-stage N               dev: Galaga stage job entered with --job 0x0b300000 plays stage N (1..10; also MEGAMIX_G01_STAGE=N)\n"
	    "  --g01-invincible            dev: Galaga's Pac-Man takes no damage (also MEGAMIX_G01_INVINCIBLE=1)\n"
	    "  --g02-stage N               dev: Rally-X stage job entered with --job 0x0c300000 plays stage index N (1..30; also MEGAMIX_G02_STAGE=N)\n"
	    "  --g04-time S                dev: Gator Panic's play time is S seconds instead of 120 (also MEGAMIX_G04_TIME=S)\n"
	    "  --g01-autoaim               dev: Galaga shots aim at the nearest enemy (with A held and the pointer on screen; also MEGAMIX_G01_AUTOAIM=1)\n"
	    "  --g03-cookies N             dev: Pac-N-Roll starts with N Pac-Dots in hand (tests gates; also MEGAMIX_G03_COOKIES=N)\n"
	    "  --g03-kill N                dev: Pac-N-Roll deals lethal damage to Pac-Man at game frame N (tests the miss / game over flow; also MEGAMIX_G03_KILL=N)\n"
	    "  --g03-standin               dev: Pac-N-Roll runs the old native stand-in (movement test) instead of the real game\n"
	    "  --next HEX                  job the save/load transition returns to when entered with --job\n"
	    "  --dump-tree [N]             run N frames (default 400) headless-style, print the task tree, exit (first argument)\n"
	    "  --no-smooth                 turn off the 4x MSAA edge smoothing (also MEGAMIX_NOSMOOTH=1)\n"
	    "  --no-nunchuk                Wiimote 1 has no Nunchuk: games show their \"connect the Nunchuk\" card\n"
	    "  --shot FRAME FILE           save a PNG of the window at that frame (repeatable)\n"
	    "  --press FRAME BUTTON        one-frame Wiimote press: a b 1 2 plus minus home left right up down z c shake (repeatable); prefix N: for Wiimote N (2:a)\n"
	    "  --stick FRAME X Y           Nunchuk stick (-1..1, y up) from that frame on (repeatable; the last one at or before the frame wins)\n"
	    "  --hold FRAME BUTTON         hold a Wiimote button from that frame on (same names as --press except home; directions are physical)\n"
	    "  --point FRAME X Y           put the pointer at game pixel X,Y (640x480, origin top-left) from that frame on (repeatable); replaces the mouse\n"
	    "  --preview-all               dev: every Pac-N-Roll / Pac-Motos / Galaga stage and world reads as unlocked, nothing is saved (also MEGAMIX_PREVIEW_ALL=1)\n"
	    "  --tap-every N               dev: inject an A press every N frames\n"
	    "  --no-audio                  run without sound (no audio device is opened)\n"
	    "  --data DIR                  the extracted disc's files folder (holds museum/; DIR/files or DIR/DATA/files also work); overrides data_root\n"
	    "  --dol FILE                  the game's main.dol (default: dol_path, else sys/main.dol next to the files folder); overrides dol_path\n"
	    "  --view-model ARCHIVE ENTRY [MODEL]   orbit viewer for a BRRES model (left drag orbit, wheel zoom)\n"
	    "      --chr ENTRY | --srt ENTRY | --pat ENTRY   play a bone / texture-scroll / texture-swap animation\n"
	    "      --at FRAME                                freeze the animations at a frame\n"
	    "\n"
	    "keys while running: P = print the task tree\n"
	    "\n"
	    "config files (working directory): megamix.cfg (data_root, language, save_dir, dol_path, audio, volume), input.cfg (controller bindings)\n"
	    "environment: MEGAMIX_DATA, MEGAMIX_DOL, MEGAMIX_SAVE, MEGAMIX_NO_AUDIO (override the config); MEGAMIX_G01_PLAYERS, MEGAMIX_G11_PLAYERS\n"
	    "             (dev: start the Galaga / Grobda stage job with N players); many debug switches (docs/OPTIONS.md section 6)\n"
	    "\n"
	    "Full reference: docs/OPTIONS.md");
}

int main(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		const std::string a = argv[i];
		if (a == "--help" || a == "-h" || a == "-?") { PrintHelp(); return 0; }
	}
	// --dump-tree: run a few frames, print the task tree and exit (no keypress needed; used for checking the boot)
	const bool dumpTree = argc > 1 && std::string(argv[1]) == "--dump-tree";
	int frames = 0;
	// --shot FRAME FILE[,FRAME FILE...]: save the frame buffer at those frames (visual checks without a keyboard)
	std::map<int, std::string> shots;
	for (int i = 1; i + 2 < argc + 0; i++)
	{
		if (std::string(argv[i]) == "--shot") shots[std::atoi(argv[i + 1])] = argv[i + 2];
	}
	int shotFrame = 0;
	int tapEvery = 0;
	std::multimap<int, std::pair<float, float>> sticks; // --stick FRAME X Y: Nunchuk stick from that frame on
	std::multimap<int, std::uint64_t> holds; // --hold FRAME BUTTON: holds the button from that frame on
	std::multimap<int, std::uint64_t> presses; // --press FRAME NAME: a one-frame press of a Wiimote button (physical direction names)
	std::multimap<int, std::pair<float, float>> points; // --point FRAME X Y: pointer position (game pixels) from that frame on
	unsigned entryJob = 0, nextJob = 0;
	bool noSmooth = false;
	for (int i = 1; i < argc; i++) if (std::string(argv[i]) == "--no-smooth") noSmooth = true; // own loop: valid as the last argument
	for (int i = 1; i < argc; i++) if (std::string(argv[i]) == "--no-nunchuk") WiiPad::SetNunchukConnected(0, false);
	for (int i = 1; i + 1 < argc; i++)
	{
		const std::string a = argv[i];
		if (a == "--tap-every") tapEvery = std::atoi(argv[i + 1]);
		else if (a == "--press" && i + 2 < argc)
		{
			static const struct { const char* n; std::uint32_t b; } kBtn[] = {{"a", WiiPad::BTN_A}, {"b", WiiPad::BTN_B}, {"1", WiiPad::BTN_1}, {"2", WiiPad::BTN_2}, {"plus", WiiPad::BTN_PLUS}, {"minus", WiiPad::BTN_MINUS}, {"home", WiiPad::BTN_HOME}, {"left", WiiPad::BTN_LEFT}, {"right", WiiPad::BTN_RIGHT}, {"up", WiiPad::BTN_UP}, {"down", WiiPad::BTN_DOWN}, {"z", WiiPad::BTN_Z}, {"c", WiiPad::BTN_C}, {"shake", WiiPad::BTN_SHAKE}};
			std::string nm = argv[i + 2]; std::uint64_t ch = 0;
			if (nm.size() > 2 && nm[1] == ':' && nm[0] >= '1' && nm[0] <= '4') { ch = (std::uint64_t)(nm[0] - '1'); nm = nm.substr(2); }   // "2:a" = Wiimote 2
			for (const auto& k : kBtn) if (nm == k.n) presses.emplace(std::atoi(argv[i + 1]), (ch << 32) | k.b);
		}
		else if (a == "--hold" && i + 2 < argc)
		{
			static const struct { const char* n; std::uint32_t b; } kHold[] = {{"a", WiiPad::BTN_A}, {"b", WiiPad::BTN_B}, {"1", WiiPad::BTN_1}, {"2", WiiPad::BTN_2}, {"plus", WiiPad::BTN_PLUS}, {"minus", WiiPad::BTN_MINUS}, {"left", WiiPad::BTN_LEFT}, {"right", WiiPad::BTN_RIGHT}, {"up", WiiPad::BTN_UP}, {"down", WiiPad::BTN_DOWN}, {"z", WiiPad::BTN_Z}, {"c", WiiPad::BTN_C}, {"shake", WiiPad::BTN_SHAKE}};
			std::string nm = argv[i + 2]; std::uint64_t ch = 0;
			if (nm.size() > 2 && nm[1] == ':' && nm[0] >= '1' && nm[0] <= '4') { ch = (std::uint64_t)(nm[0] - '1'); nm = nm.substr(2); }
			for (const auto& k : kHold) if (nm == k.n) holds.emplace(std::atoi(argv[i + 1]), (ch << 32) | k.b);
		}
		else if (a == "--stick" && i + 3 < argc) sticks.emplace(std::atoi(argv[i + 1]), std::make_pair((float)std::atof(argv[i + 2]), (float)std::atof(argv[i + 3])));
		else if (a == "--point" && i + 3 < argc) points.emplace(std::atoi(argv[i + 1]), std::make_pair((float)std::atof(argv[i + 2]), (float)std::atof(argv[i + 3])));
		else if (a == "--preview-all") devtools::SetPreviewAll(true);
		else if (a == "--job") entryJob = (unsigned)std::strtoul(argv[i + 1], nullptr, 0);
		else if (a == "--next") nextJob = (unsigned)std::strtoul(argv[i + 1], nullptr, 0);
		else if (a == "--g03-start" && i + 3 < argc) devtools::SetG03Start((float)std::atof(argv[i + 1]), (float)std::atof(argv[i + 2]), (float)std::atof(argv[i + 3]));
		else if (a == "--g03-standin") boot::SetG03StandIn(true);
		else if (a == "--g03-life") devtools::SetG03Life(std::atoi(argv[i + 1]));
		else if (a == "--g01-stage") devtools::SetG01Stage(std::atoi(argv[i + 1]));
		else if (a == "--g01-invincible") devtools::SetG01Invincible(true);
		else if (a == "--g01-autoaim") devtools::SetG01AutoAim(true);
		else if (a == "--g02-stage") devtools::SetG02Stage(std::atoi(argv[i + 1]));
		else if (a == "--g04-time") devtools::SetG04Time(std::atoi(argv[i + 1]));
		else if (a == "--g03-cookies") devtools::SetG03Cookies(std::atoi(argv[i + 1]));
		else if (a == "--g03-kill") devtools::SetG03KillFrame(std::atoi(argv[i + 1]));
		else if (a == "--round") boot::SetG03Round(std::atoi(argv[i + 1]));
	}
	const int dumpAfter = (dumpTree && argc > 2) ? std::atoi(argv[2]) : 400;

	platform::SetSmoothing(!noSmooth);
	if (!platform::Init(1280, 720, "Namco Museum Megamix (native)"))
	{
		return 1;
	}

	WiiSystem::Init("megamix.cfg");
	for (int i = 1; i < argc; i++) if (std::string(argv[i]) == "--no-audio") WiiSystem::SetAudioEnabled(false);
	WiiPad::Init("input.cfg");
	for (int i = 1; i + 1 < argc; i++)
	{
		if (std::string(argv[i]) == "--data") WiiSystem::SetDataRoot(argv[i + 1]);
		else if (std::string(argv[i]) == "--dol") WiiSystem::SetDolPath(argv[i + 1]);
	}
	std::string dataError;
	if (!WiiSystem::ResolveData(dataError))
	{
		std::fprintf(stderr, "%s\n", dataError.c_str());
		// a player who started the game from a file manager sees no terminal: show the same text in the window until it is closed
		while (!dumpTree && !platform::ShouldClose())
		{
			platform::BeginFrame();
			platform::DrawText(dataError.c_str(), 40, 40, 20);
			platform::EndFrame();
		}
		platform::Shutdown();
		return 2;
	}
	std::fprintf(stderr, "data: %s\n", WiiSystem::DataRoot().c_str());

	for (int i = 1; i + 2 < argc; i++)
	{
		if (std::string(argv[i]) == "--view-model")
		{
			const int rc = ViewModel(argv[i + 1], argv[i + 2], i + 3 < argc && argv[i + 3][0] != '-' ? argv[i + 3] : "", shots, argc, argv);
			platform::Shutdown();
			return rc;
		}
	}

	boot::SetEntryJob(entryJob, nextJob);
	boot::Init();

	while (!platform::ShouldClose())
	{
		if (tapEvery > 0 && (shotFrame + 1) % tapEvery == 0) WiiPad::InjectHeld(WiiPad::BTN_A);
		for (auto r = presses.equal_range(shotFrame + 1); r.first != r.second; ++r.first) WiiPad::InjectHeld((std::uint32_t)r.first->second, (int)(r.first->second >> 32));
		for (const auto& h : holds) if (h.first <= shotFrame + 1) WiiPad::InjectHeld((std::uint32_t)h.second, (int)(h.second >> 32));
		{ const std::pair<float, float>* st = nullptr; for (const auto& sk : sticks) if (sk.first <= shotFrame + 1) st = &sk.second; if (st) WiiPad::InjectStick(st->first, st->second); }
		for (const auto& pt : points) if (pt.first <= shotFrame + 1) WiiPad::InjectPointer(pt.second.first, pt.second.second); // later entries win
		WiiPad::Update();
		InGamePadManager::Update();
		GamePad::Update();
		LogWiiPresses();
		if (platform::KeyPressed('P'))
		{
			std::printf("--- task tree ---\n");
			PrintTaskTree(boot::Root());
		}
		platform::BeginFrame();
		boot::Frame();
		gfx::DrawLetterbox();   // the game is 640 x 480: nothing it draws shows outside that rectangle
		shotFrame++;
		if (shots.count(shotFrame)) std::printf("shot %d -> %s %s\n", shotFrame, shots[shotFrame].c_str(), gfx::SaveScreenshot(shots[shotFrame]) ? "ok" : "FAILED");
		if (dumpTree && ++frames == dumpAfter)
		{
			std::printf("--- task tree after %d frames ---\n", frames);
			PrintTaskTree(boot::Root());
			break;
		}
		platform::DrawFps(20, 20);
		platform::EndFrame();
	}
	snd::Shutdown();
	platform::Shutdown();
	return 0;
}
