
#include <cwsdk.h>
#include <cstdio>

__declspec(noinline) void Howdy()
{
	CW_TRIGGER(PerformanceType::CRITICAL, TriggerExecution::WHEN_TAMPERED,
		0, { std::printf("Hello\n"); });

	std::puts("howdy sir!");
}

int main(int, char**)
{
	std::puts("hello!");
	Howdy();
	return 0;
}