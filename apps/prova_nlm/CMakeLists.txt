add_executable(prova_nlm prova_nlm.cpp)

set_target_properties(prova_nlm PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(prova_nlm PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(prova_nlm yocto yocto_gui yocto_pathtrace yocto_extension)
