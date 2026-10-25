// cKingdomSave against a real Dolphin save (copied to a temp folder first; the original is never written).
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include "WiiSystem.hpp"
#include "kingdom/cKingdomSave.hpp"

namespace fs = std::filesystem;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)

static std::vector<char> slurp(const fs::path& p)
{
	std::ifstream f(p, std::ios::binary);
	return std::vector<char>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

int main()
{
	const fs::path real = fs::path(std::getenv("HOME")) / ".local/share/dolphin-emu/Wii/title/00010000/534e4d45/data";
	if (!fs::exists(real / "museum.bin"))
	{
		std::printf("SKIP: no Dolphin save at %s\n", real.c_str());
		return 0;
	}
	const fs::path tmp = fs::temp_directory_path() / "megamix_save_test";
	fs::remove_all(tmp);
	fs::create_directories(tmp);
	fs::copy_file(real / "museum.bin", tmp / "museum.bin");
	fs::copy_file(real / "banner.bin", tmp / "banner.bin");
	setenv("MEGAMIX_SAVE", tmp.c_str(), 1);
	WiiSystem::Init("/nonexistent.cfg");
	CHECK(WiiSystem::SaveDir() == tmp.string());

	kingdom::cKingdomSave s;
	CHECK(s.Check());                                   // both files present, banner is the right size
	CHECK(s.Status() == kingdom::cKingdomSave::eOK);
	CHECK(s.Load());                                    // CRC of the real file verifies
	CHECK(!s.IsDirty());                                // data matches the stored checksum
	s.Data()[10] = 0x42;
	CHECK(s.IsDirty());                                 // modified
	s.Data()[10] = 0;
	CHECK(!s.IsDirty());

	// byte exact round trip: Save() must reproduce the original museum.bin
	auto original = slurp(real / "museum.bin");
	fs::remove(tmp / "museum.bin");
	CHECK(!s.Check());                                  // museum.bin missing now ...
	CHECK(s.Status() == kingdom::cKingdomSave::eOK);    // ... which is "no save", not an error
	CHECK(s.Save());
	CHECK(slurp(tmp / "museum.bin") == original);

	// corrupt a data byte: Load must reject it as corrupt
	{
		auto bad = original;
		bad[100] ^= 1;
		std::ofstream(tmp / "museum.bin", std::ios::binary).write(bad.data(), bad.size());
	}
	kingdom::cKingdomSave t;
	CHECK(!t.Load());
	CHECK(t.Status() == kingdom::cKingdomSave::eCORRUPT);
	// truncated file: also corrupt
	{
		std::ofstream(tmp / "museum.bin", std::ios::binary).write(original.data(), 1000);
	}
	CHECK(!t.Load());
	CHECK(t.Status() == kingdom::cKingdomSave::eCORRUPT);

	// delete, then CheckSpace creates fresh files
	CHECK(t.Delete());
	CHECK(!fs::exists(tmp / "museum.bin") && !fs::exists(tmp / "banner.bin"));
	CHECK(t.Delete());                                  // missing files are fine
	CHECK(!t.Check());                                  // nothing there
	CHECK(t.CheckSpace());                              // creates both files
	CHECK(fs::file_size(tmp / "museum.bin") == 0xe004 && fs::file_size(tmp / "banner.bin") == 0xf0a0);
	CHECK(slurp(tmp / "museum.bin") == original);       // a fresh save is the all-zero save, identical to the real one
	CHECK(t.Check());

	// a wrong sized banner is rewritten by Check
	{
		std::ofstream(tmp / "banner.bin", std::ios::binary).write(original.data(), 100);
	}
	CHECK(t.Check());
	CHECK(fs::file_size(tmp / "banner.bin") == 0xf0a0);

	// disabled manager: everything "succeeds" and storage is untouched
	fs::remove(tmp / "museum.bin");
	t.SetDisabled(true);
	CHECK(t.Check() && t.Load() && t.Save() && t.CheckSpace() && t.Delete());
	CHECK(!fs::exists(tmp / "museum.bin"));

	fs::remove_all(tmp);
	std::printf("save tests passed\n");
	return 0;
}
