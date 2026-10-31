#include "kingdom/game/g07/cMotosGetTime.hpp"

#include <chrono>

namespace kingdom::game::g07
{

namespace
{
long NowMicro()
{
	return (long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
}

// DOL 0x800a9520 [audited: the debug name argument is dropped]: stores OSGetTime (the stand-in is a microsecond clock), returns the difference to a second read (callers ignore it)
uint cMotosGetTime::GetTime()
{
	m_nNowTime = NowMicro();
	return (uint)(NowMicro() - m_nNowTime);
}

// DOL 0x800a9594 [audited]: OSGetTime - the time stored by the last GetTime, in milliseconds (ticks / (bus clock / 4 / 1000))
uint cMotosGetTime::GetMilliSeconds()
{
	return (uint)((NowMicro() - m_nNowTime) / 1000);
}

}
