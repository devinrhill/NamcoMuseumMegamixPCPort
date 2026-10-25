#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Reader for the game's executable (main.dol): a few engine resources are embedded in it (the debug text font, the SSZL blob of the
// kingdom system). Virtual addresses are the console's (0x80000000 range). Path: `dol_path` in megamix.cfg, default "main.dol".
namespace wii
{
class Dol
{
public:
	bool Load(const std::string& path);
	bool loaded() const { return !m_raw.empty(); }
	// pointer to `size` bytes at a virtual address (nullptr if outside the sections)
	const std::uint8_t* At(std::uint32_t addr, std::size_t size) const;

private:
	struct Section { std::uint32_t offset, addr, size; };
	std::vector<std::uint8_t> m_raw;
	std::vector<Section> m_sections;
};

// The executable loaded from the configured path (once); nullptr if it cannot be read.
const Dol* GameDol();
}
