cmake_minimum_required(VERSION 2.8)

project("dicomdiritems")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal -pthread")

    set(IMEBRA_LIBRARIES pthread)

    if(CMAKE_BUILD_TYPE MATCHES DEBUG)
        message("debug mode")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} gcov)
    endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    message(STATUS "CLANG detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal -pthread")

    set(IMEBRA_LIBRARIES pthread)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    message(STATUS "MSVC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wp64")

endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")


# Specify include and source files
#---------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../library/include)

file(GLOB dicomdiritems_sources 
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

find_library(imebra_library NAMES imebra libimebra HINTS ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/../imebra ${CMAKE_CURRENT_SOURCE_DIR}/../../library-build )

# Add the source files to the project
#------------------------------------
add_executable(dicomdiritems ${imebra_sources} ${dicomdiritems_sources})

target_link_libraries(dicomdiritems ${imebra_library} pthread)

