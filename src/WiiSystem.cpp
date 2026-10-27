#include "WiiSystem.hpp"

#include <chrono>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace WiiSystem
{
namespace
{
std::string g_dataRoot;
std::string g_languageDir = "en";
std::string g_saveDir = "save";
std::string g_dolPath = "main.dol";
bool g_audio = true;
float g_volume = 1.0f;
int g_remoteSpeaker = 0;   // remote_speaker: the Wii's Wiimote speaker volume setting (WPADGetSpeakerVolume, 0..127); 0 = off

std::string Trim(const std::string& s)
{
	size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
	return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}
}

void Init(const std::string& cfgPath)
{
	const char* home = std::getenv("HOME");
	std::error_code ec;
	// default: a "data" folder in the working directory (the release layout), else the developer's extracted disc
	g_dataRoot = std::filesystem::is_directory("data", ec) ? std::string("data") : std::string(home ? home : ".") + "/Games/Wii/_nmm/DATA/files";

	std::ifstream f(cfgPath);
	std::string line;
	while (std::getline(f, line))
	{
		size_t hash = line.find('#');
		if (hash != std::string::npos) line.erase(hash);
		size_t eq = line.find('=');
		if (eq == std::string::npos) continue;
		std::string key = Trim(line.substr(0, eq)), val = Trim(line.substr(eq + 1));
		if (key == "data_root") g_dataRoot = val;
		else if (key == "language") g_languageDir = val;
		else if (key == "save_dir") g_saveDir = val;
		else if (key == "dol_path") g_dolPath = val;
		else if (key == "audio") g_audio = !(val == "off" || val == "0" || val == "false" || val == "no");
		else if (key == "remote_speaker") g_remoteSpeaker = std::max(0, std::min(127, std::atoi(val.c_str())));
		else if (key == "volume") g_volume = std::max(0.0f, std::min(100.0f, (float)std::atof(val.c_str()))) / 100.0f;
		else std::fprintf(stderr, "%s: unknown key '%s'\n", cfgPath.c_str(), key.c_str());
	}
	// the environment overrides megamix.cfg (docs/OPTIONS.md: command line > environment > megamix.cfg > defaults)
	if (const char* env = std::getenv("MEGAMIX_DATA")) g_dataRoot = env;
	if (const char* env = std::getenv("MEGAMIX_SAVE")) g_saveDir = env;
	if (std::getenv("MEGAMIX_NO_AUDIO")) g_audio = false;
	if (const char* env = std::getenv("MEGAMIX_DOL")) g_dolPath = env;
}

bool AudioEnabled() { return g_audio; }
void SetAudioEnabled(bool on) { g_audio = on; }
float Volume() { return g_volume; }
int RemoteSpeakerVolume() { return g_remoteSpeaker; }

const std::string& DolPath() { return g_dolPath; }
const std::string& DataRoot() { return g_dataRoot; }
void SetDataRoot(const std::string& dir) { g_dataRoot = dir; }
void SetDolPath(const std::string& path) { g_dolPath = path; }

bool ResolveData(std::string& why)
{
	namespace fs = std::filesystem;
	std::error_code ec;
	auto expand = [](std::string v) {
		if (v.size() > 1 && v[0] == '~' && v[1] == '/')
			if (const char* home = std::getenv("HOME")) v = std::string(home) + v.substr(1);
		return v;
	};
	std::string root = expand(g_dataRoot);
	g_dolPath = expand(g_dolPath);
	while (root.size() > 1 && root.back() == '/') root.pop_back();
	std::string found;
	for (const char* sub : {"", "/files", "/DATA/files"})
		if (fs::is_regular_file(root + sub + "/museum/resident.arc", ec)) { found = root + sub; break; }
	if (found.empty())
	{
		why = "The Namco Museum Megamix game data was not found.\n\n"
		      "Looked in: " + (root.empty() ? std::string("(no folder set)") : root) + "\n\n"
		      "Point the game at the folder of your extracted disc that holds the\n"
		      "\"museum\" folder (or its parent, e.g. Dolphin's DATA folder):\n\n"
		      "  - edit megamix.cfg:   data_root = /path/to/files\n"
		      "  - or start it with:   ./megamix.sh --data /path/to/files\n"
		      "  - or set:             MEGAMIX_DATA=/path/to/files";
		return false;
	}
	g_dataRoot = found;
	if (!fs::is_regular_file(g_dolPath, ec))
	{
		const std::string disc = fs::path(found).parent_path().string() + "/sys/main.dol";   // DATA/files -> DATA/sys/main.dol
		if (fs::is_regular_file(disc, ec)) g_dolPath = disc;
	}
	return true;
}
const std::string& SaveDir() { return g_saveDir; }
const std::string& LanguageDir() { return g_languageDir; }

GameDefine::ELanguage Language()
{
	if (g_languageDir == "jp" || g_languageDir == "ja") return GameDefine::eJAPANESE;
	if (g_languageDir == "fr") return GameDefine::eFRENCH;
	if (g_languageDir == "it") return GameDefine::eITALIAN;
	if (g_languageDir == "de") return GameDefine::eGERMAN;
	if (g_languageDir == "es") return GameDefine::eSPANISH;
	return GameDefine::eENGLISH;
}

int ScLanguage()
{
	if (g_languageDir == "jp" || g_languageDir == "ja") return 0;
	if (g_languageDir == "de") return 2;
	if (g_languageDir == "fr") return 3;
	if (g_languageDir == "es") return 4;
	if (g_languageDir == "it") return 5;
	if (g_languageDir == "nl") return 6;
	if (g_languageDir == "kr" || g_languageDir == "ko") return 9;
	return 1; // en
}

long long OSGetTime()
{
	static const auto t0 = std::chrono::steady_clock::now();
	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0).count();
	return static_cast<long long>(static_cast<double>(ns) * 60.75e6 / 1e9);
}
}
