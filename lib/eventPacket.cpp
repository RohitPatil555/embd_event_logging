
/* --------------------------------------------------------------------------
 *  Standard library headers
 * -------------------------------------------------------------------------- */
#include <cstring>

/* --------------------------------------------------------------------------
 *  Project headers
 * -------------------------------------------------------------------------- */
#include <internal/eventPacket.hpp>

using namespace std;
/* --------------------------------------------------------------------
 * Destructor: Zero out the packet buffer when an eventPacket is destroyed.
 * This helps avoid leaking sensitive information or leaving stale data in memory.
 * -------------------------------------------------------------------- */
eventPacket::~eventPacket() { memset( &buffer, 0, sizeof( buffer ) ); }

/* --------------------------------------------------------------------
 * Initialise a new packet with a stream identifier and sequence number.
 * The internal offset counters are reset and the header fields are
 * populated.  The payload area is cleared to ensure no leftover data
 * from a previous use contaminates the new packet.
 * -------------------------------------------------------------------- */
void eventPacket::init( uint32_t streamId, uint32_t seqNo ) {
	currOffset = 0;
	eventCount = 0;

	memset( &buffer, 0, sizeof( buffer ) );
	buffer.stream_id		= streamId;
	buffer.packet_seq_count = seqNo;
}

/* --------------------------------------------------------------------
 * Check whether the packet has reached its maximum number of events.
 * Returns true when no more events can be added; otherwise false.
 * -------------------------------------------------------------------- */
bool eventPacket::isPacketFull() {
	if ( eventCount < CONFIG_EVENT_MAX_PER_PACKET ) {
		return false;
	}

	return true;
}

/* --------------------------------------------------------------------
 * Append a single event to the packet payload.
 * The caller must have verified that the packet is not full.
 * Copies the raw byte representation of the event into the buffer
 * and updates the offset/size counters accordingly.
 * Returns true on success, false if the packet was already full.
 * -------------------------------------------------------------------- */
bool eventPacket::addEvent( EventIntf *eventPtr ) {
	span<const byte> eventPayload;

	// Prevent overflow: do not add when capacity is exhausted.
	if ( isPacketFull() ) {
		return false;
	}

	// Retrieve the raw, byte‑wise representation of the event.
	eventPayload = eventPtr->getEventInRaw();

	// Copy the payload into the packet buffer at the current offset.
	memcpy( &buffer.eventPayload[ currOffset ], eventPayload.data(), eventPayload.size() );

	// Update bookkeeping values for next insertion.
	currOffset += eventPayload.size();
	eventCount++;

	return true;
}

/* --------------------------------------------------------------------
 * Finalise the packet by computing its size fields.
 * The packet header is updated with total packet size (in bits)
 * and the content size (header + payload, in bits).
 * -------------------------------------------------------------------- */

void eventPacket::buildPacket() {
	size_t hdrSize = 0;

	hdrSize				= sizeof( buffer ) - buffer.eventPayload.size();
	buffer.packet_size	= sizeof( buffer ) * 8;			// convert to bit
	buffer.content_size = ( hdrSize + currOffset ) * 8; // convert to bit
}

/* --------------------------------------------------------------------
 * Return the entire packet as a byte span.
 * The caller can then transmit or otherwise process the raw data.
 * -------------------------------------------------------------------- */
span<const byte> eventPacket::getPacketInRaw() { return as_bytes( span( &buffer, 1 ) ); }
