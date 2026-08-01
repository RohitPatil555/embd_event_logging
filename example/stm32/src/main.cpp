#include <compiler_req_apis.h>
#include <cstdint>
#include <scheduler.hpp>
#include <string.h>
#include <task.hpp>

#define TOTAL_TASK_FRAME_SIZE 512

using namespace std;

TaskPool *Task::promise_type::poolPtr = nullptr;
alignas( uint32_t ) static uint8_t raw_memory_frame_pool[ TOTAL_TASK_FRAME_SIZE ];

static TaskPool *getFixMemoryPool() {
	TaskPool *ptr = new ( raw_memory_frame_pool )
		FixTaskFrameAllocator<SCHEDULER_TASK_COUNT_MAX, SCHEDULER_TASK_FRAM_SIZE>();
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

	auto task1 = countLoopTask( 10 );
	auto task2 = countLoopTask( 1000 );
	auto task3 = countLoopTask( 100000 );

	auto sched = scheduler();

	sched.append( &task1 );
	sched.append( &task2 );
	sched.append( &task3 );

	// Infinite loop to make cpu busy.
	while ( 1 ) {
		sched.run();
	}
}

// Vector Table
extern "C" uint32_t _estack;
__attribute__( ( section( ".isr_vector" ) ) ) uint32_t vector_table[] = { (uint32_t)&_estack,
																		  (uint32_t)Reset_Handler };
