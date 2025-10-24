
#pragma once

#include <array>
#include <config.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include <event.hpp>

class eventPacket;

class eventPlatform {
public:
	virtual uint64_t getTimestamp() = 0;
	virtual void eventLock()		= 0;
	virtual void eventUnlock()		= 0;
};

class eventCollector {
	struct Impl;
	static constexpr size_t ImplSize = 48;
	alignas( std::max_align_t ) std::array<std::byte, ImplSize> storage;

	Impl *impl;

	eventPlatform *pltf;

	eventPacket *currPkt;
	eventPacket *sendPkt;

	uint32_t discardEventCount;
	uint32_t pktSqnNo;
	uint32_t streamId;

	eventCollector();

	eventPacket *getCurrentPacket();
	void sendPacket();
	void sendEvent( EventIntf *evt );

public:
	static eventCollector *getInstance() noexcept;

	template <IsEventType E> inline void pushEvent( E *ptr ) {
		EventIntf *intf = ptr;
		sendEvent( intf );
	}

	std::optional<std::span<const std::byte>> getSendPacket();
	void sendPacketCompleted();

	void setStreamId( uint32_t _streamId );
	void setPlatformIntf( eventPlatform *_pltf );
};
