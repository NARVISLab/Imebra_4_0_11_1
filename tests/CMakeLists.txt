cmake_minimum_required(VERSION 2.8)

project("ImebraTests")

find_library(gtest_library NAMES gtest libgtest HINTS ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/gtest)
find_library(imebra_library NAMES imebra libimebra HINTS ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/imebra ${CMAKE_BINARY_DIR}/../library-build )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal -pthread")

    set(IMEBRA_LIBRARIES ${imebra_library} ${gtest_library} pthread)

    if(CMAKE_BUILD_TYPE MATCHES DEBUG)
        message("debug mode")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} gcov)
    endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    message(STATUS "CLANG detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal -pthread")

    set(IMEBRA_LIBRARIES ${imebra_library} ${gtest_library} pthread)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    message(STATUS "MSVC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wp64")

    set(IMEBRA_LIBRARIES ${imebra_library} ${gtest_library})

endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

file(GLOB imebra_tests_include "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB imebra_tests_src "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

if(APPLE)
    set(IMEBRA_OBJC 1)
endif(APPLE)

set(imebra_tests_objc_src "")
if("${IMEBRA_OBJC}" STREQUAL "1")
    file(GLOB imebra_tests_objc_src "${CMAKE_CURRENT_SOURCE_DIR}/objectivec/*.mm")
endif("${IMEBRA_OBJC}" STREQUAL "1")



if("${gtest_include}" STREQUAL "")
    set(gtest_include ".")
endif("${gtest_include}" STREQUAL "")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../library/include ${gtest_include})
if("${IMEBRA_OBJC}" STREQUAL "1")
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework Foundation -framework CoreServices")
        if(IOS)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework UIKit")
        else(IOS)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework AppKit")
        endif(IOS)
    else(APPLE)
        include_directories("/usr/include/GNUstep")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstant-string-class=NSConstantString")
        set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} gnustep-base objc)
    endif(APPLE)
endif("${IMEBRA_OBJC}" STREQUAL "1")

add_executable(ImebraTests
    ${imebra_tests_include}
    ${imebra_tests_src}
    ${imebra_tests_objc_src}
)

target_link_libraries(ImebraTests ${IMEBRA_LIBRARIES})


