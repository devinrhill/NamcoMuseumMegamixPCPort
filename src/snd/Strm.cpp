#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "snd/Dsp.hpp"
#include "snd/Sound.hpp"

namespace snd
{
namespace
{
std::uint32_t Be32(const std::uint8_t* p) { return ((std::uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
std::uint16_t Be16(const std::uint8_t* p) { return (std::uint16_t)((p[0] << 8) | p[1]); }
}

// RSTM: header {sig, bom, version, size, headerSize, blocks, headOff, headSize, adpcOff, adpcSize, dataOff, dataSize};
// HEAD block: refs (relative to the block data after its 8 byte header) to the stream info, track table and channel table.
bool StrmSource::Open(const std::string& path, std::uint32_t startSample)
{
	std::ifstream f(path, std::ios::binary | std::ios::ate);
	if (!f) { std::fprintf(stderr, "StrmSource: cannot open %s\n", path.c_str()); return false; }
	m_file.resize((std::size_t)f.tellg());
	f.seekg(0);
	f.read((char*)m_file.data(), m_file.size());
	const std::uint8_t* d = m_file.data();
	if (m_file.size() < 0x40 || std::memcmp(d, "RSTM", 4) != 0) { std::fprintf(stderr, "StrmSource: %s is not an RSTM\n", path.c_str()); return false; }
	const std::uint32_t head = Be32(d + 16);
	const std::uint8_t* base = d + head + 8;
	const std::uint8_t* info = base + Be32(d + head + 8 + 4);
	m_format = info[0];
	m_loop = info[1] != 0;
	m_channels = info[2];
	m_rate = Be16(info + 4);
	m_loopStart = Be32(info + 8);
	m_loopEnd = Be32(info + 12);
	m_dataOffset = Be32(info + 16);
	m_numBlocks = Be32(info + 20);
	m_blockSize = Be32(info + 24);
	m_blockSamples = Be32(info + 28);
	m_lastBlockSize = Be32(info + 32);
	m_lastBlockSamples = Be32(info + 36);
	m_lastBlockPadded = Be32(info + 40);
	if (m_format != 2 || m_channels < 1 || m_channels > 8 || m_blockSamples == 0)
	{
		std::fprintf(stderr, "StrmSource: %s: unsupported stream (format %d, %d channels)\n", path.c_str(), m_format, m_channels);
		return false;
	}
	const std::uint8_t* chTable = base + Be32(d + head + 8 + 20);
	m_ch.assign(m_channels, Ch{});
	for (int c = 0; c < m_channels; c++)
	{
		const std::uint8_t* chInfo = base + Be32(chTable + 4 + 8 * c + 4);
		const std::uint8_t* set = base + Be32(chInfo + 4);
		for (int i = 0; i < 16; i++) m_ch[c].coef[i] = (std::int16_t)Be16(set + 2 * i);
		m_ch[c].h1 = (std::int16_t)Be16(set + 36);   // AdpcmParam: coef[16], gain, ps, yn1, yn2
		m_ch[c].h2 = (std::int16_t)Be16(set + 38);
		m_ch[c].loopH1 = (std::int16_t)Be16(set + 44); // AdpcmLoopParam: ps, yn1, yn2
		m_ch[c].loopH2 = (std::int16_t)Be16(set + 46);
	}
	// first frame history = the initial one; a start position other than 0 would need the ADPC block (not used by the game's cues)
	m_pos = 0;
	m_inFrame = 14;
	(void)startSample;
	return true;
}

void StrmSource::SeekSample(std::uint32_t sample, bool useLoopHistory)
{
	m_pos = sample;
	m_inFrame = 14;
	if (useLoopHistory)
		for (Ch& c : m_ch) { c.h1 = c.loopH1; c.h2 = c.loopH2; }
}

// byte offset of the DSP frame containing `sample` for channel c
static std::uint32_t FrameByte(const StrmSource& s, std::uint32_t dataOffset, std::uint32_t numBlocks, std::uint32_t blockSize, std::uint32_t blockSamples,
                               std::uint32_t lastPadded, int channels, int c, std::uint32_t sample)
{
	std::uint32_t b = sample / blockSamples;
	if (b >= numBlocks) b = numBlocks - 1;
	const std::uint32_t inBlock = sample - b * blockSamples;
	const std::uint32_t stride = (b == numBlocks - 1) ? lastPadded : blockSize;
	return dataOffset + b * blockSize * channels + c * stride + (inBlock / 14) * 8;
}

int StrmSource::Read(std::int16_t* out, int frames)
{
	int n = 0;
	while (n < frames)
	{
		if (m_pos >= m_loopEnd)
		{
			if (!m_loop) break;
			SeekSample(m_loopStart, true);
		}
		if (m_inFrame >= 14)
		{
			const std::uint32_t fstart = m_pos - m_pos % 14;
			for (int c = 0; c < m_channels; c++)
			{
				const std::uint32_t at = FrameByte(*this, m_dataOffset, m_numBlocks, m_blockSize, m_blockSamples, m_lastBlockPadded, m_channels, c, fstart);
				if (at + 8 > m_file.size()) { std::memset(m_ch[c].buf, 0, sizeof m_ch[c].buf); continue; }
				DecodeDspFrame(m_file.data() + at, m_ch[c].coef, m_ch[c].h1, m_ch[c].h2, m_ch[c].buf);
			}
			m_inFrame = (int)(m_pos - fstart);
		}
		for (int c = 0; c < m_channels; c++) out[(std::size_t)n * m_channels + c] = m_ch[c].buf[m_inFrame];
		m_inFrame++;
		m_pos++;
		n++;
	}
	return n;
}

int WaveSource::Read(std::int16_t* out, int frames)
{
	const Wave& w = *m_wave;
	const std::uint32_t total = (std::uint32_t)(w.pcm.size() / w.channels);
	std::uint32_t end = w.loop ? std::min<std::uint32_t>(w.loopEnd, total) : total;
	if (end == 0) return 0;
	int n = 0;
	while (n < frames)
	{
		if (m_pos >= end)
		{
			if (!w.loop || w.loopStart >= end) break;
			m_pos = w.loopStart;
		}
		std::memcpy(out + (std::size_t)n * w.channels, &w.pcm[(std::size_t)m_pos * w.channels], sizeof(std::int16_t) * w.channels);
		m_pos++;
		n++;
	}
	return n;
}
}
