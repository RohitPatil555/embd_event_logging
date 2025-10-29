// SPDX-License-Identifier: MIT | Author: Rohit Patil

#pragma once

/* --------------------------------------------------------------------------
 *  Standard library headers
 * -------------------------------------------------------------------------- */
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

/* --------------------------------------------------------------------------
 *  Project headers
 * -------------------------------------------------------------------------- */
#include <config.hpp>
#include <event.hpp>

/* --------------------------------------------------------------------------
 *  Raw packet buffer layout
 *
 *  The struct is marked `packed` so that the memory representation matches
 *  what will be transmitted over the wire (no padding bytes).
 * -------------------------------------------------------------------------- */
typedef struct {
	uint32_t stream_id;		   // ID of the originating stream
	uint32_t events_discarded; // how many events were dropped before adding to this packet
	uint32_t packet_size;	   // total size of the packet in bytes (header + payload)
	uint32_t content_size;	   // size of the event payload only
	uint32_t packet_seq_count; // sequence number for ordering packets

	/* Fixed‑size buffer that will hold the concatenated raw bytes of all events. */
	std::array<uint8_t, EVENT_MAX_PAYLOAD_IN_BYTES> eventPayload;
} __attribute__( ( packed ) ) packet_buffer_t;

/* --------------------------------------------------------------------------
 *  EventPacket – a small helper class that builds a packet from Events
 *
 *  It does not expose any public data members; everything is encapsulated.
 * -------------------------------------------------------------------------- */
class eventPacket {
	/* Current offset inside the payload array where the next event will be copied. */
	std::size_t currOffset;

	/* Number of events already added to this packet. */
	std::size_t eventCount;

	/* The raw memory buffer that represents the packet. */
	packet_buffer_t buffer;

public:
	eventPacket() = default;
	~eventPacket();

	/* Initialise a new packet with a stream identifier and a sequence number. */
	void init( uint32_t streamId, uint32_t seqNo );

	/* Return true if adding another event would overflow the payload array. */
	bool isPacketFull();

	/* Add an Event to the packet; returns false if the packet is already full. */
	bool addEvent( EventIntf *eventPtr );

	/* Increment the counter of dropped events (used when a packet overflows). */
	void dropEvent() { buffer.events_discarded++; }

	/* Return a read‑only byte span that contains the entire packet (header + payload). */
	std::span<const std::byte> getPacketInRaw();

	/* Finalise the packet: compute sizes, set sequence numbers, etc. */
	void buildPacket();
};

/* --------------------------------------------------------------------------
 *  Convenience typedef for an eventPacket pointer
 * -------------------------------------------------------------------------- */
typedef eventPacket *eventPacket_ptr_t;
