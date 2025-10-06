# Add this in root CMakeLists.txt or a dedicated folder

add_executable(embdEventLog_fuzz fuzz.cpp)   # your fuzzer harness
target_link_libraries(embdEventLog_fuzz PRIVATE EmbdEvtLog)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(embdEventLog_fuzz PRIVATE -fsanitize=fuzzer-no-link,address,undefined)
    target_link_options(embdEventLog_fuzz PRIVATE -fsanitize=fuzzer-no-link,address,undefined)
endif()
