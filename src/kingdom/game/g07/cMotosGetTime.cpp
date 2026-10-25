#include "kingdom/game/g07/cMotosGetTime.hpp"

#include <chrono>

namespace kingdom::game::g07
{

uint cMotosGetTime::GetTime()
{
	// C# DateTime.Now.Ticks (100 ns units): the elapsed ticks since the previous call
	const long now = (long)std::chrono::duration_cast<std::chrono::duration<long long, std::ratio<1, 10000000>>>(std::chrono::system_clock::now().time_since_epoch()).count();
	m_nNowTime = now;   // (as in the C#: the second read is taken right after the first, the result is ~0; callers ignore it)
	const long now2 = (long)std::chrono::duration_cast<std::chrono::duration<long long, std::ratio<1, 10000000>>>(std::chrono::system_clock::now().time_since_epoch()).count();
	return (uint)(now2 - m_nNowTime);
}

uint cMotosGetTime::GetMilliSeconds()
{
	return (uint)(Time::deltaTime * 1000.0f);
}

}
