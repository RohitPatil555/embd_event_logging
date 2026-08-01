#include <compiler_req_apis.h>
#include <cstdint>
#include <string.h>
#include <task.hpp>

#define MAX_NUMBER_OF_TASK 3
#define MAX_TASK_FRAME_SIZE 128
#define TOTAL_TASK_FRAME_SIZE 512

using namespace std;

TaskPool *Task::promise_type::poolPtr = nullptr;
alignas( uint32_t ) static uint8_t raw_memory_frame_pool[ TOTAL_TASK_FRAME_SIZE ];

static TaskPool *getFixMemoryPool() {
	TaskPool *ptr = new ( raw_memory_frame_pool ) FixTaskFrameAllocator<3, 128>();
	return ptr;
}

Task countLoopTask( size_t maxLoopCount ) {
	uint32_t count = 0;

	for ( ; count < maxLoopCount; count++ )
		;

	co_await std::suspend_always{};
}

extern "C" void Reset_Handler() {
	Task::set_task_pool( getFixMemoryPool() );

	auto task = countLoopTask( 100 );

	task.resume();

	task.resume();

	// Infinite loop to make cpu busy.
	while ( 1 )
		;
}

// Vector Table
extern "C" uint32_t _estack;
__attribute__( ( section( ".isr_vector" ) ) ) uint32_t vector_table[] = { (uint32_t)&_estack,
																		  (uint32_t)Reset_Handler };
