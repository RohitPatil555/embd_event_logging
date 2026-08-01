#include <scheduler.hpp>

void scheduler::run( void ) {
	Task *currTaskPtr = nullptr;

	for ( size_t i = 0; i < taskCount; i++ ) {
		currTaskPtr = taskList[ i ];

		if ( currTaskPtr == nullptr ) {
			continue;
		}

		if ( currTaskPtr->is_completed() ) {
			taskList[ i ] = nullptr;
			continue;
		}

		currTaskPtr->resume();
	}
}
