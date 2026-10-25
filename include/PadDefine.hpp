#pragma once

#include "common.hpp"
#include "PadKeyFlag.hpp"

// Game-facing pad codes. The Unity port named these after a PlayStation-style layout; here they are
// logical buttons that InGamePadManager maps onto Wiimote / Nunchuk buttons (see InGamePadManager.cpp
// and input.cfg). Values match the C# enum so code written against it keeps working.
class PadDefine
{
public:
	enum EPadKeycode
	{
		eKEY_CROSS = 0,
		eKEY_CIRCLE = 1,
		eKEY_SQUARE = 2,
		eKEY_TRIANGLE = 3,
		eKEY_L1 = 4,
		eKEY_R1 = 5,
		eKEY_SELECT = 6,
		eKEY_START = 7,
		eKEY_L3 = 8,
		eKEY_R3 = 9,
		eKEY_UP = 10,
		eKEY_RIGHT = 11,
		eKEY_DOWN = 12,
		eKEY_LEFT = 13,
		eKEY_L2 = 14,
		eKEY_R2 = 15,
		eKEY_LSTICK_UP = 16,
		eKEY_LSTICK_RIGHT = 17,
		eKEY_LSTICK_DOWN = 18,
		eKEY_LSTICK_LEFT = 19,
		eKEY_RSTICK_UP = 20,
		eKEY_RSTICK_RIGHT = 21,
		eKEY_RSTICK_DOWN = 22,
		eKEY_RSTICK_LEFT = 23,
		eKEY_LEFT_SL = 24,
		eKEY_LEFT_SR = 25,
		eKEY_RIGHT_SL = 26,
		eKEY_RIGHT_SR = 27,
		eKEY_COUNT = 28
	};

	// What kind of controller is plugged into a channel.
	enum EPadStyle
	{
		eNONE = 0,
		eWIIMOTE = 1,
		eNUNCHUK = 2,
		eCLASSIC = 3,
		eINVALID = 4
	};

	static constexpr int scMAX_PAD_NUM = 4;
	static constexpr int scMAX_STICK_NUM = 2;
	static constexpr int scLSTICK = 0;
	static constexpr int scRSTICK = 1;

	static constexpr EPadKeycode scENTER_CODE = eKEY_CIRCLE;
	static constexpr EPadKeycode scCANCEL_CODE = eKEY_CROSS;
	static constexpr EPadKeyFlag scENTER_FLAG = EPadKeyFlag::eKEY_CIRCLE;
	static constexpr EPadKeyFlag scCENCEL_FLAG = EPadKeyFlag::eKEY_CROSS;
};
