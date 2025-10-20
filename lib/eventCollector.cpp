
#include <eventCollector.hpp>
#include <internal/eventPacket.hpp>
#include <staticPool.hpp>

// static StaticPool<eventPacket, CONFIG_PACKET_COUNT_MAX> pktPool;

eventCollector::eventCollector() {
	// add later.
}

eventCollector *eventCollector::getInstance() noexcept {
	static eventCollector eventInst;

	return &eventInst;
}
