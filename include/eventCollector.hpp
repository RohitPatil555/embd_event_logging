
#pragma once

#include <config.hpp>
#include <cstddef>
#include <cstdint>

class eventPacket;

class eventCollector {
	eventCollector();

public:
	static eventCollector *getInstance() noexcept;
};
