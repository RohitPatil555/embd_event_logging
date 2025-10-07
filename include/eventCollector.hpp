
#pragma once

#include <cstddef>
#include <cstdint>

class eventCollector {
	eventCollector();

	public:
	static eventCollector *getInstance() noexcept;
};
