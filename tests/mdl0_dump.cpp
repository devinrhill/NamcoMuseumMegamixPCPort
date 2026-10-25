// Prints "<entry>/<model> <shape> verts tris sum(pos) sum(nrm)" for every MDL0 shape; compared with the unpac Python parser.
#include <cstdio>
#include <string>
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

int main(int argc, char** argv)
{
	int bad = 0;
	for (int i = 1; i < argc; i++)
	{
		wii::Archive arc;
		if (!arc.LoadFile(argv[i])) { std::fprintf(stderr, "cannot load %s\n", argv[i]); return 2; }
		for (const auto& e : arc.Entries())
		{
			wii::Brres b;
			if (!b.Load(arc.Data(e), e.size)) continue;
			for (const auto& mn : b.ModelNames())
			{
				wii::Mdl0 m;
				if (!m.Load(b, mn)) { std::printf("%s/%s FAILED\n", e.name.c_str(), mn.c_str()); bad++; continue; }
				std::printf("%s/%s nodes=%zu mats=%zu draws=%zu\n", e.name.c_str(), mn.c_str(), m.nodes.size(), m.materials.size(), m.draws.size());
				for (const auto& s : m.meshes)
				{
					double sp = 0, sn = 0;
					for (const auto& v : s.verts) { sp += v.pos[0] + v.pos[1] + v.pos[2]; sn += v.nrm[0] + v.nrm[1] + v.nrm[2]; }
					std::printf("  %s %zu %zu %.3f %.3f\n", s.name.c_str(), s.verts.size(), s.tris.size() / 3, sp, sn);
				}
			}
		}
	}
	return bad ? 1 : 0;
}
