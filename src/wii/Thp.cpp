#include "wii/Thp.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace wii
{
namespace
{
std::uint32_t Be32(const std::uint8_t* p) { return ((std::uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
std::int16_t Be16(const std::uint8_t* p) { return (std::int16_t)((p[0] << 8) | p[1]); }
}

bool Thp::Open(const std::string& path)
{
	Close();
	m_file.open(path, std::ios::binary);
	if (!m_file) { std::fprintf(stderr, "THP: cannot open %s\n", path.c_str()); return false; }
	std::uint8_t h[0x30];
	if (!m_file.read((char*)h, sizeof h) || std::memcmp(h, "THP\0", 4) != 0) { std::fprintf(stderr, "THP: %s is not a THP file\n", path.c_str()); Close(); return false; }
	const std::uint32_t version = Be32(h + 4);
	std::uint32_t fr = Be32(h + 16);
	std::memcpy(&m_frameRate, &fr, 4);
	m_numFrames = (int)Be32(h + 20);
	m_firstFrameSize = Be32(h + 24);
	const std::uint32_t compInfo = Be32(h + 32);
	m_movieOffset = Be32(h + 40);
	if (version != 0x11000 && version != 0x10000) std::fprintf(stderr, "THP: unexpected version %x\n", version);

	std::uint8_t ci[4 + 16 + 12 + 16];
	m_file.seekg(compInfo);
	if (!m_file.read((char*)ci, sizeof ci)) { Close(); return false; }
	const int n = (int)Be32(ci);
	m_compKinds.assign(ci + 4, ci + 4 + std::min(n, 16));
	std::size_t p = 4 + 16;
	for (std::uint8_t kind : m_compKinds)
	{
		if (kind == 0) { m_width = (int)Be32(ci + p); m_height = (int)Be32(ci + p + 4); p += 12; }          // THPVideoInfo
		else if (kind == 1) { m_audioChannels = (int)Be32(ci + p); m_audioRate = (int)Be32(ci + p + 4); p += 16; } // THPAudioInfo
	}
	Rewind();
	return m_width > 0 && m_height > 0;
}

void Thp::Close()
{
	if (m_file.is_open()) m_file.close();
	m_file.clear();
	m_width = m_height = m_numFrames = m_audioChannels = m_audioRate = 0;
}

void Thp::Rewind()
{
	m_next = 0;
	m_pos = m_movieOffset;
	m_nextSize = m_firstFrameSize;
}

bool Thp::ReadFrame(Frame& out)
{
	if (!m_file || m_next >= m_numFrames) return false;
	const std::size_t hdr = 8 + 4 * m_compKinds.size();
	std::vector<std::uint8_t> buf(m_nextSize);
	m_file.clear();
	m_file.seekg((std::streamoff)m_pos);
	if (!m_file.read((char*)buf.data(), buf.size()) || buf.size() < hdr) return false;
	const std::uint32_t sizeNext = Be32(buf.data());
	std::size_t at = hdr;
	out.index = m_next;
	out.video.clear();
	out.audio.clear();
	for (std::size_t i = 0; i < m_compKinds.size(); i++)
	{
		const std::uint32_t sz = Be32(buf.data() + 8 + 4 * i);
		if (at + sz > buf.size()) return false;
		if (m_compKinds[i] == 0) out.video.assign(buf.begin() + at, buf.begin() + at + sz);
		else if (m_compKinds[i] == 1) out.audio.assign(buf.begin() + at, buf.begin() + at + sz);
		at += sz;
	}
	m_pos += m_nextSize;
	m_nextSize = sizeNext;
	m_next++;
	return true;
}

std::vector<std::uint8_t> Thp::ToStandardJpeg(const std::vector<std::uint8_t>& in)
{
	// walk the marker segments to the start of the scan data (after the SOS segment)
	std::size_t i = 2;
	while (i + 4 <= in.size())
	{
		if (in[i] != 0xff) return in; // not what we expect: let the decoder complain
		const int marker = in[i + 1];
		const std::size_t len = ((std::size_t)in[i + 2] << 8) | in[i + 3];
		i += 2 + len;
		if (marker == 0xda) break;
	}
	if (i >= in.size()) return in;
	// the scan ends at the last EOI (padding follows it)
	std::size_t end = in.size();
	while (end >= i + 2 && !(in[end - 2] == 0xff && in[end - 1] == 0xd9)) end--;
	if (end < i + 2) end = in.size(); else end -= 2;
	std::vector<std::uint8_t> out(in.begin(), in.begin() + i);
	out.reserve(in.size() + in.size() / 64);
	for (std::size_t k = i; k < end; k++)
	{
		out.push_back(in[k]);
		if (in[k] == 0xff) out.push_back(0x00);
	}
	out.push_back(0xff);
	out.push_back(0xd9);
	return out;
}

// GC/Wii DSP-ADPCM: 8 byte frames = 1 header byte (coef index << 4 | scale exponent) + 14 nibbles
int Thp::DecodeAudio(const std::vector<std::uint8_t>& block, std::vector<std::int16_t>& out) const
{
	const int ch = m_audioChannels;
	if (ch <= 0 || ch > 2 || block.size() < 8) return 0;
	const std::uint32_t chBytes = Be32(block.data());
	const int samples = (int)Be32(block.data() + 4);
	const std::size_t dataStart = 8 + 32 * 2 + 4 * 2; // header, two coefficient sets, two histories (always laid out for 2 channels)
	if (samples <= 0 || block.size() < dataStart) return 0;
	const std::size_t base = out.size();
	out.resize(base + (std::size_t)samples * ch);
	for (int c = 0; c < ch; c++)
	{
		std::int16_t coef[16];
		for (int i = 0; i < 16; i++) coef[i] = Be16(block.data() + 8 + 32 * c + 2 * i);
		int h1 = Be16(block.data() + 8 + 64 + 4 * c), h2 = Be16(block.data() + 8 + 64 + 4 * c + 2);
		const std::uint8_t* d = block.data() + dataStart + (std::size_t)c * chBytes;
		const std::uint8_t* end = block.data() + block.size();
		for (int s = 0; s < samples; s++)
		{
			const int frame = s / 14, nib = s % 14;
			const std::uint8_t* f = d + frame * 8;
			if (f + 8 > end) { out[base + (std::size_t)s * ch + c] = 0; continue; }
			const int scale = 1 << (f[0] & 0xf), idx = f[0] >> 4 & 7;
			int v = nib & 1 ? f[1 + nib / 2] & 0xf : f[1 + nib / 2] >> 4;
			if (v >= 8) v -= 16;
			int x = (v * scale * 2048 + 1024 + coef[idx * 2] * h1 + coef[idx * 2 + 1] * h2) >> 11;
			x = std::max(-32768, std::min(32767, x));
			h2 = h1;
			h1 = x;
			out[base + (std::size_t)s * ch + c] = (std::int16_t)x;
		}
	}
	return samples;
}
}
