#include <gtest/gtest.h>

#include <event.hpp>
#include <internal/eventPacket.hpp>

#define CONVERT_SIZE_IN_BITS( x ) ( ( x ) * 8 )

class MockEvent : public EventIntf {
	std::vector<std::byte> m_data;

public:
	explicit MockEvent( size_t size, uint8_t fill_value = 0 ) : m_data( size ) {
		std::fill( m_data.begin(), m_data.end(), static_cast<std::byte>( fill_value ) );
	}

	void setTimestamp( uint64_t ts ) { (void)ts; }

	std::span<const std::byte> getEventInRaw() override { return m_data; }
};

class EventPacketTest : public ::testing::Test {
protected:
	const uint32_t TEST_STREAM_ID			 = 0xABCD1234;
	const uint32_t TEST_SEQ_NO				 = 0x100;
	const uint32_t TEST_EVENT_MAX_SIZE		 = 64;
	const uint32_t TEST_EVENT_PADDING_SIZE_1 = 60;
	const uint32_t TEST_EVENT_DROP_COUNT	 = 10;
};

TEST_F( EventPacketTest, Initialization ) {
	eventPacket packet;
	const packet_buffer_t *pktBuf = nullptr;

	packet.init( TEST_STREAM_ID, TEST_SEQ_NO );
	packet.buildPacket();

	auto raw = packet.getPacketInRaw();
	pktBuf	 = reinterpret_cast<const packet_buffer_t *>( raw.data() );

	EXPECT_EQ( pktBuf->stream_id, TEST_STREAM_ID );
	EXPECT_EQ( pktBuf->events_discarded, 0 );
	EXPECT_EQ( pktBuf->packet_seq_count, TEST_SEQ_NO );

	EXPECT_EQ( pktBuf->content_size, CONVERT_SIZE_IN_BITS( sizeof( uint32_t ) * 5 ) );
	EXPECT_EQ( pktBuf->packet_size, CONVERT_SIZE_IN_BITS( sizeof( packet_buffer_t ) ) );
}

TEST_F( EventPacketTest, CapacityManagement ) {
	eventPacket packet;
	const packet_buffer_t *pktBuf = nullptr;
	MockEvent mevt( TEST_EVENT_MAX_SIZE, 0x11 );

	packet.init( TEST_STREAM_ID, TEST_SEQ_NO );

	// Push event till packet is full.
	while ( !packet.isPacketFull() ) {
		packet.addEvent( &mevt );
	}

	packet.buildPacket();

	auto raw = packet.getPacketInRaw();
	pktBuf	 = reinterpret_cast<const packet_buffer_t *>( raw.data() );

	EXPECT_EQ( pktBuf->stream_id, TEST_STREAM_ID );
	EXPECT_EQ( pktBuf->events_discarded, 0 );
	EXPECT_EQ( pktBuf->packet_seq_count, TEST_SEQ_NO );

	EXPECT_EQ( pktBuf->content_size, CONVERT_SIZE_IN_BITS( sizeof( packet_buffer_t ) ) );
	EXPECT_EQ( pktBuf->packet_size, CONVERT_SIZE_IN_BITS( sizeof( packet_buffer_t ) ) );
}


TEST_F( EventPacketTest, PaddingValidation ) {
	eventPacket packet;
	const packet_buffer_t *pktBuf = nullptr;
	MockEvent mevt( TEST_EVENT_PADDING_SIZE_1, 0x22 );
	const uint32_t padSize =
		( TEST_EVENT_MAX_SIZE - TEST_EVENT_PADDING_SIZE_1 ) * CONFIG_EVENT_MAX_PER_PACKET;

	packet.init( TEST_STREAM_ID, TEST_SEQ_NO );

	// Push event till packet is full.
	while ( !packet.isPacketFull() ) {
		packet.addEvent( &mevt );
	}

	packet.buildPacket();

	auto raw = packet.getPacketInRaw();
	pktBuf	 = reinterpret_cast<const packet_buffer_t *>( raw.data() );

	EXPECT_EQ( pktBuf->stream_id, TEST_STREAM_ID );
	EXPECT_EQ( pktBuf->events_discarded, 0 );
	EXPECT_EQ( pktBuf->packet_seq_count, TEST_SEQ_NO );

	EXPECT_EQ( pktBuf->content_size, CONVERT_SIZE_IN_BITS( sizeof( packet_buffer_t ) - padSize ) );
	EXPECT_EQ( pktBuf->packet_size, CONVERT_SIZE_IN_BITS( sizeof( packet_buffer_t ) ) );
}

TEST_F( EventPacketTest, DropEventValidation ) {
	eventPacket packet;
	const packet_buffer_t *pktBuf = nullptr;

	packet.init( TEST_STREAM_ID, TEST_SEQ_NO );

	for ( uint32_t i = 0; i < TEST_EVENT_DROP_COUNT; i++ ) {
		packet.dropEvent();
	}

	packet.buildPacket();

	auto raw = packet.getPacketInRaw();
	pktBuf	 = reinterpret_cast<const packet_buffer_t *>( raw.data() );

	EXPECT_EQ( pktBuf->stream_id, TEST_STREAM_ID );
	EXPECT_EQ( pktBuf->events_discarded, TEST_EVENT_DROP_COUNT );
	EXPECT_EQ( pktBuf->packet_seq_count, TEST_SEQ_NO );

	EXPECT_EQ( pktBuf->content_size, CONVERT_SIZE_IN_BITS( sizeof( uint32_t ) * 5 ) );
	EXPECT_EQ( pktBuf->packet_size, CONVERT_SIZE_IN_BITS( sizeof( packet_buffer_t ) ) );
}
