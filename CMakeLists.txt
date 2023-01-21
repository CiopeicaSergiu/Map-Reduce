cmake_minimum_required(VERSION 3.13)  # CMake version check
project(TEMA_APD)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)            # Enable c++14 standard

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES main.cpp MapReduce.cpp)
#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fopenmp)

# Add executable target with source files listed in SOURCE_FILES variable

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

add_subdirectory(dependencies/boost)

find_package(MPI)
include_directories(TEMA_APD ${MPI_INCLUDE_PATH})

target_include_directories(${PROJECT_NAME} PUBLIC dependencies/boost/boost)
target_link_directories(${PROJECT_NAME} PRIVATE dependencies/boost/libs)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} 
                                      ${MPI_C_LIBRARIES}
)
