// SPDX-License-Identifier: MIT | Author: Rohit Patil

#pragma once

/* --------------------------------------------------------------------------
 *  Standard library headers
 * -------------------------------------------------------------------------- */
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

/* --------------------------------------------------------------------------
 *  Project headers
 * -------------------------------------------------------------------------- */
#include <config.hpp>
#include <event.hpp>

/* Forward declaration – the packet type is defined elsewhere */
class eventPacket;

/* --------------------------------------------------------------------------
 *  Platform Interface
 *
 *  The `eventPlatform` class defines the only operations that the user must
 *  provide.  All methods are pure virtual so that any concrete platform can
 *  be plugged in without changing this header.
 * -------------------------------------------------------------------------- */
class eventPlatform {
public:
	/* Return a monotonically increasing timestamp (nanoseconds is common). */
	virtual uint64_t getTimestamp() = 0;

	/* Acquire an exclusive lock before mutating shared state. */
	virtual void eventLock() = 0;

	/* Release the lock after mutation is finished. */
	virtual void eventUnlock() = 0;
};

/* --------------------------------------------------------------------------
 *  Event Collector
 *
 *  `eventCollector` is a singleton that collects events into packets.
 *  It owns an opaque implementation object (`Impl`) stored in a fixed‑size
 *  buffer to avoid dynamic allocation on the critical path.  The collector
 *  keeps two packet pointers:
 *
 *      - currPkt : packet currently being built
 *      - sendPkt : packet that has been finished and is ready for sending
 * -------------------------------------------------------------------------- */
class eventCollector {
	/* ----------------------------------------------------------------------
	 *  Impl
	 *
	 *  The actual implementation details are hidden behind the pointer `impl`.
	 *  To keep memory usage predictable, the implementation is stored inside
	 *  a fixed‑size array (`storage`) that is large enough for any expected
	 *  layout.  The size of this buffer is intentionally small – it can be
	 *  tuned by changing `ImplSize` if the implementation grows.
	 * ---------------------------------------------------------------------- */
	struct Impl;						   // forward declaration
	static constexpr size_t ImplSize = 48; // bytes reserved for Impl
	alignas( std::max_align_t ) std::array<std::byte, ImplSize> storage;

	Impl *impl;

	/* ----------------------------------------------------------------------
	 *  Platform pointer
	 *
	 *  The collector delegates timestamp and locking to this interface.
	 * ---------------------------------------------------------------------- */
	eventPlatform *pltf;

	/* ----------------------------------------------------------------------
	 *  Packet bookkeeping
	 *
	 *  Two packet pointers are maintained:
	 *      - currPkt: the packet being populated with events
	 *      - sendPkt: the packet that is in progress to send out.
	 * ---------------------------------------------------------------------- */
	eventPacket *currPkt;
	eventPacket *sendPkt;

	uint32_t
		discardEventCount; // number of events dropped because the current packet is not available
	uint32_t pktSqnNo;	   // monotonically increasing sequence number for packets.
	uint32_t streamId;	   // identifier that will be embedded in each packet header.

	/* ----------------------------------------------------------------------
	 *  Private constructor
	 *
	 *  The singleton is created on first use via `getInstance()`.  The ctor
	 *  sets up the Impl object inside the storage buffer.
	 * ---------------------------------------------------------------------- */
	eventCollector();

	/* lazily creates or re‑uses a packet for writing. */
	eventPacket *getCurrentPacket();

	/* finalises the current packet and hands it to send-Q */
	void sendPacket();

	/* serialises an event into the current packet.*/
	void sendEvent( EventIntf *evt );

public:
	/* ----------------------------------------------------------------------
	 *  Singleton accessor
	 *
	 *  `noexcept` guarantees that retrieving the instance cannot throw.
	 * ---------------------------------------------------------------------- */
	static eventCollector *getInstance() noexcept;

	/* ----------------------------------------------------------------------
	 *  Event submission
	 *
	 *  The template accepts any type that satisfies the `IsEventType`
	 *  concept.  It simply casts to the base interface and forwards it
	 *  to `sendEvent()`.
	 * ---------------------------------------------------------------------- */
	template <IsEventType E> inline void pushEvent( E *ptr ) {
		EventIntf *intf = ptr;
		sendEvent( intf );
	}

	/* ----------------------------------------------------------------------
	 *  Packet retrieval
	 *
	 *  If a packet has been queued for transmission, this returns a span over
	 *  its raw bytes.  The caller is responsible for handling the data and
	 *  then notifying the collector that sending is finished.
	 * ---------------------------------------------------------------------- */
	std::optional<std::span<const std::byte>> getSendPacket();
	void sendPacketCompleted(); // Notify that the platform has finished sending `sendPkt`

	/* ----------------------------------------------------------------------
	 *  Configuration helpers
	 *
	 *  streamId – sets the identifier that will be embedded in every packet.
	 *  pltf     – registers the platform interface implementation.
	 * ---------------------------------------------------------------------- */
	void setStreamId( uint32_t _streamId );
	void setPlatformIntf( eventPlatform *_pltf );
};
