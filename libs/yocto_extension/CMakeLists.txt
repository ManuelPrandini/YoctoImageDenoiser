add_library(yocto_extension yocto_extension.h yocto_extension.cpp ext/image_io.h ext/image_io.cpp)

find_library(OIDN_LIBRARIES NAMES OpenImageDenoise PATHS ${CMAKE_SOURCE_DIR}/libs/oidn/lib)
find_library(OIIO_LIBRARIES NAMES OpenImageIO OpenImageIO_Util)
set_target_properties(yocto_extension PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(yocto_extension PUBLIC ${CMAKE_SOURCE_DIR}/libs ${CMAKE_SOURCE_DIR}/libs/oidn/include )
#${CMAKE_SOURCE_DIR}/libs/oiio/include) 
target_link_libraries(yocto_extension yocto yocto_pathtrace ${OIDN_LIBRARIES} ${OIIO_LIBRARIES})
