#ifndef INCLUDE_TOYTK_DETAIL_LOW_EVENT_HXX_K49QPG8E
#define INCLUDE_TOYTK_DETAIL_LOW_EVENT_HXX_K49QPG8E

#include <cstdint>
#include <variant>

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <linux/input-event-codes.h>

namespace toytk::detail
{
    struct PointerEnterLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	std::uint32_t serial;
	double x, y;
	xkb_state *state;
    };

    struct PointerLeaveLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	std::uint32_t serial;
	xkb_state *state;
    };

    struct PointerMotionLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	double x, y;
	xkb_state *state;
    };

    struct PointerButtonLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	std::uint32_t serial;
	std::uint32_t button;
	std::uint32_t button_state;
	double x, y;
	xkb_state *state;
    };

    struct PointerAxisLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	double horizontal_scroll;
	double vertical_scroll;
	double x, y;
	xkb_state *state;
    };

    struct KeyboardEnterLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	std::uint32_t serial;
	xkb_state *state;
    };

    struct KeyboardLeaveLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	std::uint32_t serial;
	xkb_state *state;
    };

    struct KeyboardKeyLowEvent
    {
	wl_surface *surface;
	wl_seat *seat;
	std::uint32_t serial;
	xkb_keysym_t key;
	std::uint32_t key_state;
	xkb_state *state;
    };

    using LowEvent = std::variant<
	PointerEnterLowEvent,
	PointerLeaveLowEvent,
	PointerMotionLowEvent,
	PointerButtonLowEvent,
	PointerAxisLowEvent,
	KeyboardEnterLowEvent,
	KeyboardLeaveLowEvent,
	KeyboardKeyLowEvent>;
}

#endif
