
#include <toytk/event.hxx>

namespace toytk
{
    bool is_bubbling_event(const Event &event)
    {
	return std::get_if<PointerMotionEvent>(&event)
	    || std::get_if<ButtonDownEvent>(&event)
	    || std::get_if<ButtonUpEvent>(&event)
	    || std::get_if<PointerAxisEvent>(&event)
	    || std::get_if<KeyDownEvent>(&event)
	    || std::get_if<KeyUpEvent>(&event);
    }
}
