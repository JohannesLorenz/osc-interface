# build the main library

include_directories(../include/rtosc/include)
include_directories(../include/ringbuffer/include)
add_definitions(-fPIC -Wall -Wextra -Weffc++ -Werror)
add_library(spa STATIC ${rtosc_lib_src} ${rtosc_lib_hdr} ${ringbuffer_lib_src} ${ringbuffer_lib_hdr})
install(TARGETS spa
	EXPORT spa-export
	ARCHIVE DESTINATION ${INSTALL_LIB_DIR})
