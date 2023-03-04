project(tracker)

add_library(tracker SHARED
  tracker.cpp
)

set_property(TARGET tracker PROPERTY CXX_STANDARD 17)
