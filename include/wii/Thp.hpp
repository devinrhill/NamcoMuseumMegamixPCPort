#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// THP movie reader (Nintendo's video format: a JPEG per frame plus DSP-ADPCM audio). Layout from RVL_SDK/include/revolution/thpfile.h:
//   THPHeader (0x30 bytes, big endian) -> component info -> frames; every frame = {u32 sizeNext, u32 sizePrevious, u32 compSize[n]}
//   then the components in order (video JPEG, audio block), padded to 32 bytes. This file has no frame offset table (offsetData = 0),
//   so frames are read sequentially; the player (DOL 0x800c3cf4 THPPlayerPrepare family) starts at frame 0.
// Audio block: {u32 channelBytes, u32 numSamples, s16 coef[ch][16], s16 yn1/yn2 per channel, then channelBytes of DSP-ADPCM per channel}.
namespace wii
{
class Thp
{
public:
	struct Frame
	{
		int index{};
		std::vector<std::uint8_t> video;   // one baseline JPEG
		std::vector<std::uint8_t> audio;   // one audio block (empty when the file has no audio)
	};

	bool Open(const std::string& path);
	void Close();
	bool IsOpen() const { return m_file.is_open(); }

	int width() const { return m_width; }
	int height() const { return m_height; }
	float frameRate() const { return m_frameRate; }
	int numFrames() const { return m_numFrames; }
	bool hasAudio() const { return m_audioChannels > 0; }
	int audioChannels() const { return m_audioChannels; }
	int audioRate() const { return m_audioRate; }

	// Reads the next frame; false at the end of the movie.
	bool ReadFrame(Frame& out);
	void Rewind();
	int nextFrame() const { return m_next; }

	// THP video frames are baseline JPEGs whose entropy-coded data has no byte stuffing (an 0xFF byte is not followed by 0x00).
	// Returns a standard JPEG: header up to the SOS segment unchanged, scan data stuffed, EOI appended.
	static std::vector<std::uint8_t> ToStandardJpeg(const std::vector<std::uint8_t>& thpFrame);

	// Decodes one audio block to interleaved 16-bit PCM (appended to `out`); returns the number of sample frames.
	int DecodeAudio(const std::vector<std::uint8_t>& block, std::vector<std::int16_t>& out) const;

private:
	std::ifstream m_file;
	int m_width{}, m_height{}, m_numFrames{}, m_audioChannels{}, m_audioRate{};
	float m_frameRate{30};
	std::uint32_t m_movieOffset{}, m_firstFrameSize{};
	std::vector<std::uint8_t> m_compKinds;
	int m_next{};
	std::uint32_t m_nextSize{};
	std::uint64_t m_pos{};
};
}
