// ==========================
// common/progressdisplay.cpp
// ==========================

#include "progressdisplay.h"

#include "stringutil.h"
#if XXX_GAME
#include "Utility/timer.h"
#endif

ProgressDisplay::ProgressDisplay(const char* format, ...)
{
	memset(this, 0, sizeof(*this));
	GET_STR_VARARGS(temp,1024,format);
	sprintf(m_string, "%s .. ", temp);
	if (!sm_silent) {
		fprintf(stdout, "%s", m_string);
		fflush(stdout);
	}
	m_startTime = GetCurrentPerformanceTime();
	Update(0.0f);
}

ProgressDisplay::~ProgressDisplay()
{
	DEBUG_ASSERT(m_startTime == 0);
}

float ProgressDisplay::GetTimeInSeconds(uint64 startTime)
{
	const uint64 dt = GetCurrentPerformanceTime() - startTime;
	return (float)((double)dt/(double)GetTimeFrequency());
}

float ProgressDisplay::GetDeltaTimeInSeconds(uint64& prevTime)
{
	const uint64 currTime = GetCurrentPerformanceTime();
	const uint64 dt = currTime - prevTime;
	prevTime = currTime;
	return (float)((double)dt/(double)GetTimeFrequency());
}

void ProgressDisplay::Update(float f, const char* format, ...)
{
	m_progress = f;
	if (sm_showRunningTime && !sm_silent) {
		char temp[128] = "";
		sprintf(temp, "%.3f%%", 100.0f*m_progress);
		if (format) {
			GET_STR_VARARGS(temp2,128,format);
			strcat(temp, " ");
			strcat(temp, temp2);
		}
		Erase(m_numChars);
		fprintf(stdout, "%s", temp);
		fflush(stdout);
		m_numChars = (int)strlen(temp);
		if (sm_showRemainingTime) { // show remaining time
			const uint64 time = GetCurrentPerformanceTime();
			const float elapsed = GetTimeInSeconds();
			if (elapsed > 5.0f && m_progress > 0.0f) {
				if (time - m_remainingTime > GetTimeFrequency()) {
					const float secs = elapsed*(1.0f - m_progress)/m_progress;
					const float mins = secs/60.0f;
					const float hrs  = mins/60.0f;
					if      (secs <= 100.0f) { sprintf(m_remainingTimeStr, " (%.2f secs remaining)", secs); }
					else if (mins <= 100.0f) { sprintf(m_remainingTimeStr, " (%.2f mins remaining)", mins); }
					else                     { sprintf(m_remainingTimeStr, " (%.2f hours remaining)", hrs); }
					m_remainingTime = time;
				}
				fprintf(stdout, "%s", m_remainingTimeStr);
				fflush(stdout);
				m_numChars += (int)strlen(m_remainingTimeStr);
			}
		}
	}
}

float ProgressDisplay::End(const char* format, ...)
{
	const float secs = GetTimeInSeconds();
	if (!sm_silent) {
		Erase(m_numChars);
		GET_STR_VARARGS(temp,1024,format);
		fprintf(stdout, "%s.", temp);
		if (sm_showElapsedTime) {
			char totalStr[64] = "";
			if (sm_showTotalTime) {
				static float totalSecs = 0.0f;
				totalSecs += secs;
				const float totalMins = totalSecs/60.0f;
				const float totalHrs  = totalMins/60.0f;
				if      (totalSecs <=   1.0f) { sprintf(totalStr, ""); }
				else if (totalSecs <= 100.0f) { sprintf(totalStr, ", total %.2f secs", totalSecs); }
				else if (totalMins <= 100.0f) { sprintf(totalStr, ", total %.2f mins", totalMins); }
				else                          { sprintf(totalStr, ", total %.2f hours", totalHrs); }
			}
			const float mins = secs/60.0f;
			const float hrs  = mins/60.0f;
			if      (secs <=   1.0f) { fprintf(stdout, " (%.6f msecs%s)", secs*1000.0f, totalStr); }
			else if (secs <= 100.0f) { fprintf(stdout, " (%.6f secs%s)", secs, totalStr); }
			else if (mins <= 100.0f) { fprintf(stdout, " (%.6f mins%s)", mins, totalStr); }
			else                     { fprintf(stdout, " (%.6f hours%s)", hrs, totalStr); }
		}
		fprintf(stdout, "                             \n"); // extra space to clear leftover chars
	}
	memset(this, 0, sizeof(*this));
	return secs;
}

uint64 ProgressDisplay::GetCurrentPerformanceTime()
{
#if PLATFORM_PC
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return t.QuadPart;
#elif PLATFORM_PS4
	return TimerBase::GetSysCountReal();
#endif
}

uint64 ProgressDisplay::GetTimeFrequency()
{
#if PLATFORM_PC
	LARGE_INTEGER f;
	QueryPerformanceFrequency(&f);
	return f.QuadPart;
#elif PLATFORM_PS4
	return TimerBase::GetSysCounterFrequency();
#endif
}

void ProgressDisplay::Restart(const char* format, ...)
{
	if (m_startTime)
		End();
	memset(this, 0, sizeof(*this));
	GET_STR_VARARGS(temp,1024,format);
	sprintf(m_string, "%s .. ", temp);
	if (!sm_silent) {
		fprintf(stdout, "%s", m_string);
		fflush(stdout);
	}
	m_startTime = GetCurrentPerformanceTime();
	Update(0.0f);
}

void ProgressDisplay::Erase(int numChars)
{
	for (int k = 0; k < numChars; k++)
		fprintf(stdout, "%c", 0x08);
	fflush(stdout);
}

bool ProgressDisplay::sm_silent = false;
bool ProgressDisplay::sm_showRunningTime = PLATFORM_PC ? true : false;
bool ProgressDisplay::sm_showRemainingTime = PLATFORM_PC ? true : false;
bool ProgressDisplay::sm_showElapsedTime = true;
bool ProgressDisplay::sm_showTotalTime = true;