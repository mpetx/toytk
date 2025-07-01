
#include <toytk/core.hxx>

#include <wayland-client.h>

#include <algorithm>
#include <utility>

namespace toytk
{
    namespace detail
    {
	Display &application_get_display(Application &this_)
	{
	    return this_.m_display;
	}

	void ApplicationLowEventHandlerVisitor::operator()(const PointerEnterLowEvent &levent) const
	{
	    auto win = application.get_window_from_surface(levent.surface);

	    if (!win.has_value())
	    {
		return;
	    }

	    auto root = win->get().get_root();

	    if (!root.has_value())
	    {
		return;
	    }

	    PointerEnterEvent event { levent.x, levent.y, levent.state };

	    win->get().m_hover_stacks.erase(levent.seat);
	    win->get().m_foci.erase(levent.seat);
	    win->get().m_grab.erase(levent.seat);

	    auto hover_stack = root->get().get_widget_stack_below(event.x, event.y);

	    for (auto widget : hover_stack)
	    {
		widget.get().handle_event(win->get(), event);
	    }

	    win->get().m_hover_stacks.emplace(levent.seat, std::move(hover_stack));
	}

	void ApplicationLowEventHandlerVisitor::operator()(const PointerLeaveLowEvent &levent) const
	{
	    auto win = application.get_window_from_surface(levent.surface);

	    if (!win.has_value())
	    {
		return;
	    }

	    auto root = win->get().get_root();

	    if (!root.has_value())
	    {
		return;
	    }

	    PointerLeaveEvent event { levent.state };

	    win->get().m_grab.erase(levent.seat);

	    auto i = win->get().m_hover_stacks.find(levent.seat);

	    if (i == win->get().m_hover_stacks.end())
	    {
		return;
	    }

	    auto &hover_stack = i->second;

	    for (auto j = hover_stack.rbegin(); j != hover_stack.rend(); ++j)
	    {
		j->get().handle_event(win->get(), event);
	    }

	    win->get().m_hover_stacks.erase(i);
	}

	template <typename Iter, typename Sent>
	static void emit_leave_event_for_each(Iter i, Sent s, Window &win, const PointerLeaveEvent &e)
	{
	    if (i != s)
	    {
		Widget &widget = i->get();
		emit_leave_event_for_each(++i, s, win, e);
		widget.handle_event(win, e);
	    }
	}

	void ApplicationLowEventHandlerVisitor::operator()(const PointerMotionLowEvent &levent) const
	{
	    auto win = application.get_window_from_surface(levent.surface);

	    if (!win.has_value())
	    {
		return;
	    }

	    auto root = win->get().get_root();

	    if (!root.has_value())
	    {
		return;
	    }

	    PointerLeaveEvent leave_event { levent.state };
	    PointerMotionEvent motion_event { levent.x, levent.y, levent.state };
	    PointerEnterEvent enter_event { levent.x, levent.y, levent.state };

	    auto grab_i = win->get().m_grab.find(levent.seat);

	    if (grab_i != win->get().m_grab.end())
	    {
		grab_i->second.get().handle_event(win->get(), motion_event);
		return;
	    }

	    auto i = win->get().m_hover_stacks.find(levent.seat);

	    if (i == win->get().m_hover_stacks.end())
	    {
		return;
	    }

	    auto &old_hover_stack = i->second;
	    auto new_hover_stack = root->get().get_widget_stack_below(levent.x, levent.y);

	    auto [new_i, old_i] = std::ranges::mismatch(new_hover_stack, old_hover_stack, [](auto x, auto y) {
		return &x.get() == &y.get();
	    });

	    emit_leave_event_for_each(old_i, old_hover_stack.end(), win->get(), leave_event);

	    if (auto j = new_i; j == new_hover_stack.end() && j != new_hover_stack.begin())
	    {
		(--j)->get().handle_event(win->get(), motion_event);
	    }

	    for (auto j = new_i; j != new_hover_stack.end(); ++j)
	    {
		j->get().handle_event(win->get(), enter_event);
	    }

	    old_hover_stack = std::move(new_hover_stack);
	}

	void ApplicationLowEventHandlerVisitor::emit_blur_focus_events(Window &win, Widget &widget, wl_seat *seat, xkb_state *state) const
	{
	    auto i = win.m_foci.find(seat);

	    BlurEvent blur_event { state };
	    FocusEvent focus_event { state };

	    if (i == win.m_foci.end())
	    {
		win.m_foci.emplace(seat, widget);
		widget.handle_event(win, focus_event);
	    }
	    else
	    {
		Widget &old_focus = i->second.get();

		if (&old_focus != &widget)
		{
		    i->second = widget;
		    old_focus.handle_event(win, blur_event);
		    widget.handle_event(win, focus_event);
		}
	    }
	}

