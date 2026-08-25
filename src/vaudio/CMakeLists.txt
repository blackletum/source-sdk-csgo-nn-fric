add_library(vaudio_miles SHARED vaudio_minimp3.cpp)
target_link_libraries(vaudio_miles PRIVATE interface SDL3::SDL3-static)
