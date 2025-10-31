// SPDX-License-Identifier: MIT | Author: Rohit Patil

#include <event.hpp>
#include <eventCollector.hpp>
#include <event_types.hpp>

#include <mutex>

#pragma once

class TestPlatform : public eventPlatform {
	std::mutex eventMutex;
	std::mutex packetMutex;

public:
	uint64_t getTimestamp();
	bool eventTryLock();
	void eventUnlock();
	void packetLock();
	void packetUnlock();
};
