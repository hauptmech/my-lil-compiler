set(PG_SRCS
c_generator.c	c_generator.h
internal.c	internal.h
parsergen.c	parsergen.h
peg_parser.c	peg_parser.h
)
include_directories(${PROJECT_SOURCE_DIR}/src/narwhal_utils)
link_directories(${PROJECT_BINARY_DIR}/lib)

add_executable(parsergen ${PG_SRCS})

target_link_libraries(parsergen narwhal_utils)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/parsergen DESTINATION bin)
