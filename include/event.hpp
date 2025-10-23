
#include <config.hpp>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

#pragma once

// define copyable type in concept.
template <typename T>
concept EventMemCopyable =
	std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> && std::is_aggregate_v<T> &&
	!std::is_polymorphic_v<T> && sizeof( T ) <= CONFIG_EVENT_SIZE_MAX;


class EventIntf {
public:
	EventIntf()			 = default;
	virtual ~EventIntf() = default;

	virtual std::span<const std::byte> getEventInRaw() = 0;
};

template <EventMemCopyable T>
// requires std::is_standard_layout_v<T>
class Event final : public EventIntf {
	struct __attribute__( ( packed ) ) EventPayload {
		T param;
	} evtPayload;

public:
	Event()	 = default;
	~Event() = default;

	T *getParam() { return &evtPayload.param; }

	std::span<const std::byte> getEventInRaw() {
		return std::as_bytes( std::span<EventPayload>( &evtPayload, 1 ) );
	}
};

// Define time spacific concept for event
template <typename U> struct is_event_t : std::false_type {};

// Define time spacific concept for event
template <typename T> struct is_event_t<Event<T>> : std::true_type {};

template <typename U> inline constexpr bool is_event_v = is_event_t<U>::value;

template <typename U>
concept IsEventType = is_event_v<U>;
