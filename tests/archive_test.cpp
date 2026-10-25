// Checks the native SSZL/VCRA reader against the real disc data (skips if the data is not there).
#include <cstdio>
#include <cstdlib>
#include "WiiSystem.hpp"
#include "wii/Archive.hpp"

#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); return 1; } } while (0)

int main()
{
	WiiSystem::Init("/nonexistent.cfg");
	std::printf("data root: %s\n", WiiSystem::DataRoot().c_str());

	CHECK(wii::Crc32(std::string("MAINTITLE_ANITA")) == 0x706a47bau); // the hash cAppTitle passes to the archive lookup

	wii::Archive arc;
	if (!arc.LoadFile(WiiSystem::DataRoot() + "/museum/en/g10_menu/g10title.lzs"))
	{
		std::printf("SKIP: g10title.lzs not found\n");
		return 0;
	}
	CHECK(arc.Entries().size() > 100);
	const wii::Archive::Entry* e = arc.Find(0x706a47bau);
	CHECK(e != nullptr);
	CHECK(e->name == "MAINTITLE_ANITA" && e->size == 2240);
	CHECK(arc.Find("TITLE_PACMAN_MDL") && arc.Find("TITLE_PACMAN_MDL")->size == 24576);
	CHECK(arc.Find("nope") == nullptr);

	// through the store, the way cKingdomSys::AttachArcData / SearchData do it
	CHECK(wii::ArchiveStore::Attach("g10_menu/g10title.lzs"));
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(0x706a47bau, size);
	CHECK(d != nullptr && size == 2240);
	CHECK(!wii::ArchiveStore::Attach("g10_menu/does_not_exist.lzs"));
	std::printf("entries: %zu, MAINTITLE_ANITA: %zu bytes, first bytes: %02x %02x %02x %02x\n", arc.Entries().size(), size, d[0], d[1], d[2], d[3]);
	std::printf("archive tests passed\n");
	return 0;
}
