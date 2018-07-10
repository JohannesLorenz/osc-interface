# Rather than cmake-including sub-projects,
# we grab their files on our own here

set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB rtosc_lib_src ${src_dir}/rtosc/src/*.cpp)
file(GLOB rtosc_lib_hdr ${src_dir}/rtosc/include/rtosc/*.h)

install(FILES ${rtosc_lib_hdr} DESTINATION include/rtosc)

file(GLOB ringbuffer_lib_src ${src_dir}/src/lib/*.cpp)
file(GLOB ringbuffer_lib_hdr ${src_dir}/include/ringbuffer/*.h)
install(FILES ${ringbuffer_lib_hdr} DESTINATION include/ringbuffer)

# add_library(ringbuffer STATIC ${ringbuffer_lib_src} ${ringbuffer_lib_hdr})

file(GLOB ringbuffer_lib_src ringbuffer/src/lib/*.cpp)
file(GLOB ringbuffer_lib_hdr ringbuffer/include/ringbuffer/*.h)
set(rtosc_lib_src ${rtosc_lib_src} PARENT_SCOPE)
set(rtosc_lib_hdr ${rtosc_lib_hdr} PARENT_SCOPE)

set(ringbuffer_lib_src ${ringbuffer_lib_src} PARENT_SCOPE)
set(ringbuffer_lib_hdr ${ringbuffer_lib_hdr} PARENT_SCOPE)



install(FILES spa/spa_fwd.h spa/spa.h spa/audio_fwd.h spa/audio.h DESTINATION include/spa)



