add_executable(yimagedenoise yimagedenoise.cpp)

set_target_properties(yimagedenoise PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(yimagedenoise PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(yimagedenoise yocto yocto_gui yocto_pathtrace yocto_extension)
