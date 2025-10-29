// SPDX-License-Identifier: MIT | Author: Rohit Patil
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

// Concept: Only pointer types are allowed
template <typename T>
concept PointerType = std::is_pointer_v<T>;

template <PointerType T, std::size_t N> class Queue {
	static_assert( N > 0, "Queue size must be greater than 0" );

private:
	T buffer[ N ];
	std::size_t head  = 0;
	std::size_t tail  = 0;
	std::size_t count = 0;

public:
	constexpr Queue() {
		head  = 0;
		tail  = 0;
		count = 0;

		memset( buffer, 0, sizeof( buffer ) );
	}

	constexpr bool insert( T item ) noexcept {
		if ( isFull() ) {
			return false;
		}
		buffer[ tail ] = item;
		tail		   = ( tail + 1 ) % N;
		++count;
		return true;
	}

	constexpr std::optional<T> remove() noexcept {
		if ( isEmpty() ) {
			return std::nullopt;
		}
		T item = buffer[ head ];
		head   = ( head + 1 ) % N;
		--count;
		return item;
	}

	constexpr bool isEmpty() const noexcept { return count == 0; }

	constexpr bool isFull() const noexcept { return count == N; }

	constexpr std::size_t size() const noexcept { return count; }
};
