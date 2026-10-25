#include "wii/ThpPlayer.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "WiiSystem.hpp"
#include "audio/PcmOut.hpp"

namespace wii
{
namespace
{
constexpr double kVideoHz = 60.0 * 1000.0 / 1001.0; // the console's frame rate: one game frame per VI retrace
constexpr double kReadAhead = 0.5;                  // seconds of movie kept queued
}

bool ThpPlayer::Open(const std::string& path)
{
	Close();
	if (!m_thp.Open(path)) return false;
	m_state = STOP;
	return true;
}

bool ThpPlayer::Prepare()
{
	if (!m_thp.IsOpen() || m_state != STOP) return false;
	m_thp.Rewind();
	m_queue.clear();
	m_eof = false;
	m_shown = -1;
	m_time = 0;
	m_audioPushed = 0;
	m_audio = m_thp.hasAudio() && audio::Open(m_thp.audioRate(), m_thp.audioChannels());
	if (m_audio) { audio::SetVolume(WiiSystem::Volume()); audio::SetPaused(true); } // starts with Play
	Fill();
	m_state = PREPARED;
	return true;
}

bool ThpPlayer::Play()
{
	if (m_state != PREPARED && m_state != PAUSED) return false;
	if (m_audio) audio::SetPaused(false);
	m_state = PLAYING;
	return true;
}

bool ThpPlayer::Pause()
{
	if (m_state != PLAYING) return false;
	if (m_audio) audio::SetPaused(true);
	m_state = PAUSED;
	return true;
}

void ThpPlayer::Close()
{
	if (m_audio) audio::Close();
	m_audio = false;
	if (m_tex != gfx::NO_TEX) { gfx::DestroyTexture(m_tex); m_tex = gfx::NO_TEX; }
	m_thp.Close();
	m_queue.clear();
	m_state = STOP;
}

void ThpPlayer::Fill()
{
	const double fps = m_thp.frameRate() > 0 ? m_thp.frameRate() : 30.0;
	// keep `kReadAhead` seconds between the frame being shown and the last frame queued
	while (!m_eof)
	{
		const int last = m_queue.empty() ? std::max(m_shown, 0) : m_queue.back().index;
		if ((last - std::max(m_shown, 0)) / fps >= kReadAhead && !m_queue.empty()) break;
		Thp::Frame f;
		if (!m_thp.ReadFrame(f)) { m_eof = true; break; }
		if (m_audio && !f.audio.empty())
		{
			m_pcm.clear();
			const int n = m_thp.DecodeAudio(f.audio, m_pcm);
			if (n > 0) { audio::Push(m_pcm.data(), (std::size_t)n); m_audioPushed += (double)n / m_thp.audioRate(); }
		}
		m_queue.push_back({f.index, std::move(f.video)});
	}
}

void ThpPlayer::ShowDue()
{
	const double fps = m_thp.frameRate() > 0 ? m_thp.frameRate() : 30.0;
	const int due = (int)(m_time * fps);
	// every queued frame up to the due one is consumed; only the newest is decoded (the others are skipped if the clock ran ahead)
	std::vector<std::uint8_t> newest;
	bool have = false;
	while (!m_queue.empty() && m_queue.front().index <= due)
	{
		m_shown = m_queue.front().index;
		newest = std::move(m_queue.front().video);
		have = true;
		m_queue.pop_front();
	}
	if (!have) return;
	int w = 0, h = 0;
	std::vector<std::uint8_t> rgba;
	const std::vector<std::uint8_t> jpeg = Thp::ToStandardJpeg(newest);
	if (!gfx::DecodeJpeg(jpeg.data(), jpeg.size(), w, h, rgba)) return;
	if (m_tex == gfx::NO_TEX || w != m_texW || h != m_texH)
	{
		if (m_tex != gfx::NO_TEX) gfx::DestroyTexture(m_tex);
		m_tex = gfx::CreateTexture(w, h, rgba.data());
		m_texW = w;
		m_texH = h;
	}
	else gfx::UpdateTexture(m_tex, w, h, rgba.data());
}

void ThpPlayer::Update()
{
	if (m_state != PLAYING) return;
	if (m_audio && audio::IsOpen()) m_time = audio::PlayedSeconds();
	else m_time += 1.0 / kVideoHz;
	Fill();
	ShowDue();
	const double fps = m_thp.frameRate() > 0 ? m_thp.frameRate() : 30.0;
	if (m_eof && m_queue.empty() && m_time >= m_thp.numFrames() / fps) m_state = DONE;
}

void ThpPlayer::Draw() const
{
	if (m_tex == gfx::NO_TEX) return;
	// the whole 640 x 480 game screen: corners TL, BL, TR, BR in the y-up centre-origin space, v from the top
	const float xy[8] = {-320, 240, -320, -240, 320, 240, 320, -240};
	const float uv[8] = {0, 0, 0, 1, 1, 0, 1, 1};
	gfx::DrawQuad(m_tex, xy, uv, 255, 255, 255, 255, gfx::BLEND_ALPHA, true);
}
}
