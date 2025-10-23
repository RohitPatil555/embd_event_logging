
#pragma once

#include <array>
#include <config.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include <event.hpp>

class eventPacket;

class eventCollector {
	struct Impl;
	static constexpr size_t ImplSize = 48;
	alignas( std::max_align_t ) std::array<std::byte, ImplSize> storage;

	Impl *impl;

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
};
