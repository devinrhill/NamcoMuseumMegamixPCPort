#pragma once

#include <cstdint>

// GAMESTATE (nmwii2.rso, RSO2 0x80d87e28..0x80d88118, 0xa0 bytes): what a game reports about itself (demo / playing / ..., the stage, every
// player's state) to the frontend. PACMAN only writes it; RALLYX::VBlankIn_Extend (RSO2 0x80d75220) reads GetState. The name tables
// GameStateTable / PlayerStateTable (GetStateString 0x80d88094 / GetPlayerStateString 0x80d88104) are module data: not ported.
namespace gotch
{
class GameState
{
public:
	static constexpr int kSupport = 9;    // +0x11 .. +0x19
	static constexpr int kPlayers = 32;   // +0x20 .. +0x9c

	GameState();                                    // RSO2 0x80d87e28
	virtual ~GameState() = default;                 // RSO2 0x80d87ea0
	void Reset();                                   // RSO2 0x80d87f24
	virtual void InitPlayerState();                 // RSO2 0x80d87f90 (vtable +0x8)
	virtual void InitSupportList();                 // RSO2 0x80d88018 (vtable +0xc): states 0..4 allowed, 5..8 not
	void ChangeState(int state);                    // RSO2 0x80d88048: ignored when unchanged or not in the support list; leaves the play field
	int GetState() const { return state; }          // RSO2 0x80d88084
	int GetOldState() const { return oldState; }    // RSO2 0x80d8808c
	int GetStageNo() const { return stageNo; }      // RSO2 0x80d880a8
	void SetStageNo(int n) { stageNo = n; }         // RSO2 0x80d880b0
	bool GetOnPlayField() const { return onPlayField; }            // RSO2 0x80d880b8
	void SetOnPlayField(bool on) { onPlayField = on; }             // RSO2 0x80d880c0
	const std::uint8_t* GetGameStateSupportList() const { return support; }              // RSO2 0x80d880d4
	void SetGameStateSupportList(int state, std::uint8_t on) { support[state] = on; }   // RSO2 0x80d880dc
	void SetPlayerState(int player, int s) { playerState[player] = s; }                 // RSO2 0x80d880e8
	int GetPlayerState(int player) const { return playerState[player]; }
	void SetField1c(int v) { unk1c = v; }   // +0x1c: RALLYX's hooks store the player count here (never read)

private:
	int state = 1;                     // +0x04
	int oldState = 1;                  // +0x08
	int stageNo = -1;                  // +0x0c
	bool onPlayField = false;          // +0x10
	std::uint8_t support[kSupport]{};  // +0x11
	std::int32_t unk1c = 0;            // +0x1c (cleared by the ctor / Reset, never read)
	std::int32_t playerState[kPlayers]{};   // +0x20
};
}
