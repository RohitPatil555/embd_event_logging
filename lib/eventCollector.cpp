
#include <eventCollector.hpp>

eventCollector::eventCollector() {
	// add later.
}

eventCollector *eventCollector::getInstance() noexcept {
	static eventCollector eventInst;

	return &eventInst;
}
