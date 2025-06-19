#ifndef INCLUDE_TOYTK_DETAIL_DISPLAY_HXX_R487Y5LE
#define INCLUDE_TOYTK_DETAIL_DISPLAY_HXX_R487Y5LE

#include <toytk/detail/low-event.hxx>
#include <toytk/detail/output.hxx>
#include <toytk/detail/seat.hxx>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

namespace toytk::detail
{
    struct WlDisplayDelete
    {
	void operator()(wl_display *) const;
    };

    using WlDisplayPtr = std::unique_ptr<wl_display, WlDisplayDelete>;

    struct WlRegistryDelete
    {
	void operator()(wl_registry *) const;
    };

    using WlRegistryPtr = std::unique_ptr<wl_registry, WlRegistryDelete>;

    struct WlCompositorDelete
    {
	void operator()(wl_compositor *) const;
    };

    using WlCompositorPtr = std::unique_ptr<wl_compositor, WlCompositorDelete>;

    struct WlShmDelete
    {
	void operator()(wl_shm *) const;
    };

    using WlShmPtr = std::unique_ptr<wl_shm, WlShmDelete>;

    struct WlShellDelete
    {
	void operator()(wl_shell *) const;
    };

    using WlShellPtr = std::unique_ptr<wl_shell, WlShellDelete>;

    struct XkbContextDelete
    {
	void operator()(xkb_context *) const;
    };

    using XkbContextPtr = std::unique_ptr<xkb_context, XkbContextDelete>;

    class Output;
    class Seat;

    class Display
    {
	WlDisplayPtr m_display;
	WlRegistryPtr m_registry;
	WlCompositorPtr m_compositor;
	WlShmPtr m_shm;
	WlShellPtr m_shell;
	std::unordered_map<std::uint32_t, std::unique_ptr<Output>> m_outputs;
	std::unordered_map<std::uint32_t, std::unique_ptr<Seat>> m_seats;
	XkbContextPtr m_xkb_context;
	bool m_error = false;

	std::unordered_set<std::uint32_t> m_shm_formats;

	void initialize(wl_display *);

	static void handle_registry_global(void *, wl_registry *, std::uint32_t, const char *, std::uint32_t);
	static void handle_registry_global_remove(void *, wl_registry *, std::uint32_t);
	static const wl_registry_listener registry_listener;

	void initialize_compositor(std::uint32_t);

	void initialize_shm(std::uint32_t);
	static void handle_shm_format(void *, wl_shm *, std::uint32_t);
	static const wl_shm_listener shm_listener;

	void initialize_shell(std::uint32_t);

	void initialize_output(std::uint32_t);
	void finalize_output(std::uint32_t);

	void initialize_seat(std::uint32_t);
	void finalize_seat(std::uint32_t);

    public:

	void (*on_low_event_func)(const LowEvent &, void *) = nullptr;
	void *on_low_event_data = nullptr;

	void (*on_seat_delete_func)(wl_seat *, void *) = nullptr;
	void *on_seat_delete_data = nullptr;

	explicit Display(wl_display *disp);

	Display(const Display &) = delete;
	Display(Display &&) = delete;
	Display &operator=(const Display &) = delete;
	Display &operator=(Display &&) = delete;

	wl_display *get_display() const;
	wl_registry *get_registry() const;
	wl_compositor *get_compositor() const;
	wl_shm *get_shm() const;
	wl_shell *get_shell() const;
	const std::unordered_map<std::uint32_t, std::unique_ptr<Output>> &get_outputs() const;
	const std::unordered_map<std::uint32_t, std::unique_ptr<Seat>> &get_seats() const;
	xkb_context *get_xkb_context() const;

	const std::unordered_set<std::uint32_t> &get_shm_formats() const;

	void handle_low_event(const LowEvent &);

	void handle_seat_delete(wl_seat *);

	explicit operator bool() const;

	void dispatch_low_events();
    };
}

#endif