	void ApplicationLowEventHandlerVisitor::operator()(const PointerButtonLowEvent &levent) const
	{
	    auto win = application.get_window_from_surface(levent.surface);

	    if (!win.has_value())
	    {
		return;
	    }

	    auto i = win->get().m_hover_stacks.find(levent.seat);

	    if (i == win->get().m_hover_stacks.end())
	    {
		return;
	    }

	    auto &hover_stack = i->second;
	    Widget &widget = hover_stack[hover_stack.size() - 1].get();
   
	    if (levent.button_state == WL_POINTER_BUTTON_STATE_RELEASED)
	    {
		ButtonUpEvent event { { levent.seat, levent.serial }, levent.button, levent.x, levent.y, levent.state };
		widget.handle_event(win->get(), event);

		if (levent.button == BTN_LEFT)
		{
		    win->get().m_grab.erase(levent.seat);
		}
	    }
	    else if (levent.button_state == WL_POINTER_BUTTON_STATE_PRESSED)
	    {
		ButtonDownEvent event { { levent.seat, levent.serial }, levent.button, levent.x, levent.y, levent.state };
		widget.handle_event(win->get(), event);

		if (levent.button == BTN_LEFT)
		{
		    emit_blur_focus_events(win->get(), widget, levent.seat, levent.state);
		    win->get().m_grab.emplace(levent.seat, widget);
		}
	    }
	}

	void ApplicationLowEventHandlerVisitor::operator()(const PointerAxisLowEvent &levent) const
	{
	    auto win = application.get_window_from_surface(levent.surface);

	    if (!win.has_value())
	    {
		return;
	    }

	    PointerAxisEvent event {
		levent.horizontal_scroll,
		levent.vertical_scroll,
		levent.x, levent.y,
		levent.state
	    };

	    auto grab_i = win->get().m_grab.find(levent.seat);

	    if (grab_i != win->get().m_grab.end())
	    {
		grab_i->second.get().handle_event(win->get(), event);
		return;
	    }

	    auto i = win->get().m_hover_stacks.find(levent.seat);

	    if (i == win->get().m_hover_stacks.end())
	    {
		return;
	    }

	    auto &hover_stack = i->second;
	    Widget &widget = hover_stack[hover_stack.size() - 1].get();

	    widget.handle_event(win->get(), event);
	}

	void ApplicationLowEventHandlerVisitor::operator()(const KeyboardEnterLowEvent &levent) const
	{
	}

	void ApplicationLowEventHandlerVisitor::operator()(const KeyboardLeaveLowEvent &levent) const
	{
	}

	void ApplicationLowEventHandlerVisitor::operator()(const KeyboardKeyLowEvent &levent) const
	{
	    auto win = application.get_window_from_surface(levent.surface);

	    auto i = win->get().m_foci.find(levent.seat);

	    if (i == win->get().m_foci.end())
	    {
		return;
	    }

	    if (levent.key_state == WL_KEYBOARD_KEY_STATE_RELEASED)
	    {
		KeyUpEvent event { { levent.seat, levent.serial }, levent.key, levent.state };
		i->second.get().handle_event(win->get(), event);
	    }
	    else
	    {
		KeyDownEvent event { { levent.seat, levent.serial }, levent.key, levent.state };
		i->second.get().handle_event(win->get(), event);
	    }
	}
    }

    Application::Application(std::string_view cls)
	: m_display { wl_display_connect(nullptr) },
	  m_class { cls }
    {
	initialize();

	m_display.on_low_event_func = [](const detail::LowEvent &event, void *data) {
	    Application &this_ = *reinterpret_cast<Application *>(data);
	    detail::ApplicationLowEventHandlerVisitor visitor { this_ };
	    std::visit(visitor, event);

	    this_.m_zombie_widgets.clear();

	    for (auto i = this_.m_windows.begin(); i != this_.m_windows.end();)
	    {
		if (i->second->get_should_close())
		{
		    i = this_.m_windows.erase(i);
		}
		else
		{
		    if (i->second->get_redraw_needed())
		    {
			i->second->redraw();
		    }

		    ++i;
		}
	    }
	};
	m_display.on_low_event_data = this;

	m_display.on_seat_delete_func = [](wl_seat *seat, void *data) {
	    Application &this_ = *reinterpret_cast<Application *>(data);

	    for (auto &pair : this_.m_windows)
	    {
		pair.second->handle_seat_delete(seat);
	    }
	};
	m_display.on_seat_delete_data = this;
    }

    static wl_display *wl_display_connect_to_string_view(std::string_view name_sv)
    {
	std::string name { name_sv };
	return wl_display_connect(name.c_str());
    }

    Application::Application(std::string_view cls, std::string_view name)
	: m_display { wl_display_connect_to_string_view(name) }
    {
	initialize();
    }

    Application::Application(std::string_view cls, int fd)
	: m_display { wl_display_connect_to_fd(fd) },
	  m_class { cls }
    {
	initialize();
    }

    void Application::initialize()
    {
    }

    const std::string &Application::get_class() const
    {
	return m_class;
    }

    Application::operator bool() const
    {
	return static_cast<bool>(m_display);
    }

    void Application::dispatch_events()
    {
 	m_display.dispatch_low_events();
    }

    std::reference_wrapper<Window> Application::create_window(std::string_view title)
    {
	std::unique_ptr<Window> p { new Window { *this, title } };
	Window *rp = p.get();

	wl_surface *surface = p->m_surface.get();
	m_windows.emplace(surface, std::move(p));

	return *rp;
    }

    bool Application::has_window() const
    {
	return !m_windows.empty();
    }

    std::optional<std::reference_wrapper<Window>> Application::get_window_from_surface(wl_surface *surface)
    {
	auto i = m_windows.find(surface);

	if (i == m_windows.end())
	{
	    return std::nullopt;
	}
	else
	{
	    return *i->second;
	}
    }

    void Application::add_zombie_widget(PmrPtr<Widget> &&widget)
    {
	m_zombie_widgets.push_back(std::move(widget));
    }
}
