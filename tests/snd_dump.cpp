// Decodes cues of the sound archive to WAV files and prints a summary: tests/snd_dump ARCHIVE.brsar SOUNDDIR OUTDIR cue...
// (no cue = every cue, summary only). Used to check the RSAR / RWSD / RWAV / RSTM readers.
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include "snd/Sound.hpp"

static void WriteWav(const std::string& path, const std::vector<std::int16_t>& pcm, int ch, int rate)
{
	std::ofstream f(path, std::ios::binary);
	auto w32 = [&](std::uint32_t v) { f.write((const char*)&v, 4); };
	auto w16 = [&](std::uint16_t v) { f.write((const char*)&v, 2); };
	const std::uint32_t bytes = (std::uint32_t)(pcm.size() * 2);
	f.write("RIFF", 4); w32(36 + bytes); f.write("WAVEfmt ", 8); w32(16); w16(1); w16((std::uint16_t)ch); w32(rate); w32(rate * ch * 2); w16((std::uint16_t)(ch * 2)); w16(16);
	f.write("data", 4); w32(bytes);
	f.write((const char*)pcm.data(), bytes);
}

static void Summary(const char* name, const std::vector<std::int16_t>& pcm, int ch, int rate)
{
	double sum = 0, diff = 0; int peak = 0;
	for (size_t i = 0; i < pcm.size(); i++)
	{
		sum += (double)pcm[i] * pcm[i];
		peak = std::max(peak, std::abs((int)pcm[i]));
		if (i >= (size_t)ch) diff += std::abs((double)pcm[i] - pcm[i - ch]);
	}
	const double rms = std::sqrt(sum / std::max<size_t>(1, pcm.size()));
	std::printf("%-28s %6.2fs %dch %dHz peak=%5d rms=%7.1f smooth=%.3f\n", name, pcm.size() / ch / (double)rate, ch, rate, peak, rms, rms > 0 ? diff / pcm.size() / rms : 0.0);
}

int main(int argc, char** argv)
{
	if (argc < 4) { std::fprintf(stderr, "usage: snd_dump ARCHIVE SOUNDDIR OUTDIR [cue...]\n"); return 2; }
	snd::Archive a;
	if (!a.Load(argv[1])) return 2;
	std::printf("%zu sounds\n", a.count());
	int bad = 0;
	auto one = [&](const snd::SoundDef& s, bool write) {
		std::vector<std::int16_t> pcm; int ch = 1, rate = 32000;
		if (s.type == 3)
		{
			auto w = a.LoadWave(s);
			if (!w) { std::printf("%-28s wave FAILED\n", s.name.c_str()); bad++; return; }
			pcm = w->pcm; ch = w->channels; rate = (int)(w->rate);
		}
		else if (s.type == 2)
		{
			snd::StrmSource st;
			if (!st.Open(std::string(argv[2]) + "/" + s.extPath)) { std::printf("%-28s stream FAILED\n", s.name.c_str()); bad++; return; }
			ch = st.channels(); rate = st.sampleRate();
			const int maxFrames = write ? rate * 30 : rate * 5; // first seconds only
			pcm.resize((size_t)maxFrames * ch);
			const int n = st.Read(pcm.data(), maxFrames);
			pcm.resize((size_t)n * ch);
		}
		else return;
		Summary(s.name.c_str(), pcm, ch, rate);
		if (write) WriteWav(std::string(argv[3]) + "/" + s.name + ".wav", pcm, ch, rate);
	};
	if (argc > 4)
		for (int i = 4; i < argc; i++)
		{
			const snd::SoundDef* s = a.Find(argv[i]);
			if (!s) { std::printf("%s: unknown cue\n", argv[i]); bad++; continue; }
			one(*s, true);
		}
	else
		for (const auto& s : a.sounds()) one(s, false);
	return bad ? 1 : 0;
}
