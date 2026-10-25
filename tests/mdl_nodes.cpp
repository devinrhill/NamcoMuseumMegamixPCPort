// Prints nodes (billboard mode, translate), draws and shapes of a model: `mdl_nodes ARCHIVE ENTRY...`
#include <cstdio>
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"
int main(int argc, char** argv)
{
	wii::Archive a;
	if (argc < 3 || !a.LoadFile(argv[1])) return 1;
	for (int i = 2; i < argc; i++)
	{
		const auto* e = a.Find(argv[i]);
		wii::Brres b;
		if (!e || !b.Load(a.Data(*e), e->size)) { std::printf("%s: missing\n", argv[i]); continue; }
		for (const auto& mn : b.ModelNames())
		{
			wii::Mdl0 m;
			if (!m.Load(b, mn)) continue;
			std::printf("%s/%s nodes=%zu mats=%zu meshes=%zu draws=%zu\n", argv[i], mn.c_str(), m.nodes.size(), m.materials.size(), m.meshes.size(), m.draws.size());
			for (size_t n = 0; n < m.nodes.size(); n++)
				std::printf("  node %zu %s bb=%d t=%.2f,%.2f,%.2f\n", n, m.nodes[n].name.c_str(), m.nodes[n].billboard, m.nodes[n].translate[0], m.nodes[n].translate[1], m.nodes[n].translate[2]);
			for (const auto& mt : m.materials)
			{
				std::printf("  mat %s cull=%d tevs=%d\n", mt.name.c_str(), mt.cullMode, mt.nTevs);
				for (const auto& t : mt.tex)
				{
					std::printf("    tex %s\n", t.texName.c_str());
					wii::Image img;
					if (b.DecodeTexture(t.texName, img))
					{
						std::string f = std::string("/tmp/claude-1000/") + t.texName + ".ppm";
						if (FILE* o = std::fopen(f.c_str(), "wb"))
						{
							std::fprintf(o, "P6 %d %d 255\n", img.width, img.height);
							for (size_t k = 0; k < (size_t)img.width * img.height; k++) std::fwrite(&img.rgba[k * 4], 1, 3, o);
							std::fclose(o);
						}
					}
				}
			}
			for (const auto& s : m.meshes) std::printf("  mesh %s verts=%zu tris=%zu\n", s.name.c_str(), s.verts.size(), s.tris.size() / 3);
		}
	}
}
