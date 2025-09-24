# tests/CMakeLists.txt
enable_testing()

find_package(GTest REQUIRED)          # gtest/gmock via vcpkg, Conan or system

#add_executable(tests)
#target_link_libraries(tests PRIVATE embdEventLog GTest::gtest GTest::gmock)

# Add coverage if enabled
if(ENABLE_COVERAGE)
    #target_compile_options(tests PRIVATE --coverage -O0)
endif()

#add_test(NAME unit_tests COMMAND tests)

