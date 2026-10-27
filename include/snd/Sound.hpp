#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Sound sources of the NW4R sound system: streamed music (RSTM = .brstm) and sound effects (waves of a sound archive, RSAR = .brsar).
// Formats: NintendoWare/Revolution/Library/include/nw4r/snd/{StrmFile,WaveFile,WsdFile,SoundArchiveFile}.h.
namespace snd
{
// Something the mixer pulls interleaved 16-bit frames from.
class Source
{
public:
	virtual ~Source() = default;
	virtual int channels() const = 0;
	virtual int sampleRate() const = 0;
	// Writes up to `frames` interleaved frames; returns how many were produced (0 = finished).
	virtual int Read(std::int16_t* out, int frames) = 0;
};

// ---- RSTM stream ------------------------------------------------------------------------------------------------------------
class StrmSource : public Source
{
public:
	bool Open(const std::string& path, std::uint32_t startSample = 0);
	int channels() const override { return m_channels; }
	int sampleRate() const override { return m_rate; }
	int Read(std::int16_t* out, int frames) override;
	bool loops() const { return m_loop; }
	std::uint32_t totalSamples() const { return m_loopEnd; }

private:
	void SeekSample(std::uint32_t sample, bool useLoopHistory);

	std::vector<std::uint8_t> m_file;
	int m_format{}, m_channels{}, m_rate{};
	bool m_loop{false};
	std::uint32_t m_loopStart{}, m_loopEnd{}, m_dataOffset{}, m_numBlocks{}, m_blockSize{}, m_blockSamples{}, m_lastBlockSize{}, m_lastBlockSamples{},
	    m_lastBlockPadded{};
	struct Ch
	{
		std::int16_t coef[16]{};
		int h1{}, h2{};
		std::int16_t loopH1{}, loopH2{};
		std::int16_t buf[14]{};
		std::uint32_t frameByte{};   // file offset of the next DSP frame of this channel
		std::uint32_t blockLeft{};   // DSP frames left in the current block
	};
	std::vector<Ch> m_ch;
	std::uint32_t m_pos{};           // sample position
	int m_inFrame{14};               // next sample in buf (14 = decode a new frame)
	std::uint32_t m_block{};
};

// ---- wave (sound effect) ------------------------------------------------------------------------------------------------------
struct Wave
{
	std::vector<std::int16_t> pcm;   // interleaved
	int channels{1};
	int rate{32000};
	bool loop{false};
	std::uint32_t loopStart{}, loopEnd{};
	float volume{1.0f};              // note volume (0..1)
	float pitch{1.0f};               // playback rate multiplier
};

class WaveSource : public Source
{
public:
	explicit WaveSource(std::shared_ptr<const Wave> w) : m_wave(std::move(w)) {}
	int channels() const override { return m_wave->channels; }
	int sampleRate() const override { return (int)(m_wave->rate * m_wave->pitch); }
	int Read(std::int16_t* out, int frames) override;

private:
	std::shared_ptr<const Wave> m_wave;
	std::uint32_t m_pos{};
};

// ---- sound archive -----------------------------------------------------------------------------------------------------------
struct SoundDef
{
	std::string name;
	int id{-1};
	int type{0};                     // 1 sequence, 2 stream, 3 wave
	int fileId{-1};
	int playerId{-1};
	float volume{1.0f};              // SoundCommonInfo volume / 127
	std::string extPath;             // streams: "stream/NAME.brstm" relative to the sound folder
	std::uint32_t startPosition{};   // streams
	int subNo{-1};                   // waves: index of the wave sound in its RWSD
	// SoundCommonInfo param3dRef (nw4r SoundArchiveFile::Sound3DParam): FLAG_NOT_CTRL_VOLUME 1, FLAG_NOT_CTRL_PAN 2; decay curve 1 log, 2 linear
	std::uint32_t flags3d{};
	int decayCurve3d{1};
	float decayRatio3d{0.5f};        // decayRatio / 256
};

class Archive
{
public:
	bool Load(const std::string& path);
	bool loaded() const { return !m_d.empty(); }
	const SoundDef* Find(const std::string& label) const;
	std::size_t count() const { return m_sounds.size(); }
	const std::vector<SoundDef>& sounds() const { return m_sounds; }
	// Decodes the wave of a wave sound (cached by the caller).
	std::shared_ptr<Wave> LoadWave(const SoundDef& s) const;

private:
	struct Ref { int type, dtype; std::uint32_t value; };
	std::uint32_t U32(std::size_t o) const;
	std::uint16_t U16(std::size_t o) const;
	Ref R(std::size_t o) const;

	std::vector<std::uint8_t> m_d;
	std::size_t m_info{};            // base of the INFO block's Info struct
	std::vector<SoundDef> m_sounds;
	std::vector<std::pair<std::string, int>> m_index;   // name -> sound id (sorted)
};
}
