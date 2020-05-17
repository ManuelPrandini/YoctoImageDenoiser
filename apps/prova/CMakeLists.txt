add_executable(prova prova.cpp)

set_target_properties(prova PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(prova PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(prova yocto yocto_gui yocto_pathtrace yocto_extension)
