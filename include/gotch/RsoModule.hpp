#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// The arcade collection lives in two relocatable modules on the disc (museum/<lang>/rso_original/nmwii.rso = vol 1, nmwii2.rso = vol 2): Namco's
// emulator framework "gotch" with the arcade ROMs as named data exports (CPU1_MainMemory__6PACMAN, CGROM_A__6PACMAN, ...). The native port runs
// its own C++ translation of the code and takes only the DATA from the player's module: this class reads the file and looks exports up by their
// (mangled) name. No relocation is applied: ROM / table bytes need none (pointer tables of the module are ported as code, not read from here);
// single pointer words (strings) are resolved on request by Pointer().
namespace gotch
{
class RsoModule
{
public:
	struct Export
	{
		const std::uint8_t* data = nullptr;   // bytes in the file image (big-endian as on the Wii)
		std::uint32_t size = 0;               // up to the next export of the same section (the RSO stores no sizes)
		int section = 0;
	};

	// path relative to the data root, e.g. "museum/en/rso_original/nmwii2.rso"; false when the file is missing or not an RSO
	bool Load(const std::string& path);
	bool IsLoaded() const { return !m_image.empty(); }
	// mangled CodeWarrior name, e.g. "CPU1_MainMemory__6PACMAN"; nullptr when absent
	const Export* Find(const std::string& name) const;
	// copies an export into dst (at most n bytes); returns the number of bytes copied (0 = not found)
	std::uint32_t Copy(const std::string& name, void* dst, std::uint32_t n) const;
	// n bytes at a module address as the reversing notes cite it (RSO2 0x80d..: the module image is cited as loaded at 0x80d00000, so address =
	// 0x80d00000 + file offset); for unexported tables (e.g. the CGen pixel layouts). nullptr when outside the file
	const std::uint8_t* At(std::uint32_t addr, std::uint32_t n) const;
	// the pointer word at a module address (RSO2 0x80d..), resolved through the module's internal ADDR32 relocation for it (the file holds the
	// word unrelocated): the target's bytes in the image, e.g. the MenuStr_* string pointers. nullptr when the word has no such relocation
	const char* Pointer(std::uint32_t addr) const;
	// the big-endian word at a module address as the loaded module holds it: a word with an internal ADDR32 relocation becomes its target's
	// module address (0x80d00000 + file offset), any other word is read as it is. 0 outside the file
	std::uint32_t Word(std::uint32_t addr) const;

	// vol 1 (nmwii.rso, games 0..8) / vol 2 (nmwii2.rso, games 9..17): loaded on first use from the data root's language folder
	static const RsoModule& Volume(int vol);

private:
	std::vector<std::uint8_t> m_image;
	std::unordered_map<std::string, Export> m_exports;
	std::unordered_map<std::uint32_t, std::uint32_t> m_pointers;   // file offset of an ADDR32 word -> file offset of its target
};
}
