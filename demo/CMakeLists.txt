
set(ToyTkDemoSources
  main.cxx)
list(TRANSFORM ToyTkDemoSources PREPEND src/)

add_executable(ToyTkDemo
  ${ToyTkDemoSources})

target_link_libraries(ToyTkDemo
  PRIVATE
  ToyTk)

set_target_properties(ToyTkDemo
  PROPERTIES
  OUTPUT_NAME toytk-demo)

install(TARGETS ToyTkDemo)
