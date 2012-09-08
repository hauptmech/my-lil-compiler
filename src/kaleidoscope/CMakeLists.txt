set(SRCS
kscope.c	kscope.h
kaleidoscope.cpp

	)


execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --libfiles OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)



add_executable(kaleidoscope ${SRCS})

set_target_properties(kaleidoscope PROPERTIES COMPILE_FLAGS "-g3  ${LLVM_CXXFLAGS} ")

target_link_libraries(kaleidoscope "${LLVM_LDFLAGS} ${LLVM_LIBS}")


install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.ks DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
