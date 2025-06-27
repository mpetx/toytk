#ifndef INCLUDE_TOYTK_CORE_HXX_C874VGTW
#define INCLUDE_TOYTK_CORE_HXX_C874VGTW

#include <toytk/common.hxx>
#include <toytk/event.hxx>
#include <toytk/detail/display.hxx>
#include <toytk/detail/buffer.hxx>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <cairo/cairo.h>

namespace toytk
{
    class Application;
    class Window;
    class Widget;

    namespace detail
    {
	Display &application_get_display(Application &);

	struct ApplicationLowEventHandlerVisitor
	{
	    Application &application;

	    void emit_blur_focus_events(Window &, Widget &, wl_seat *, xkb_state *) const;

	    void operator()(const detail::PointerEnterLowEvent &) const;
	    void operator()(const detail::PointerLeaveLowEvent &) const;
	    void operator()(const detail::PointerMotionLowEvent &) const;
	    void operator()(const detail::PointerButtonLowEvent &) const;
	    void operator()(const detail::PointerAxisLowEvent &) const;
	    void operator()(const detail::KeyboardEnterLowEvent &) const;
	    void operator()(const detail::KeyboardLeaveLowEvent &) const;
	    void operator()(const detail::KeyboardKeyLowEvent &) const;
	};
    }

    class Application
    {
	detail::Display m_display;
	std::string m_class;
	std::unordered_map<wl_surface *, std::unique_ptr<Window>> m_windows;
	void initialize();

	friend detail::Display &detail::application_get_display(Application &);

	friend detail::ApplicationLowEventHandlerVisitor;

    public:

	Application(std::string_view);
	explicit Application(std::string_view, std::string_view);
	explicit Application(std::string_view, int);

	Application(const Application &) = delete;
	Application(Application &&) = delete;
	Application &operator=(const Application &) = delete;
	Application &operator=(Application &&) = delete;

	const std::string &get_class() const;

	operator bool() const;

	void dispatch_events();

	std::reference_wrapper<Window> create_window(std::string_view);
	bool has_window() const;
	std::optional<std::reference_wrapper<Window>> get_window_from_surface(wl_surface *);
    };

    namespace detail
    {
	struct WlSurfaceDelete
	{
	    void operator()(wl_surface *p) const;
	};

	using WlSurfacePtr = std::unique_ptr<wl_surface, WlSurfaceDelete>;

	struct WlShellSurfaceDelete
	{
	    void operator()(wl_shell_surface *p) const;
	};

	using WlShellSurfacePtr = std::unique_ptr<wl_shell_surface, WlShellSurfaceDelete>;

	struct CairoSurfaceDelete
	{
	    void operator()(cairo_surface_t *p) const;
	};

	using CairoSurfacePtr = std::unique_ptr<cairo_surface_t, CairoSurfaceDelete>;

	struct CairoDelete
	{
	    void operator()(cairo_t *p) const;
	};

	using CairoPtr = std::unique_ptr<cairo_t, CairoDelete>;
    }

    class Window
    {
	Application &m_application;
	std::string m_title;
	detail::WlSurfacePtr m_surface;
	detail::WlShellSurfacePtr m_shell_surface;
	wl_output *m_output;
	bool m_should_close = false;

	Dimension m_minimal_dimension { 1, 1 };

	detail::Buffer m_buffers[2];
	bool m_buffers_busy[2] { false, false };
	bool m_redraw_needed = false;

	PmrPtr<Widget> m_root;

	std::map<wl_seat *, std::vector<std::reference_wrapper<Widget>>> m_hover_stacks;
	std::map<wl_seat *, std::reference_wrapper<Widget>> m_foci;
	std::map<wl_seat *, std::reference_wrapper<Widget>> m_grab;

	Window(Application &, std::string_view);

	int get_free_buffer_index() const;

	static void handle_surface_enter(void *, wl_surface *, wl_output *);
	static void handle_surface_leave(void *, wl_surface *, wl_output *);
	static const wl_surface_listener surface_listener;

