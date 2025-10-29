// SPDX-License-Identifier: MIT | Author: Rohit Patil

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <config.hpp>

#include <event.hpp>

typedef struct {
	uint32_t stream_id;
	uint32_t events_discarded;
	uint32_t packet_size;
	uint32_t content_size;
	uint32_t packet_seq_count;
	std::array<uint8_t, EVENT_MAX_PAYLOAD_IN_BYTES> eventPayload;
} __attribute__( ( packed ) ) packet_buffer_t;

class eventPacket {
	std::size_t currOffset;
	std::size_t eventCount;
	packet_buffer_t buffer;

public:
	eventPacket() = default;
	~eventPacket();

	void init( uint32_t streamId, uint32_t seqNo );

	bool isPacketFull();
	bool addEvent( EventIntf *eventPtr );
	void dropEvent() { buffer.events_discarded++; }

	std::span<const std::byte> getPacketInRaw();

	void buildPacket();
};

typedef eventPacket *eventPacket_ptr_t;
