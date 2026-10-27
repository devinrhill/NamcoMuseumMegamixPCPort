#include "gotch/GameState.hpp"

namespace gotch
{
// RSO2 0x80d87e28 (the ctor calls InitPlayerState directly, InitSupportList through the vtable)
GameState::GameState()
{
	GameState::InitPlayerState();
	GameState::InitSupportList();
}

// RSO2 0x80d87f24
void GameState::Reset()
{
	oldState = 1;
	state = 1;
	stageNo = -1;
	onPlayField = false;
	unk1c = 0;
	InitPlayerState();
	InitSupportList();
}

// RSO2 0x80d87f90
void GameState::InitPlayerState()
{
	for (std::int32_t& s : playerState) s = 0;
}

// RSO2 0x80d88018
void GameState::InitSupportList()
{
	for (int i = 0; i < kSupport; i++) support[i] = i < 5 ? 1 : 0;
}

// RSO2 0x80d88048
void GameState::ChangeState(int s)
{
	if (state == s) return;
	if (support[s] == 0) return;
	oldState = state;
	state = s;
	onPlayField = false;
}
}
