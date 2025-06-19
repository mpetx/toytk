
set(ToyTkSources
  detail/display.cxx
  detail/output.cxx
  detail/seat.cxx
  detail/buffer.cxx
  application.cxx
  window.cxx
  widget.cxx
  event.cxx
  font.cxx
  widget/frame.cxx
  widget/padding-box.cxx
  widget/vertical-box.cxx
  widget/label.cxx
  widget/button.cxx)
list(TRANSFORM ToyTkSources PREPEND src/)

set(ToyTkHeaders
  toytk/prelude.hxx
  toytk/detail/display.hxx
  toytk/detail/output.hxx
  toytk/detail/seat.hxx
  toytk/detail/low-event.hxx
  toytk/detail/buffer.hxx
  toytk/core.hxx
  toytk/common.hxx
  toytk/event.hxx
  toytk/font.hxx
  toytk/widget/frame.hxx
  toytk/widget/padding-box.hxx
  toytk/widget/vertical-box.hxx
  toytk/widget/label.hxx
  toytk/widget/button.hxx)
list(TRANSFORM ToyTkHeaders PREPEND include/)

add_library(ToyTk
  ${ToyTkSources})

target_sources(ToyTk
  PUBLIC
  FILE_SET toytk_headers
  TYPE HEADERS
  BASE_DIRS include
  FILES ${ToyTkHeaders})

target_include_directories(ToyTk
  PUBLIC
  ${CAIRO_INCLUDE_DIRS}
  ${WAYLAND_CLIENT_INCLUDE_DIRS}
  ${XKBCOMMON_INCLUDE_DIRS})

target_link_libraries(ToyTk
  PUBLIC
  ${CAIRO_LIBRARIES}
  ${WAYLAND_CLIENT_LIBRARIES}
  ${XKBCOMMON_LIBRARIES})

set_target_properties(ToyTk
  PROPERTIES
  OUTPUT_NAME toytk
  EXPORT_NAME ToyTk)

install(TARGETS ToyTk
  EXPORT ToyTk
  FILE_SET toytk_headers)
