cmake_minimum_required (VERSION 2.6)
project (dbench)
EXEC_PROGRAM(/bin/date ARGS +'%a %b %d %Y' OUTPUT_VARIABLE DATE)
find_package(Boost REQUIRED COMPONENTS program_options thread)
set (dbench_VERSION_MAJOR 1)
set (dbench_VERSION_MINOR 0)
add_definitions(-std=c++11)
include_directories(include ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
file(GLOB SOURCES "src/*.cpp")
add_library(dbenchlib STATIC ${SOURCES})
add_executable(dbench bench_writer.cpp)
add_executable(dbench2 bench_writer2.cpp)
target_link_libraries(dbench
  dbenchlib
  ${Boost_LIBRARIES}
  pthread
)

install(TARGETS dbench DESTINATION /usr/local/bin)
if(RERUN)
message(WARNING "Please re-run cmake once again...")
endif(RERUN)
