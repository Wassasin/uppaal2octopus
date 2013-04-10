cmake_minimum_required(VERSION 2.8)

project(uppaal2octopus)

include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

file(GLOB_RECURSE uppaal2octopus_HEADERS src/*.hpp)
file(GLOB_RECURSE uppaal2octopus_SOURCES src/*.cpp)

add_executable(uppaal2octopus
	${uppaal2octopus_SOURCES}
)

add_definitions("-Wall -Wextra -Weffc++ -std=c++0x -pedantic -g3 -O3")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(Boost COMPONENTS system program_options filesystem regex REQUIRED)

include_directories(SYSTEM
                    ${Boost_INCLUDE_DIRS})
                    
target_link_libraries(uppaal2octopus
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_PROGRAM_OPTIONS_LIBRARY}
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_REGEX_LIBRARY})
