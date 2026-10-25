#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Reader for NW4R BRRES files ("bres"): the texture part, which is all the 2D layouts need. Format references:
//   ~/WiiDevelopmentPackage/.../NintendoWare/Revolution (g3d_resfile / g3d_restex), and ~/Code/proj/unpac/revolution/brres (Python).
// Structure: bres header -> "root" block -> dictionary of folders -> dictionary of named blocks ("Textures(NW4R)" holds TEX0,
// "Palettes(NW4R)" holds PLT0). Everything is big endian unless the byte order mark says otherwise.
namespace wii
{
struct Image
{
	int width{};
	int height{};
	std::vector<std::uint8_t> rgba; // width * height * 4, row 0 = top
};

// GX texture formats (TEX0 `fmt`)
enum GxTexFmt
{
	GX_I4 = 0, GX_I8 = 1, GX_IA4 = 2, GX_IA8 = 3, GX_RGB565 = 4, GX_RGB5A3 = 5, GX_RGBA32 = 6,
	GX_C4 = 8, GX_C8 = 9, GX_C14X2 = 10, GX_CMPR = 14
};
// Palette formats (PLT0 `fmt`)
enum GxTlutFmt { GX_TL_IA8 = 0, GX_TL_RGB565 = 1, GX_TL_RGB5A3 = 2 };

// Bytes of one mip level / the first `levels` levels of a texture in the given format
std::size_t GxLevelSize(int fmt, int width, int height);
std::size_t GxImageSize(int fmt, int width, int height, int levels);

// Decodes one level to RGBA8. `palette` (RGBA, 4 bytes per entry) is required for the C* formats. Returns false on bad input.
bool GxDecode(int fmt, const std::uint8_t* data, std::size_t avail, int width, int height, const std::vector<std::uint8_t>* palette, Image& out);
bool GxDecodePalette(int tlutFmt, const std::uint8_t* data, std::size_t avail, int count, std::vector<std::uint8_t>& rgba);

class Brres
{
public:
	bool Load(const std::uint8_t* data, std::size_t size);

	const std::vector<std::string>& TextureNames() const { return m_texNames; }
	bool HasTexture(const std::string& name) const { return m_tex.count(name) != 0; }

	const std::vector<std::string>& ModelNames() const { return m_mdlNames; }
	// Raw access for the model reader: the whole file and the offset of a model's MDL0 block (-1 if absent).
	const std::vector<std::uint8_t>& Data() const { return m_data; }
	bool BigEndian() const { return m_be; }
	long ModelOffset(const std::string& name) const
	{
		auto it = m_mdl.find(name);
		return it == m_mdl.end() ? -1 : (long)it->second;
	}

	// Every folder of the root dictionary ("AnmChr(NW4R)", "AnmTexSrt(NW4R)", ...): (entry name, absolute offset of the block)
	const std::vector<std::pair<std::string, std::uint32_t>>* Folder(const std::string& folder) const
	{
		auto it = m_folders.find(folder);
		return it == m_folders.end() ? nullptr : &it->second;
	}

	// Level 0 of the named texture as RGBA8. C* textures use the palette of the same name.
	bool DecodeTexture(const std::string& name, Image& out, int level = 0) const;

private:
	struct Dic
	{
		std::vector<std::pair<std::string, std::uint32_t>> entries; // name, absolute offset of the data
	};
	bool ParseDic(std::uint32_t offset, Dic& out) const;
	std::string ReadBinString(std::uint32_t offset) const;
	std::uint32_t U32(std::uint32_t o) const;
	std::uint16_t U16(std::uint32_t o) const;

	std::vector<std::uint8_t> m_data;
	bool m_be{true};
	std::map<std::string, std::uint32_t> m_tex; // TEX0 block offsets
	std::map<std::string, std::uint32_t> m_plt; // PLT0 block offsets
	std::map<std::string, std::uint32_t> m_mdl; // MDL0 block offsets
	std::vector<std::string> m_mdlNames;
	std::vector<std::string> m_texNames;        // in file order
	std::map<std::string, std::vector<std::pair<std::string, std::uint32_t>>> m_folders;
};
}
