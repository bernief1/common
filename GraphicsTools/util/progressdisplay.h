// ========================
// common/progressdisplay.h
// ========================

#ifndef _INCLUDE_COMMON_PROGRESSDISPLAY_H_
#define _INCLUDE_COMMON_PROGRESSDISPLAY_H_

#include "common/common.h"

class ProgressDisplay
{
public:
	ProgressDisplay() : m_progress(0.0f), m_numChars(0), m_startTime(0), m_remainingTime(0) {}
	ProgressDisplay(const char* format, ...);
	~ProgressDisplay();

	inline float GetTimeInSeconds() const { return GetTimeInSeconds(m_startTime); };
	void Update(float f, const char* format = NULL, ...);
	inline void Update(size_t i, size_t n) { Update((float)i/(float)n); }
	float End(const char* format = "done", ...);
	void Restart(const char* format, ...);

	static uint64 GetCurrentPerformanceTime();
	static uint64 GetTimeFrequency();
	static float GetTimeInSeconds(uint64 startTime);
	static float GetDeltaTimeInSeconds(uint64& prevTime);

	static bool SetSilent(bool silent) { const bool was = sm_silent; sm_silent = silent; return was; }

protected:
	static void Erase(int numChars);

	char   m_string[512];
	float  m_progress;
	int    m_numChars;
	uint64 m_startTime;
	uint64 m_remainingTime;
	char   m_remainingTimeStr[64];

public:
	static bool sm_silent;
	static bool sm_showRunningTime;
	static bool sm_showRemainingTime;
	static bool sm_showElapsedTime;
	static bool sm_showTotalTime;
};

#endif // _INCLUDE_COMMON_PROGRESSDISPLAY_H_