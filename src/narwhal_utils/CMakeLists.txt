

set(UTIL_SRCS
narwhal_arrays.c	narwhal_buffers.c	narwhal_unicode.c
narwhal_utils.c		narwhal_utils.h	
)

add_library(narwhal_utils ${UTIL_SRCS})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libnarwhal_utils.a DESTINATION lib)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/narwhal_utils.h DESTINATION include)
