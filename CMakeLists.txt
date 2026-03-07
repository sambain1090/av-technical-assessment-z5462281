cmake_minimum_required(VERSION 3.8)
project(nmp25)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -lpthread)
endif()

foreach(IDX RANGE 1 3 1)
	add_executable(Question-${IDX}-out Q${IDX}/Question-${IDX}.cc)
	target_link_libraries(Question-${IDX}-out pthread)
	add_test(question-${IDX} Question-${IDX}-out)
endforeach()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(Question-2-test Q2/Question-2.test.cc)
target_compile_definitions(Question-2-test PRIVATE TESTING)
target_link_libraries(Question-2-test GTest::gtest_main pthread)  
