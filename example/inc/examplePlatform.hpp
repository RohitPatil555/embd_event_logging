
#include <event.hpp>
#include <eventCollector.hpp>
#include <event_types.hpp>

#include <mutex>

#pragma once

class TestPlatform : public eventPlatform {
	std::mutex eventMutex;

public:
	uint64_t getTimestamp();
	void eventLock();
	void eventUnlock();
};
