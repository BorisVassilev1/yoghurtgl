#pragma once

/// COPY PASTA FROM https://github.com/poseidon4o/urban-spork
/// I have made no changes to this file, I own none of it.

/**
* @file timer.h
* @brief Time Management Classes
*/

#include <stdint.h>
#include <condition_variable>

#if __linux__ != 0 || defined(__EMSCRIPTEN__)
#include <time.h>

static uint64_t timer_nsec() {
#if defined(CLOCK_MONOTONIC_RAW)
	const clockid_t clockid = CLOCK_MONOTONIC_RAW;

#else
	const clockid_t clockid = CLOCK_MONOTONIC;

#endif

	timespec t;
	clock_gettime(clockid, &t);

	return t.tv_sec * 1000000000UL + t.tv_nsec;
}

#elif _WIN64 != 0 || _WIN32 != 0
#define NOMINMAX
#include <Windows.h>

static struct TimerBase {
	LARGE_INTEGER freq;
	TimerBase() {
		QueryPerformanceFrequency(&freq);
	}
} timerBase;

// the order of global initialisaitons is non-deterministic, do
// not use this routine in the ctors of globally-scoped objects
static uint64_t timer_nsec() {
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);

	return 1000000000ULL * t.QuadPart / timerBase.freq.QuadPart;
}

#elif __APPLE__ != 0
#include <mach/mach_time.h>

static struct TimerBase {
	mach_timebase_info_data_t tb;
	TimerBase() {
		mach_timebase_info(&tb);
	}
} timerBase;

// the order of global initialisaitons is non-deterministic, do
// not use this routine in the ctors of globally-scoped objects
static uint64_t timer_nsec() {
	const uint64_t t = mach_absolute_time();
	return t * timerBase.tb.numer / timerBase.tb.denom;
}

#endif

struct Timer {
	Timer()
		: start(timer_nsec()) {}

	template <typename T>
	static T toMs(T ns) {
		return T(ns / 1e6);
	}

	int64_t elapsedNs() const {
		const uint64_t now = timer_nsec();
		return now - start;
	}

	uint64_t start;
};

#include <cassert>

#define ASSERT_ALWAYS

#if defined(_MSC_VER)

#if defined(_DEBUG) || defined(ASSERT_ALWAYS)
#define MASSERT_ENABLED
#define mAssert(test) ( !!(test) ? (void)0 : __debugbreak() );
#else
#define mAssert(test) (void)0;
#endif
#else

#define mAssert(test) assert(test)

#endif
