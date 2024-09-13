#pragma once

#include <chrono>
#include <iostream>

enum class timeFormat : uint32_t
{
	nanoscnds = 1000000000, microscnds = 1000000, miliscnds = 1000, scnds = 1, mints = 60, hrs = 3600
};

// Calculate time duration from start point to end point, you can define time format of output(in seconds, miliseconds, etc..) and type of result(int, uint32_t, etc..)
template<typename DurationTimeFormat = uint32_t>
DurationTimeFormat tt_calcDuration(std::chrono::high_resolution_clock::time_point start, std::chrono::high_resolution_clock::time_point end, timeFormat timeFormat = timeFormat::miliscnds)
{

	auto duration = end - start;

	switch (timeFormat)
	{
	case timeFormat::nanoscnds:
		return (DurationTimeFormat)(duration).count();
	case timeFormat::microscnds:
		return (DurationTimeFormat)(duration).count() / 1000;
	case timeFormat::miliscnds:
		return (DurationTimeFormat)(duration).count() / 1000000;
	case timeFormat::scnds:
		return (DurationTimeFormat)(duration).count() / 1000000000;
	case timeFormat::mints:
		return (DurationTimeFormat)((duration).count() / 1000000000) / 60;
	case timeFormat::hrs:
		return (DurationTimeFormat)((duration).count() / 1000000000) / (60 * 60);
	default:
		break;
	}

	return (DurationTimeFormat)(0);

}

template<typename DefaultOutputTimeFormat = uint32_t, timeFormat def_ttTimeFormat = timeFormat::miliscnds>
class tt_Timer
{
private:
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point endTime;

public:
	// Start counting time from now
	std::chrono::high_resolution_clock::time_point Count()
	{
		startTime = std::chrono::high_resolution_clock::now();
		return startTime;
	};

	// Stop count, returns the result of count duration
	template<typename DurationTimeFormat = DefaultOutputTimeFormat>
	DurationTimeFormat Stop(timeFormat timeFormat = def_ttTimeFormat)
	{
		endTime = std::chrono::high_resolution_clock::now();

		return tt_calcDuration<DurationTimeFormat>(startTime, endTime, timeFormat);
	}

	// Get duration of last time count. 
	// Note, if time count is not stopped for now, return value could be <0
	template<typename DurationTimeFormat = DefaultOutputTimeFormat>
	DurationTimeFormat GetTimeCount(timeFormat timeFormat = def_ttTimeFormat)
	{
		return tt_calcDuration<DurationTimeFormat>(startTime, endTime, timeFormat);
	}

	// Get duration from last count time-start position to now
	template<typename DurationTimeFormat = DefaultOutputTimeFormat>
	DurationTimeFormat GetTimeCountNow(timeFormat timeFormat = def_ttTimeFormat)
	{
		auto nowEndTime = std::chrono::high_resolution_clock::now();
		return tt_calcDuration<DurationTimeFormat>(startTime, nowEndTime, timeFormat);
	}

	// Print debug msg about current global time point( std::chrono::system_clock::now() )
	void TimeNow(timeFormat timeFormat = def_ttTimeFormat)
	{
		std::cout << "[TIMER] TIME NOW: " << std::chrono::system_clock::now() << std::endl;
	}

	// Get time now from std::chrono
	auto GetTimeNowHRC()
	{
		return std::chrono::high_resolution_clock::now();
	}
};