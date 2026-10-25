// Prints "<archive entry>/<texture> <w>x<h> <crc32 of the decoded RGBA>" for every BRRES texture in the given archives.
#include <cstdio>
#include <string>
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

int main(int argc, char** argv)
{
	int bad = 0;
	for (int i = 1; i < argc; i++)
	{
		wii::Archive arc;
		if (!arc.LoadFile(argv[i])) { std::fprintf(stderr, "cannot load %s\n", argv[i]); return 2; }
		for (const auto& e : arc.Entries())
		{
			if (e.name.size() < 6 || e.name.substr(e.name.size() - 6) != ".brres") continue;
			wii::Brres b;
			if (!b.Load(arc.Data(e), e.size)) { std::printf("%s FAILED-LOAD\n", e.name.c_str()); bad++; continue; }
			for (const auto& t : b.TextureNames())
			{
				wii::Image img;
				if (!b.DecodeTexture(t, img)) { std::printf("%s/%s FAILED-DECODE\n", e.name.c_str(), t.c_str()); bad++; continue; }
				std::printf("%s/%s %dx%d %08x\n", e.name.c_str(), t.c_str(), img.width, img.height, wii::Crc32(img.rgba.data(), img.rgba.size()));
			}
		}
	}
	return bad ? 1 : 0;
}
