set(SRCS
gram.c	gram.h
grammar.cpp

	)


execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE LLVM_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config --libfiles OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)



add_executable(grammar ${SRCS})

set_target_properties(grammar PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")

target_link_libraries(grammar "${LLVM_LDFLAGS} ${LLVM_LIBS}")


#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.grammar DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