	static void handle_shell_surface_ping(void *, wl_shell_surface *, std::uint32_t);
	static void handle_shell_surface_configure(void *, wl_shell_surface *, std::uint32_t, std::int32_t, std::int32_t);
	static const wl_shell_surface_listener shell_surface_listener;

	static void handle_buffer_release(void *, wl_buffer *);
	static const wl_buffer_listener buffer_listener;

	friend Application;
	friend detail::ApplicationLowEventHandlerVisitor;

    public:

	Window(const Window &) = delete;
	Window(Window &&) = delete;
	Window &operator=(const Window &) = delete;
	Window &operator=(Window &&) = delete;

	Application &get_application() const;

	const std::string &get_title() const;
	void set_title(std::string_view);

	wl_output *get_output() const;

	bool get_should_close() const;
	void set_should_close();

	Dimension get_minimal_dimension() const;
	void set_minimal_dimension(const Dimension &);

	bool get_redraw_needed() const;
	void set_redraw_needed();

	std::optional<std::reference_wrapper<Widget>> get_root() const;
	void set_root(PmrPtr<Widget> &&);

	void redraw();

	void handle_seat_delete(wl_seat *);
	void notify_widget_removal(Widget &);

	void move(const Serial &);

	static constexpr unsigned int edges_top = 0b0001;
	static constexpr unsigned int edges_bottom = 0b0010;
	static constexpr unsigned int edges_left = 0b0100;
	static constexpr unsigned int edges_right = 0b1000;

	void resize(const Serial &, unsigned int);
    };

    namespace detail
    {
	void widget_prepare_for_owned(Widget &);

	struct BasicWidgetHandlerVisitor
	{
	    Widget &widget;

	    void operator()(const PointerEnterEvent &) const;
	    void operator()(const PointerLeaveEvent &) const;
	    void operator()(const ButtonDownEvent &) const;
	    void operator()(const ButtonUpEvent &) const;
	    void operator()(const FocusEvent &) const;
	    void operator()(const BlurEvent &) const;

	    template <typename E>
	    void operator()(const E &) const { }
	};
    }

    using EventHandler = Bubbling (*)(Widget &, Window &, const Event &);

    class Widget
    {
	Position m_position;
	Dimension m_dimension;
	std::optional<std::reference_wrapper<Widget>> m_parent;
	std::vector<EventHandler> m_event_handlers;
	int m_hover_count = 0;
	int m_activation_count = 0;
	int m_focus_count = 0;

	friend void detail::widget_prepare_for_owned(Widget &);
	friend detail::BasicWidgetHandlerVisitor;

    public:

	Widget();

	virtual ~Widget();

	Widget(const Widget &) = delete;
	Widget(Widget &&) = delete;
	Widget &operator=(const Widget &) = delete;
	Widget &operator=(Widget &&) = delete;

	const Position &get_position() const;
	void set_position(const Position &);
	const Dimension &get_dimension() const;
	void set_dimension(const Dimension &);

	std::optional<std::reference_wrapper<Widget>> get_parent() const;

	void own_child(PmrPtr<Widget> &);
	void unown_child(PmrPtr<Widget> &);

	bool is_hovered() const;
	bool is_active() const;
	bool is_focused() const;

	void add_event_handler(EventHandler);
	void remove_event_handler(EventHandler);

	virtual void layout() = 0;
	virtual void paint(cairo_t *) const = 0;
	virtual Dimension get_preferred_dimension() const = 0;
	virtual void for_each_child(void (*)(Widget &, void *), void *) = 0;

	bool contains_point(double, double) const;
	std::optional<std::reference_wrapper<Widget>> get_child_at(double, double);
	std::vector<std::reference_wrapper<Widget>> get_widget_stack_below(double, double);
	std::vector<std::reference_wrapper<Widget>> get_widget_stack_above();

	void draw_recursively(cairo_t *);

	void preferize_dimension();

	void handle_event(Window &, const Event &);
    };
}

#endif
