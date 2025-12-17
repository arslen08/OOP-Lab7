add_executable(${PROJECT_NAME}_tests
    test.cpp
)

target_include_directories(${PROJECT_NAME}_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../includes
)

target_link_libraries(
  ${PROJECT_NAME}_tests
  PRIVATE
  ${PROJECT_NAME}_lib
  gtest_main
  Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)