// SPDX-License-Identifier: MIT | Author: Rohit Patil

#include <fstream>
#include <iostream>
#include <thread>

#include <examplePlatform.hpp>

using namespace std;

/*
 * Example program showing how to use the event‑collector API.
 *
 * The collector is platform‑dependent; therefore we instantiate a
 * TestPlatform object here and register it with the collector before
 * any events are pushed.
 */
static TestPlatform g_pltf;

/*
 * Demonstrates posting an event that carries an array of numbers.
 *
 * The event type `elementList_t` contains a fixed‑size array called
 * `nums`.  We obtain the parameter structure via `getParam()`,
 * populate it, and then push the event to the collector.
 */
void event_array_example() {
	Event<elementList_t> evt;
	elementList_t *param = nullptr;
	eventCollector *inst = nullptr;

	/* Retrieve the singleton instance of the collector. */
	inst = eventCollector::getInstance();

	/* Get a pointer to the event’s payload and fill it. */
	param			 = evt.getParam();
	param->nums[ 0 ] = 11;
	param->nums[ 1 ] = 22;
	param->nums[ 2 ] = 33;
	param->nums[ 3 ] = 44;

	/* Push the fully‑filled event to the collector. */
	inst->pushEvent( &evt );
}

/*
 * Posts a series of events in a tight loop.
 *
 * Each event carries an incrementing counter (type `loopCount_t`).
 * The function can be used to stress‑test timing or performance
 * characteristics of the collector.
 */
void event_loop_index( uint32_t maxLoopCount ) {
	uint32_t idx = 0;
	Event<loopCount_t> evt;
	loopCount_t *param	 = nullptr;
	eventCollector *inst = nullptr;

	/* Retrieve the singleton instance of the collector. */
	inst = eventCollector::getInstance();

	param = evt.getParam();
	for ( idx = 0; idx < maxLoopCount; idx++ ) {
		/* Update the counter and push the event. */
		param->count = idx;
		inst->pushEvent( &evt );
	}
}

/*
 * Dumps all packets collected so far into a binary file.
 *
 * The function forces the collector to flush its buffers, then
 * repeatedly retrieves packets via `getSendPacket()`.  Each packet
 * is written to the supplied output stream and marked as sent
 * using `sendPacketCompleted()`.
 */
bool dumpFile( string_view filePath ) {
	ofstream ofs( filePath.data(), ios::binary );
	eventCollector *inst = nullptr;

	if ( !ofs ) {
		cerr << "Failed to open file.\n";
		return false;
	}

	/* Retrieve the singleton instance of the collector. */
	inst = eventCollector::getInstance();

	/* Flush pending data so that packets are available for export. */
	inst->forceSync();
	auto pkt = inst->getSendPacket();

	/* Write all available packets to the file. */
	while ( pkt.has_value() ) {
		auto data = pkt.value();
		ofs.write( reinterpret_cast<const char *>( data.data() ), data.size() );
		inst->sendPacketCompleted(); // Mark packet as transmitted.
		pkt = inst->getSendPacket(); // Get next packet, if any.
	}

	ofs.close();

	return true;
}

int main() {
	eventCollector *inst = nullptr;

	/* Initialise the collector with a stream ID and platform interface. */
	inst = eventCollector::getInstance();
	inst->setStreamId( EVENT_STREAM_ID );
	inst->setPlatformIntf( &g_pltf );

	/* Generate sample events. */
	event_loop_index( 10 );
	event_array_example();

	/* Export the collected data to a file. */
	if ( !dumpFile( "stream.bin" ) ) {
		cerr << "Stream is not captured " << endl;
		return -1;
	}

	return 0;
}
