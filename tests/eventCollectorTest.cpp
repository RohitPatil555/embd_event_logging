#include <event.hpp>
#include <eventCollector.hpp>
#include <gtest/gtest.h>

using namespace std;

// Mock event class for testing
typedef struct {
	array<std::byte, 10> value;
} __attribute__( ( packed ) ) mock_event_t;

// Test: Verify singleton instance
TEST( EventCollectorTest, SingletonInstance ) {
	auto *inst1 = eventCollector::getInstance();
	auto *inst2 = eventCollector::getInstance();
	EXPECT_EQ( inst1, inst2 );
}

// Test: Push events and send first packet
TEST( EventCollectorTest, PushAndSendFirstPacket ) {
	auto *collector = eventCollector::getInstance();
	collector->setStreamId( 100 );

	// Create mock events
	auto evt			= new Event<mock_event_t>;
	mock_event_t *param = evt->getParam();
	memset( param->value.data(), 0x11, 10 );

	// Push events to fill current packet
	for ( int i = 0; i < CONFIG_EVENT_MAX_PER_PACKET; i++ ) {
		collector->pushEvent( evt );
	}

	delete evt;

	// Verify packet can be sent
	auto sendPacket = collector->getSendPacket();
	EXPECT_TRUE( sendPacket.has_value() );
	EXPECT_FALSE( sendPacket.value().empty() );

	// Complete the packet
	collector->sendPacketCompleted();
}

// Test: Handle empty send queue
TEST( EventCollectorTest, EmptySendQueue ) {
	auto *collector = eventCollector::getInstance();
	collector->setStreamId( 42 ); // Arbitrary stream ID

	auto sendPacket = collector->getSendPacket();
	EXPECT_FALSE( sendPacket.has_value() );
}

// Test: Sequential packet sending with completion
TEST( EventCollectorTest, SequentialPackets ) {
	auto *collector = eventCollector::getInstance();
	collector->setStreamId( 200 );

	// Create mock events
	auto evt1			 = new Event<mock_event_t>;
	mock_event_t *param1 = evt1->getParam();
	memset( param1->value.data(), 0x11, 10 );

	// Push events to fill current packet
	for ( int i = 0; i < CONFIG_EVENT_MAX_PER_PACKET; i++ ) {
		collector->pushEvent( evt1 );
	}

	delete evt1;

	// Get first packet
	auto span1 = collector->getSendPacket();
	EXPECT_TRUE( span1.has_value() );
	vector<byte> data1( span1.value().data(), span1.value().data() + span1.value().size() );

	// Complete first packet
	collector->sendPacketCompleted();

	// Create mock events
	auto evt2			 = new Event<mock_event_t>;
	mock_event_t *param2 = evt2->getParam();
	memset( param2->value.data(), 0x22, 10 );

	// Push events to fill current packet
	for ( int i = 0; i < CONFIG_EVENT_MAX_PER_PACKET; i++ ) {
		collector->pushEvent( evt2 );
	}

	delete evt2;

	// Get second packet and verify it's different
	auto span2 = collector->getSendPacket();
	EXPECT_TRUE( span2.has_value() );
	vector<byte> data2( span2.value().data(), span2.value().data() + span2.value().size() );

	// Ensure spans are for different packet. Skip 20 byte header size.
	for ( size_t i = 20; i < 10; i++ ) {
		EXPECT_NE( data1[ i ], data2[ i ] );
	}
}
