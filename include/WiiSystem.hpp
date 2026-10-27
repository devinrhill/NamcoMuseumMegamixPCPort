#pragma once

#include <string>

#include "GameDefine.hpp"

// Console-level settings the Wii game reads from the system (SCGetLanguage, the disc, ...).
// Overridable from megamix.cfg ("data_root = ...", "language = en") or the MEGAMIX_DATA environment variable.
namespace WiiSystem
{
void Init(const std::string& cfgPath = "megamix.cfg");

// Root of the extracted disc "files" directory (contains museum/ and opening.bnr).
const std::string& DataRoot();

// Command line overrides (--data / --dol), applied after Init: they win over megamix.cfg and the environment.
void SetDataRoot(const std::string& dir);
void SetDolPath(const std::string& path);

// Finds the disc data under the configured root: the root itself, root/files or root/DATA/files (a Dolphin "Extract Entire Disc"
// folder), whichever holds museum/resident.arc, and moves DataRoot() there. When the configured main.dol does not exist, the disc's
// sys/main.dol next to that files/ folder is used. Returns false (with a message for the user in `why`) when no game data is found.
bool ResolveData(std::string& why);

// Directory standing in for the game's NAND save folder (title 00010000/534e4d45/data). Default "save"; set `save_dir = ...` in
// megamix.cfg or MEGAMIX_SAVE to use another one, e.g. a Dolphin NAND copy.
const std::string& SaveDir();

// Path of the game executable (main.dol), for resources embedded in it (debug font): `dol_path` in megamix.cfg or MEGAMIX_DOL.
const std::string& DolPath();

// Audio: `audio = on|off` and `volume = 0..100` in megamix.cfg, `--no-audio` on the command line, MEGAMIX_NO_AUDIO=1 in the environment.
bool AudioEnabled();
void SetAudioEnabled(bool on);
float Volume();                      // master volume 0..1
// `remote_speaker = 0..127` in megamix.cfg: the Wii's Wiimote speaker volume setting (WPADGetSpeakerVolume); 0 (default) = the remote copies
// of sounds (kingdom 0x80025b6c) are not played. A PC has no Wiimote speaker: the copies are mixed into the main output (as Dolphin does).
int RemoteSpeakerVolume();

// Language directory used under museum/ ("en", ...).
const std::string& LanguageDir();
GameDefine::ELanguage Language();

// SCGetLanguage() (DOL 0x802ae750): 0 JP, 1 EN, 2 DE, 3 FR, 4 ES, 5 IT, 6 NL, 7 ZH-simplified, 8 ZH-traditional, 9 KR.
int ScLanguage();

// OSGetTime() (DOL 0x802a8710): the 60.75 MHz time base (bus clock 243 MHz / 4), counting from process start.
// TicksPerMsec is what the game derives from the bus-clock word at 0x800000f8 (60750).
long long OSGetTime();
constexpr long long TicksPerMsec = 60750;
}
