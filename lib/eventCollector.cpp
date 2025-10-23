
#include <cassert>

#include <Queue.hpp>
#include <eventCollector.hpp>
#include <internal/eventPacket.hpp>
#include <staticPool.hpp>

static StaticPool<eventPacket, CONFIG_PACKET_COUNT_MAX> pktPool;

struct eventCollector::Impl {
	Queue<eventPacket_ptr_t, CONFIG_PACKET_COUNT_MAX> queue;
};

eventCollector::eventCollector() {
	static_assert( ImplSize == sizeof( Impl ), "hidden implementation size not matching" );

	impl			  = new ( storage.data() ) Impl();
	sendPkt			  = nullptr;
	currPkt			  = nullptr;
	discardEventCount = 0;
	pktSqnNo		  = 0;
	streamId		  = 0;
}

eventPacket *eventCollector::getCurrentPacket() {
	if ( currPkt == nullptr ) {
		currPkt = pktPool.allocate();
		currPkt->init( streamId, pktSqnNo );
		discardEventCount = 0;
		pktSqnNo++;
	}

	return currPkt;
}

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

eventCollector *eventCollector::getInstance() noexcept {
	static eventCollector eventInst;

	return &eventInst;
}


void eventCollector::sendEvent( EventIntf *evt ) {
	eventPacket *curr = getCurrentPacket();

	if ( curr == nullptr ) {
		discardEventCount++;
		return;
	}

	curr->addEvent( evt );

	if ( curr->isPacketFull() ) {
		curr->buildPacket();
		sendPacket();
	}
}

void eventCollector::sendPacketCompleted() {
	if ( sendPkt != nullptr ) {
		pktPool.release( sendPkt );
		sendPkt = nullptr;
	}
}

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

void eventCollector::setStreamId( uint32_t _streamId ) {
	// either called 2 time or some error in init path.
	assert( streamId == 0 );

	streamId = _streamId;
}
