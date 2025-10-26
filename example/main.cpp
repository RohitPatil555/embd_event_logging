#include <fstream>
#include <iostream>
#include <thread>

#include <examplePlatform.hpp>

using namespace std;

static TestPlatform g_pltf;

void runLoop( uint32_t maxLoopCount ) {
	uint32_t idx = 0;
	Event<loopCount_t> evt;
	loopCount_t *param	 = nullptr;
	eventCollector *inst = nullptr;


	inst = eventCollector::getInstance();
	inst->setStreamId( EVENT_STREAM_ID );
	inst->setPlatformIntf( &g_pltf );

	param = evt.getParam();
	for ( idx = 0; idx < maxLoopCount; idx++ ) {
		param->count = idx;
		inst->pushEvent( &evt );
	}
}

bool dumpFile( string_view filePath ) {
	ofstream ofs( filePath.data(), ios::binary );
	eventCollector *inst = nullptr;

	if ( !ofs ) {
		cerr << "Failed to open file.\n";
		return false;
	}

	// get packet
	inst	 = eventCollector::getInstance();
	auto pkt = inst->getSendPacket();

	if ( pkt.has_value() ) {
		auto data = pkt.value();
		ofs.write( reinterpret_cast<const char *>( data.data() ), data.size() );
		inst->sendPacketCompleted();
	} else {
		cerr << "fail to get event packet" << endl;
		return false;
	}

	ofs.close();

	return true;
}

int main() {
	runLoop( 10 );

	if ( !dumpFile( "stream.bin" ) ) {
		cerr << "Stream is not captured " << endl;
		return -1;
	}

	return 0;
}
