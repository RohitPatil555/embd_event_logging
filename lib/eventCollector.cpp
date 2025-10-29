/*********************************************************************
 *  eventCollector implementation
 *
 *  This file implements a lightweight runtime component that
 *  aggregates individual events into packets for transmission.
 *  The design follows the P‑Impl idiom to keep the public interface
 *  stable while allowing internal changes without breaking ABI.
 *********************************************************************/

/* --------------------------------------------------------------------------
 *  Standard library headers
 * -------------------------------------------------------------------------- */
#include <cassert>

/* --------------------------------------------------------------------------
 *  Project headers
 * -------------------------------------------------------------------------- */
#include <Queue.hpp>
#include <eventCollector.hpp>
#include <internal/eventPacket.hpp>
#include <staticPool.hpp>

/* --------------------------------------------------------------------
 *  Global pool of packet objects.
 *
 *  The pool size is defined by the configuration macro
 *  CONFIG_PACKET_COUNT_MAX.  It provides fast allocation and
 *  deallocation without dynamic memory fragmentation.
 * -------------------------------------------------------------------- */
static StaticPool<eventPacket, CONFIG_PACKET_COUNT_MAX> pktPool;

/* --------------------------------------------------------------------
 *  Private implementation of eventCollector (P‑Impl).
 *
 *  The Impl only contains a queue that holds pointers to packets
 *  ready for transmission.  The queue capacity matches the pool
 *  size so no allocation failure can occur.
 * -------------------------------------------------------------------- */
struct eventCollector::Impl {
	Queue<eventPacket_ptr_t, CONFIG_PACKET_COUNT_MAX> queue;
};

/* --------------------------------------------------------------------
 *  Constructor – initialise hidden implementation and state.
 *
 *  * `impl`   : placement‑new of Impl inside the storage buffer.
 * -------------------------------------------------------------------- */
eventCollector::eventCollector() {
	static_assert( ImplSize == sizeof( Impl ), "hidden implementation size not matching" );

	impl			  = new ( storage.data() ) Impl();
	sendPkt			  = nullptr;
	currPkt			  = nullptr;
	discardEventCount = 0;
	pktSqnNo		  = 0;
	streamId		  = 0;
	pltf			  = nullptr;
}

/* --------------------------------------------------------------------
 *  Acquire the current packet for event insertion.
 *
 *  If no packet is currently active, allocate one from the pool,
 *  initialise it with the current stream ID and sequence number
 *  and reset the discard counter.
 * -------------------------------------------------------------------- */
eventPacket *eventCollector::getCurrentPacket() {
	if ( currPkt == nullptr ) {
		currPkt = pktPool.allocate();
		currPkt->init( streamId, pktSqnNo );
		discardEventCount = 0;
		pktSqnNo++;
	}

	return currPkt;
}

/* --------------------------------------------------------------------
 *  Send the current packet to the queue.
 *
 *  The packet must already exist (checked by assert).  It is
 *  inserted into the internal queue and then cleared so a new one
 *  can be built.  Because the queue capacity equals the pool size,
 *  insertion never fails – the following assert guarantees that.
 * -------------------------------------------------------------------- */
void eventCollector::sendPacket() {
	bool qstatus = false;
	// this must not be null in this path as per design.
	assert( currPkt != nullptr );
	qstatus = impl->queue.insert( currPkt );

	// As queue size and packet buffer have same count it
	// never get asserted.
	assert( qstatus );

	currPkt = nullptr;
}

/* --------------------------------------------------------------------
 *  Singleton accessor.
 *
 *  The collector is a global, stateless object.  This function
 *  returns the single instance.
 * -------------------------------------------------------------------- */
eventCollector *eventCollector::getInstance() noexcept {
	static eventCollector eventInst;

	return &eventInst;
}

/* --------------------------------------------------------------------
 *  Public API – add an event to the collector.
 *
 *  1. Obtain or create a current packet.
 *  2. Acquire platform timestamp and store it in the event.
 *  3. Add the event to the packet (thread‑safe via platform lock).
 *  4. If the packet becomes full, build its wire format and
 *     enqueue it for sending.
 * -------------------------------------------------------------------- */
void eventCollector::sendEvent( EventIntf *evt ) {
	eventPacket *curr = getCurrentPacket();
	uint64_t _ts	  = 0;

	if ( curr == nullptr ) {
		discardEventCount++;
		return;
	}

	pltf->eventLock();
	_ts = pltf->getTimestamp();
	evt->setTimestamp( _ts );
	curr->addEvent( evt );
	pltf->eventUnlock();

	if ( curr->isPacketFull() ) {
		curr->buildPacket();
		sendPacket();
	}
}

/* --------------------------------------------------------------------
 *  Callback invoked when a previously sent packet has been processed.
 *
 *  The packet is returned to the pool so it can be reused.
 * -------------------------------------------------------------------- */
void eventCollector::sendPacketCompleted() {
	if ( sendPkt != nullptr ) {
		pktPool.release( sendPkt );
		sendPkt = nullptr;
	}
}

/* --------------------------------------------------------------------
 *  Retrieve a ready‑to‑send packet for transmission.
 *
 *  If no packet is currently cached, pull one from the queue.  The
 *  caller receives an optional byte span that points to the raw
 *  packet buffer; if the queue was empty `std::nullopt` is returned.
 * -------------------------------------------------------------------- */
std::optional<std::span<const std::byte>> eventCollector::getSendPacket() {
	if ( sendPkt == nullptr ) {
		auto pkt = impl->queue.remove();
		if ( !pkt.has_value() ) {
			return std::nullopt;
		}
		sendPkt = pkt.value();
	}

	return std::optional<std::span<const std::byte>>( sendPkt->getPacketInRaw() );
}

/* --------------------------------------------------------------------
 *  Configure the stream identifier for packets.
 *
 *  The call must be idempotent – it is only legal to set the ID
 *  once during initialization.  An assertion guards against misuse.
 * -------------------------------------------------------------------- */
void eventCollector::setStreamId( uint32_t _streamId ) {
	// either called 2 time or some error in init path.
	assert( streamId == 0 );

	streamId = _streamId;
}

/* --------------------------------------------------------------------
 *  Attach the platform interface implementation.
 *
 *  The collector uses this to obtain timestamps and perform
 *  thread‑synchronisation.  It must only be set once; otherwise
 *  a double‑initialisation would corrupt state.
 * -------------------------------------------------------------------- */
void eventCollector::setPlatformIntf( eventPlatform *_pltf ) {
	assert( pltf == nullptr );

	pltf = _pltf;
}
