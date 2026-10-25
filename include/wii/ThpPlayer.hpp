#pragma once

#include <deque>
#include <string>
#include <vector>

#include "gfx/Gfx.hpp"
#include "wii/Thp.hpp"

namespace wii
{
// The SDK's THPPlayer as the game uses it (DOL 0x800c34f0..0x800c4bb8, state byte at 0x800016e0+0xa4):
//   Open (THPPlayerOpen 0x800c3670) -> Prepare (0x800c3cf4, state PREPARED) -> Play (0x800c3fa4: PREPARED/PAUSED -> PLAYING)
//   Pause (0x800c40e0: PLAYING -> PAUSED) -> ... -> DONE when the last frame has been shown (state 3, tested by 0x800c4bb8).
// The Wii decodes on threads and draws through GX (0x800c44f8); here Update() decodes the due frame (JPEG via the gfx backend), feeds the
// audio device and Draw() puts the frame on the 640x480 screen. Time base: the audio clock when a device is open, else game frames at
// 59.94 Hz (UNVERIFIED: the SDK runs the video on the VI retrace, with the audio as master).
class ThpPlayer
{
public:
	enum State { STOP = 0, PREPARED = 1, PLAYING = 2, DONE = 3, PAUSED = 4 };

	~ThpPlayer() { Close(); }

	bool Open(const std::string& path);   // THPPlayerOpen
	bool Prepare();                       // THPPlayerPrepare(frame 0): audio device + first frames
	bool Play();                          // THPPlayerPlay: true if the state changed
	bool Pause();                         // THPPlayerPause
	void Update();                        // once per game frame
	void Draw() const;                    // THPPlayerDrawCurrentFrame over the whole screen
	void Close();

	State state() const { return m_state; }
	bool done() const { return m_state == DONE; }
	int currentFrame() const { return m_shown; }
	const Thp& file() const { return m_thp; }
	// seconds of movie time shown so far (testing / logging)
	double time() const { return m_time; }

private:
	void Fill();                          // read ahead: queue frames and push their audio until ~0.5 s are buffered
	void ShowDue();

	struct Queued { int index; std::vector<std::uint8_t> video; };

	Thp m_thp;
	State m_state{STOP};
	std::deque<Queued> m_queue;
	bool m_eof{false};
	bool m_audio{false};
	gfx::TexId m_tex{gfx::NO_TEX};
	int m_texW{}, m_texH{};
	int m_shown{-1};
	double m_time{0};
	double m_audioPushed{0};              // seconds of audio sent to the device
	std::vector<std::int16_t> m_pcm;
};
}
