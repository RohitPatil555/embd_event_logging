#include <cstring>
#include <internal/eventPacket.hpp>

using namespace std;

eventPacket::eventPacket( uint32_t streamId, uint32_t seqNo ) {
	currOffset = 0;
	eventCount = 0;
	memset( &buffer, 0, sizeof( buffer ) );

	buffer.stream_id		= streamId;
	buffer.packet_seq_count = seqNo;
}

eventPacket::~eventPacket() { memset( &buffer, 0, sizeof( buffer ) ); }

bool eventPacket::isPacketFull() {
	if ( eventCount < CONFIG_EVENT_MAX_PER_PACKET ) {
		return false;
	}

	return true;
}

bool eventPacket::addEvent( EventIntf *eventPtr ) {
	span<const byte> eventPayload;

	if ( isPacketFull() ) {
		return false;
	}

	eventPayload = eventPtr->getEventInRaw();

	memcpy( &buffer.eventPayload[ currOffset ], eventPayload.data(), eventPayload.size() );
	currOffset += eventPayload.size();
	eventCount++;

	return true;
}

void eventPacket::buildPacket() {
	size_t hdrSize = 0;

	hdrSize				= sizeof( buffer ) - buffer.eventPayload.size();
	buffer.packet_size	= sizeof( buffer );
	buffer.content_size = hdrSize + currOffset;
}

span<const byte> eventPacket::getPacketInRaw() { return as_bytes( span( &buffer, 1 ) ); }
