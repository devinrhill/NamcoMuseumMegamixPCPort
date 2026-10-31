#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "snd/Dsp.hpp"
#include "snd/Sound.hpp"

namespace snd
{
std::uint32_t Archive::U32(std::size_t o) const
{
	if (o + 4 > m_d.size()) return 0;
	return ((std::uint32_t)m_d[o] << 24) | (m_d[o + 1] << 16) | (m_d[o + 2] << 8) | m_d[o + 3];
}
std::uint16_t Archive::U16(std::size_t o) const
{
	if (o + 2 > m_d.size()) return 0;
	return (std::uint16_t)((m_d[o] << 8) | m_d[o + 1]);
}
Archive::Ref Archive::R(std::size_t o) const { return {m_d[o], m_d[o + 1], U32(o + 4)}; }

// RSAR: header {sig, bom, version, size, headerSize, blocks, symbOff, symbSize, infoOff, infoSize, fileOff, fileSize}.
// SYMB: string table + patricia trees (we only need the leaves of the sound tree: label -> sound id).
// INFO: Info = 6 data refs (sounds, banks, players, files, groups, player info), all offsets relative to the Info struct.
bool Archive::Load(const std::string& path)
{
	std::ifstream f(path, std::ios::binary | std::ios::ate);
	if (!f) { std::fprintf(stderr, "snd::Archive: cannot open %s\n", path.c_str()); return false; }
	m_d.resize((std::size_t)f.tellg());
	f.seekg(0);
	f.read((char*)m_d.data(), m_d.size());
	if (m_d.size() < 0x40 || std::memcmp(m_d.data(), "RSAR", 4) != 0) { std::fprintf(stderr, "snd::Archive: %s is not an RSAR\n", path.c_str()); m_d.clear(); return false; }
	const std::size_t symb = U32(16), info = U32(24);
	const std::size_t sb = symb + 8;
	const std::size_t tab = U32(sb), soundTree = U32(sb + 4);
	auto str = [&](std::uint32_t idx) {
		std::size_t o = sb + U32(sb + tab + 4 + 4 * idx);
		std::string s;
		while (o < m_d.size() && m_d[o]) s += (char)m_d[o++];
		return s;
	};
	std::vector<std::pair<std::string, int>> names;
	const std::uint32_t nodes = U32(sb + soundTree + 4);
	for (std::uint32_t i = 0; i < nodes; i++)
	{
		const std::size_t o = sb + soundTree + 8 + 20 * i;
		if (U16(o) & 1) names.emplace_back(str(U32(o + 12)), (int)U32(o + 16)); // leaf: string index, sound id
	}

	m_info = info + 8;
	const std::size_t soundTable = m_info + R(m_info).value;
	const std::uint32_t n = U32(soundTable);
	m_sounds.assign(n, SoundDef{});
	const std::size_t fileTable = m_info + R(m_info + 24).value;
	for (std::uint32_t i = 0; i < n; i++)
	{
		SoundDef& s = m_sounds[i];
		const std::size_t o = m_info + R(soundTable + 4 + 8 * i).value;
		s.id = (int)i;
		s.fileId = (int)U32(o + 4);
		s.playerId = (int)U32(o + 8);
		s.volume = m_d[o + 20] / 127.0f;
		const Ref p3d = R(o + 12);
		if (p3d.value)
		{
			const std::size_t q = m_info + p3d.value;
			s.flags3d = U32(q);
			s.decayCurve3d = m_d[q + 4];
			s.decayRatio3d = m_d[q + 5] / 256.0f;
		}
		s.type = m_d[o + 22];
		const std::size_t sinfo = m_info + R(o + 24).value;
		if (s.type == 2)
		{
			s.startPosition = U32(sinfo);
			const std::size_t fi = m_info + R(fileTable + 4 + 8 * s.fileId).value;
			const Ref ext = R(fi + 12);
			if (ext.value)
			{
				std::size_t p = m_info + ext.value;
				while (p < m_d.size() && m_d[p]) s.extPath += (char)m_d[p++];
			}
		}
		else if (s.type == 3) s.subNo = (int)U32(sinfo);
	}
	for (const auto& nm : names)
		if (nm.second >= 0 && nm.second < (int)n) m_sounds[nm.second].name = nm.first;
	m_index = std::move(names);
	std::sort(m_index.begin(), m_index.end());
	return true;
}

const SoundDef* Archive::Find(const std::string& label) const
{
	auto it = std::lower_bound(m_index.begin(), m_index.end(), std::make_pair(label, -1));
	if (it == m_index.end() || it->first != label) return nullptr;
	return &m_sounds[it->second];
}

// A wave sound: its file (RWSD) sits in a group of the archive's file image, the wave data (an RWAR wave archive) next to it.
//   RWSD DATA block -> Wsd[subNo] -> note table -> note 0 -> wave index -> RWAR TABL entry -> RWAV (INFO + DATA) -> DSP-ADPCM.
std::shared_ptr<Wave> Archive::LoadWave(const SoundDef& s) const
{
	if (s.type != 3 || s.fileId < 0) return nullptr;
	const std::size_t fileTable = m_info + R(m_info + 24).value;
	const std::size_t groupTable = m_info + R(m_info + 32).value;
	const std::size_t fi = m_info + R(fileTable + 4 + 8 * s.fileId).value;
	const Ref posRef = R(fi + 20);
	if (!posRef.value) return nullptr;
	const std::size_t posTable = m_info + posRef.value;
	if (U32(posTable) == 0) return nullptr;
	const std::size_t pos = m_info + R(posTable + 4).value;
	const std::uint32_t groupId = U32(pos), index = U32(pos + 4);
	const std::size_t g = m_info + R(groupTable + 4 + 8 * groupId).value;
	const std::uint32_t gOff = U32(g + 16), gWaveOff = U32(g + 24);
	const std::size_t items = m_info + R(g + 32).value;
	const std::size_t item = m_info + R(items + 4 + 8 * index).value;
	const std::size_t rwsd = gOff + U32(item + 4), rwar = gWaveOff + U32(item + 12);
	if (rwsd + 32 > m_d.size() || std::memcmp(&m_d[rwsd], "RWSD", 4) != 0) return nullptr;
	if (U16(rwsd + 6) < 0x103) { std::fprintf(stderr, "snd::Archive: RWSD version %x not supported\n", U16(rwsd + 6)); return nullptr; }

	const std::size_t dataBlock = rwsd + U32(rwsd + 16), base = dataBlock + 8;
	if (s.subNo < 0 || (std::uint32_t)s.subNo >= U32(dataBlock + 8)) return nullptr;
	const std::size_t wsd = base + R(dataBlock + 12 + 8 * s.subNo).value;
	const std::size_t wsdInfo = base + R(wsd).value;
	const std::size_t noteTable = base + R(wsd + 16).value;
	if (U32(noteTable) == 0) return nullptr;
	const std::size_t note = base + R(noteTable + 4).value;
	const int waveIndex = (int)U32(note);
	auto f32 = [&](std::size_t o) { std::uint32_t v = U32(o); float x; std::memcpy(&x, &v, 4); return x; };

	if (rwar + 32 > m_d.size() || std::memcmp(&m_d[rwar], "RWAR", 4) != 0) return nullptr;
	const std::size_t tabl = rwar + U32(rwar + 16), rwarData = rwar + U32(rwar + 24);
	if (waveIndex < 0 || (std::uint32_t)waveIndex >= U32(tabl + 8)) return nullptr;
	const std::size_t rwav = rwarData + U32(tabl + 12 + 12 * waveIndex + 4);
	if (rwav + 32 > m_d.size() || std::memcmp(&m_d[rwav], "RWAV", 4) != 0) return nullptr;

	const std::size_t wi = rwav + U32(rwav + 16) + 8;
	const std::size_t dataBlk = rwav + U32(rwav + 24);
	auto w = std::make_shared<Wave>();
	const int format = m_d[wi];
	w->loop = m_d[wi + 1] != 0;
	w->channels = std::max<int>(1, std::min<int>(m_d[wi + 2], 2));
	w->rate = U16(wi + 4);
	w->loopStart = U32(wi + 8);
	w->loopEnd = U32(wi + 12);
	const std::uint32_t chInfoOff = U32(wi + 16);
	if (format != 2) { std::fprintf(stderr, "snd::Archive: wave of '%s' has format %d (only ADPCM handled)\n", s.name.c_str(), format); return nullptr; }
	const int srcCh = m_d[wi + 2];
	const std::uint32_t dataSize = U32(dataBlk + 4) - 8;
	std::vector<std::vector<std::int16_t>> chans(w->channels);
	for (int c = 0; c < w->channels; c++)
	{
		const std::size_t ci = wi + U32(wi + chInfoOff + 4 * c);
		const std::uint32_t dataOff = U32(ci);
		const std::size_t ap = wi + U32(ci + 4);
		std::int16_t coef[16];
		for (int i = 0; i < 16; i++) coef[i] = (std::int16_t)U16(ap + 2 * i);
		int h1 = (std::int16_t)U16(ap + 36), h2 = (std::int16_t)U16(ap + 38);
		const std::uint32_t perCh = dataSize / srcCh;
		const std::size_t at = dataBlk + 8 + dataOff;
		const std::uint32_t frames = perCh / 8;
		chans[c].resize((std::size_t)frames * 14);
		for (std::uint32_t k = 0; k < frames && at + (k + 1) * 8 <= m_d.size(); k++)
			DecodeDspFrame(&m_d[at + k * 8], coef, h1, h2, &chans[c][(std::size_t)k * 14]);
	}
	const std::size_t len = chans[0].size();
	w->pcm.resize(len * w->channels);
	for (std::size_t i = 0; i < len; i++)
		for (int c = 0; c < w->channels; c++) w->pcm[i * w->channels + c] = chans[c][i];
	w->attack = m_d[note + 4]; w->decay = m_d[note + 5]; w->sustain = m_d[note + 6]; w->release = m_d[note + 7]; w->hold = m_d[note + 8];
	w->originalKey = m_d[note + 12]; w->noteVolume = m_d[note + 13]; w->notePitch = f32(note + 16);
	w->wsdPitch = f32(wsdInfo); w->pan = m_d[wsdInfo + 4]; w->mainSend = m_d[wsdInfo + 9];
	w->volume = m_d[note + 13] / 127.0f;
	w->pitch = f32(note + 16) * f32(wsdInfo);
	if (w->pitch <= 0) w->pitch = 1.0f;
	if (!w->loop || w->loopEnd == 0 || w->loopEnd > len) w->loopEnd = (std::uint32_t)len;
	return w;
}
}
