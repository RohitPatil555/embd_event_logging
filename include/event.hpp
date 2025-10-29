// SPDX-License-Identifier: MIT | Author: Rohit Patil

#pragma once

/* --------------------------------------------------------------------------
 *  Standard library headers
 * -------------------------------------------------------------------------- */
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

/* --------------------------------------------------------------------------
 *  Project headers
 * -------------------------------------------------------------------------- */
#include <config.hpp>

/* --------------------------------------------------------------------------
 *  Concept: EventMemCopyable
 *
 *  A type T is considered an EventMemCopyable if it satisfies the following:
 *      • Standard layout (no virtual functions, consistent memory layout)
 *      • Trivially copyable (can be mem‑copied safely)
 *      • Aggregate (plain struct/array with no constructors)
 *      • Non‑polymorphic
 *      • Size does not exceed CONFIG_EVENT_SIZE_MAX
 *
 *  These constraints guarantee that the event can be stored and transmitted as a raw byte buffer.
 * -------------------------------------------------------------------------- */
template <typename T>
concept EventMemCopyable =
	std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> && std::is_aggregate_v<T> &&
	!std::is_polymorphic_v<T> && sizeof( T ) <= CONFIG_EVENT_SIZE_MAX;

/* Forward declaration of EventId.
 *   EventId<T>::value must provide a unique 32‑bit identifier for the payload type T. */
template <typename T> struct EventId;

/* --------------------------------------------------------------------------
 *  Abstract interface for all events
 *
 *  All concrete event classes derive from this and must implement:
 *      • getEventInRaw() – returns a byte view of the entire event object.
 *      • setTimestamp(uint64_t) – records when the event was generated/received.
 * -------------------------------------------------------------------------- */
class EventIntf {
public:
	EventIntf()			 = default;
	virtual ~EventIntf() = default;

	virtual std::span<const std::byte> getEventInRaw() = 0;
	virtual void setTimestamp( uint64_t ts )		   = 0;
};

/* --------------------------------------------------------------------------
 *  Concrete Event implementation
 *
 *  T must satisfy EventMemCopyable.
 *  The payload structure is packed to avoid any compiler‑added padding.
 * -------------------------------------------------------------------------- */
template <EventMemCopyable T> class Event final : public EventIntf {
	/* Packed event payload that will be serialised as raw bytes. */
	struct __attribute__( ( packed ) ) EventPayload {
		uint32_t id;
		uint64_t timestamp;
		T param;
	} evtPayload;

public:
	/* Constructor initialises the id field from EventId<T> */
	Event() { evtPayload.id = EventId<T>::value; }
	~Event() = default;

	/* Return a pointer to the payload so callers can read/write it. */
	T *getParam() { return &evtPayload.param; }

	/* Implement the interface: expose the whole event as a byte span. */
	std::span<const std::byte> getEventInRaw() {
		return std::as_bytes( std::span<EventPayload>( &evtPayload, 1 ) );
	}

	/* Store the timestamp in the packed payload. */
	void setTimestamp( uint64_t ts ) { evtPayload.timestamp = ts; }
};

/* --------------------------------------------------------------------------
 *  Type trait and concept to recognise event types
 *
 *  is_event_t<T> is true if T is an instantiation of Event<…>.
 *  IsEventType<U> can be used in generic code to constrain templates.
 * -------------------------------------------------------------------------- */
template <typename U> struct is_event_t : std::false_type {};
template <typename T> struct is_event_t<Event<T>> : std::true_type {};
template <typename U> inline constexpr bool is_event_v = is_event_t<U>::value;

template <typename U>
concept IsEventType = is_event_v<U>;
