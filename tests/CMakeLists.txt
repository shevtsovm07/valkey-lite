include(FetchContent)


FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(valkey_tests
  string_test.cpp
  set_test.cpp
  list_test.cpp
  geo_test.cpp
  common_test.cpp
)

target_link_libraries(valkey_tests
  PRIVATE
  valkey_lib
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(valkey_tests)