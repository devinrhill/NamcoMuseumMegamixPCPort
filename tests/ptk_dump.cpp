// Dumps the generators of PTK entries found in archives: `ptk_dump ARCHIVE ENTRY...`
#include <cstdio>
#include "wii/Archive.hpp"
#include "wii/Ptk.hpp"
int main(int argc, char** argv)
{
	wii::Archive a;
	if (argc < 3 || !a.LoadFile(argv[1])) return 1;
	for (int i = 2; i < argc; i++)
	{
		const wii::Archive::Entry* e = a.Find(argv[i]);
		wii::PtkFile f;
		if (!e || !f.Parse(a.Data(*e), e->size)) { std::printf("%s: bad\n", argv[i]); continue; }
		std::printf("%s loop=%g scale=%g gens=%zu\n", argv[i], f.scn.loopFrame, f.scn.drawScale, f.gens.size());
		for (auto& g : f.gens)
			std::printf(" %s tex=%s flags=%x/%x/%x max=%g/frm=%g style=%d age=%g+-%g size=%g,%g blend=%d pos=%g,%g,%g div=%dx%d\n", g.name.c_str(),
			            wii::PtkTextureName(g.tex).c_str(), g.genFlag, g.ptcFlag, g.texFlag, g.genMax, g.genMaxFrm, g.genStyle, g.age, g.ageRnd,
			            g.sizeX, g.sizeY, g.blendStyle, g.genPos[0], g.genPos[1], g.genPos[2], g.texDivX, g.texDivY);
	}
}
