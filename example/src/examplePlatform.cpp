
#include <chrono>
#include <cstdint>
#include <examplePlatform.hpp>

#include <iostream>

uint64_t TestPlatform::getTimestamp() {
	using namespace std::chrono;
	return static_cast<uint64_t>(
		duration_cast<nanoseconds>( system_clock::now().time_since_epoch() ).count() );
}

bool TestPlatform::eventTryLock() { return eventMutex.try_lock(); }

void TestPlatform::eventUnlock() { eventMutex.unlock(); }

void TestPlatform::packetLock() { packetMutex.lock(); }

void TestPlatform::packetUnlock() { packetMutex.unlock(); }
