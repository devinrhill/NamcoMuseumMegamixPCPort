// Prints every sampled value of every animation entry of an archives (CHR0 / SRT0 / PAT0 / SCN0 / CLR0); tests/anim_ref.py prints the same
// with the unpac Python readers and tests/run.sh compares the two.
#include <cstdio>
#include "wii/Anim.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"

int main(int argc, char** argv)
{
	wii::Archive arc;
	if (argc < 2 || !arc.LoadFile(argv[1])) { std::fprintf(stderr, "cannot load archive\n"); return 2; }
	for (const auto& e : arc.Entries())
	{
		wii::Brres b;
		if (!b.Load(arc.Data(e), e.size)) continue;
		wii::ChrAnim chr; wii::SrtAnim srt; wii::PatAnim pat; wii::ScnAnim scn; wii::ClrAnim clr;
		if (chr.Load(b))
		{
			std::printf("%s CHR0 frames=%d loop=%d\n", e.name.c_str(), chr.numFrame, chr.loops);
			for (const std::string& bone : chr.BoneNames())
				for (float f : {0.0f, 3.25f, 17.0f})
				{
					wii::BoneKey k;
					chr.Sample(bone, f, k);
					std::printf(" %s @%.2f %.3f %.3f %.3f | %.3f %.3f %.3f | %.3f %.3f %.3f\n", bone.c_str(), f, k.scale[0], k.scale[1], k.scale[2], k.rotate[0], k.rotate[1], k.rotate[2], k.trans[0], k.trans[1], k.trans[2]);
				}
		}
		if (srt.Load(b))
		{
			std::printf("%s SRT0 frames=%d mode=%d loop=%d\n", e.name.c_str(), srt.numFrame, srt.texMtxMode, srt.loops);
			for (const char* m : {"g10b_sky01", "sky02", "cloud", "a_cloud", "z_cloud"})
				for (int mtx = 0; mtx < 2; mtx++)
				{
					wii::TexSrt t;
					for (float f : {0.0f, 100.0f}) if (srt.Sample(m, mtx, f, t)) std::printf(" %s.%d @%.0f %.3f %.3f %.3f %.3f %.3f\n", m, mtx, f, t.su, t.sv, t.rot, t.tu, t.tv);
				}
		}
		if (clr.Load(b))
		{
			std::printf("%s CLR0 frames=%d loop=%d\n", e.name.c_str(), clr.numFrame, clr.loops);
			for (const std::string& m : clr.MaterialNames())
				for (float f : {0.0f, 1.0f, 7.0f})
				{
					wii::ClrResult r;
					clr.Sample(m, f, r);
					for (int t = 0; t < 11; t++)
						if (r.exist & (1u << t)) std::printf(" %s.%d @%.0f mask=%08x rgba=%08x\n", m.c_str(), t, f, r.mask[t], r.rgba[t]);
				}
		}
		if (pat.Load(b)) std::printf("%s PAT0 frames=%d map0@0='%s'\n", e.name.c_str(), pat.numFrame, pat.Sample("pac_eye", 0, 0).c_str());
		if (scn.Load(b))
		{
			wii::CameraResult c;
			scn.SampleCamera("", 0, c);
			std::printf("%s SCN0 frames=%d aim=%d proj=%d pos=%.3f %.3f %.3f aim=%.3f %.3f %.3f twist=%.3f asp=%.3f near=%.3f far=%.1f fov=%.3f\n", e.name.c_str(), scn.numFrame, c.aim, c.projType, c.pos[0], c.pos[1], c.pos[2], c.aimPos[0], c.aimPos[1], c.aimPos[2], c.twist, c.aspect, c.nearZ, c.farZ, c.fovy);
		}
	}
	return 0;
}
