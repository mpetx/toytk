
#include <toytk/prelude.hxx>

#include <iostream>
#include <format>

static toytk::Bubbling hello_button_event_handler(toytk::Widget &, toytk::Window &, const toytk::Event &event)
{
    auto p = std::get_if<toytk::ButtonDownEvent>(&event);

    if (!p || p->button != BTN_LEFT)
    {
	return toytk::Bubbling::bubble;
    }

    std::cout << "hello, world" << std::endl;

    return toytk::Bubbling::stop;
}

static toytk::Bubbling quit_button_event_handler(toytk::Widget &, toytk::Window &win, const toytk::Event &event)
{
    auto p = std::get_if<toytk::ButtonDownEvent>(&event);

    if (!p || p->button != BTN_LEFT)
    {
	return toytk::Bubbling::bubble;
    }

    win.set_should_close();

    return toytk::Bubbling::stop;
}

int main()
{
    toytk::Application app { "toytk-demo" };

    if (!app)
    {
	std::cout << "fail" << std::endl;
	return 1;
    }

    cairo_font_face_t *serif_face = cairo_toy_font_face_create("serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    toytk::Font serif { serif_face, 18 };
    cairo_font_face_destroy(serif_face);
    serif_face = nullptr;

    cairo_font_face_t *sans_face = cairo_toy_font_face_create("sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    toytk::Font sans { sans_face, 18 };
    cairo_font_face_destroy(sans_face);
    sans_face = nullptr;

    std::pmr::memory_resource *mr = std::pmr::new_delete_resource();
    toytk::PmrDelete<toytk::Widget> del { mr };
    std::pmr::polymorphic_allocator alloc { mr };

    auto hello_button = alloc.new_object<toytk::Button>();
    hello_button->set_text("Hello");
    hello_button->set_font(sans);
    hello_button->add_event_handler(hello_button_event_handler);
    toytk::PmrPtr<toytk::Widget> owned_hello_button { hello_button, del };

    auto hello_pbox = alloc.new_object<toytk::PaddingBox>();
    hello_pbox->set_top_padding(8);
    hello_pbox->set_bottom_padding(8);
    hello_pbox->set_left_padding(8);
    hello_pbox->set_right_padding(8);
    hello_pbox->set_content(std::move(owned_hello_button));
    toytk::PmrPtr<toytk::Widget> owned_hello_pbox { hello_pbox, del };

    auto quit_button = alloc.new_object<toytk::Button>();
    quit_button->set_text("Quit");
    quit_button->set_font(sans);
    quit_button->add_event_handler(quit_button_event_handler);
    toytk::PmrPtr<toytk::Widget> owned_quit_button { quit_button, del };

    auto quit_pbox = alloc.new_object<toytk::PaddingBox>();
    quit_pbox->set_bottom_padding(8);
    quit_pbox->set_left_padding(8);
    quit_pbox->set_right_padding(8);
    quit_pbox->set_content(std::move(owned_quit_button));
    toytk::PmrPtr<toytk::Widget> owned_quit_pbox { quit_pbox, del };

    auto vbox = alloc.new_object<toytk::VerticalBox>();
    vbox->add_child(std::move(owned_hello_pbox));
    vbox->add_child(std::move(owned_quit_pbox));
    toytk::PmrPtr<toytk::Widget> owned_vbox { vbox, del };

    auto frame = alloc.new_object<toytk::Frame>(true);
    frame->set_title("ToyTk Demo");
    frame->set_font(serif);
    frame->set_content(std::move(owned_vbox));
    auto frame_dim = frame->get_preferred_dimension();
    frame_dim.width = std::max(160, frame_dim.width);
    frame->set_dimension(frame_dim);
    toytk::PmrPtr<toytk::Widget> owned_frame { frame, del };

    auto win = app.create_window("ToyTk Demo");
    win.get().set_root(std::move(owned_frame));
    win.get().set_minimal_dimension(frame_dim);

    while (app && app.has_window())
    {
	app.dispatch_events();
    }

    std::cout << "done" << std::endl;

    return 0;
}
