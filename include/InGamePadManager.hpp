#pragma once

#include "common.hpp"
#include "PadDefine.hpp"

// Port of the C# InGamePadManager, backed by WiiPad instead of Unity/Steam input.
// padNum is a WPAD channel (0-3). `isEnableAnyPad` is accepted for source compatibility and ignored.
class InGamePadManager
{
public:
	// Call once per frame after WiiPad::Update(); snapshots the stick-direction codes for edge detection.
	static void Update();

	static bool get_Disable();
	static void set_Disable(bool value);

	static Vector2 GetStick(int padNum, int stickNum, bool isEnableAnyPad = true);
	static float GetLTrigger(int padNum);
	static float GetRTrigger(int padNum);

	static bool GetKey(int padNum, PadDefine::EPadKeycode key, bool isEnableAnyPad = true);
	static bool GetKeyDown(int padNum, PadDefine::EPadKeycode key, bool isEnableAnyPad = true);
	static bool GetKeyUp(int padNum, PadDefine::EPadKeycode key, bool isEnableAnyPad = true);
	static bool GetKeyRepeat(int padNum, PadDefine::EPadKeycode key, bool isEnableAnyPad = true);
	static bool GetKeyFlag(int padNum, EPadKeyFlag flag, bool isEnableAnyPad = true);
	static bool GetKeyDownFlag(int padNum, EPadKeyFlag flag, bool isEnableAnyPad = true);
	static bool GetKeyUpFlag(int padNum, EPadKeyFlag flag, bool isEnableAnyPad = true);
	static bool GetKeyRepeatFlag(int padNum, EPadKeyFlag flag, bool isEnableAnyPad = true);
	static bool GetEnterKeyDown(int padNum, bool isEnableAnyPad = true);
	static bool GetCancelKeyDown(int padNum, bool isEnableAnyPad = true);

	static bool IsConnectPad(int padNum);
	static bool IsConnectPad2(int padNum);
	static PadDefine::EPadStyle GetPadStyle(int padNum);
	static void SetForcedStop(bool isFlag);
	static bool GetForcedStop();

	static int GetPadNumFirst();
	static int GetPadNumSecond();
	static int GetPadNum(int playerIdx);
	static int GetPad1No();
	static int GetPad2No();
	static int GetPad3No();
	static int GetPad4No();
	static int GetCurrentPadNo();

	// Remap which Wii button a game pad code reads (e.g. SetKeycodeMapping(eKEY_CIRCLE, WiiPad::BTN_2)).
	static void SetKeycodeMapping(PadDefine::EPadKeycode key, unsigned wiiButtonMask);
};
