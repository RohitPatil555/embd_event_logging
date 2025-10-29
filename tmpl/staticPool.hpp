// SPDX-License-Identifier: MIT | Author: Rohit Patil

#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <type_traits>

template <typename T, std::size_t N>
	requires( !std::is_polymorphic_v<T> )
class StaticPool {
public:
	StaticPool();
	T *allocate() noexcept;
	void release( T *ptr ) noexcept;
	std::size_t usedCount() noexcept;

private:
	std::array<T, N> pool;
	std::bitset<N> used{};
};

// include template implementation
#include <staticPool.tpp>
