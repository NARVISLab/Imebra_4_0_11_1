# Options:
# -DIMEBRA_SHARED_STATIC=SHARED|STATIC (default = SHARED)
# -DJPEG2000=0|1|2 include JPEG2000 codec (1=version 1.X 2=version2.X)
# -DADDITIONAL_INCLUDE_PATH=path to additional include folders (optional)
# -DADDITIONAL_LIB_PATH=path to additional libraries (optional)
# -DIMEBRA_CHARSET_CONVERSION=ICONV|ICU|JAVA|WINDOWS (default = ICONV on posix, WINDOWS on Windows)
# -DIMEBRA_OBJC=1|0 (default = 0)
# -DIOS=PHONE|SIMULATOR (default not defined)
# -DCMAKE_OSX_SYSROOT

cmake_minimum_required(VERSION 2.8)

project("imebra")

set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(IMEBRA_LIBRARIES "")

# Select the proper charset conversion libraries according to the charset conversion method
#------------------------------------------------------------------------------------------
if(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)
    message("Using ICONV")
    add_definitions(-DIMEBRA_USE_ICONV)
    if(APPLE)
        set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} iconv)
    endif(APPLE)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES ICU)
    message("Using ICU")
    add_definitions(-DIMEBRA_USE_ICU)
    set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} icuuc)
    set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} icuio)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES JAVA)
    message("Using JAVA")
    add_definitions(-DIMEBRA_USE_JAVA)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES WINDOWS)
    message("Using WINDOWS CHARSET CONVERSION")
    add_definitions(-DIMEBRA_USE_WINDOWS_CHARSET)
else(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)
    if(WIN32)
        message("Using WINDOWS CHARSET CONVERSION (default)")
        add_definitions(-DIMEBRA_USE_WINDOWS_CHARSET)
    else(WIN32)
        message("Using ICONV (default)")
        add_definitions(-DIMEBRA_USE_ICONV)
        if(APPLE)
            set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} iconv)
        endif(APPLE)
    endif(WIN32)
endif(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)

# For IPhone build a static library, otherwise build a dynamic library
#---------------------------------------------------------------------
if(NOT DEFINED IMEBRA_SHARED_STATIC)
    set(IMEBRA_SHARED_STATIC "SHARED")
endif(NOT DEFINED IMEBRA_SHARED_STATIC)

if(APPLE)
    set(IMEBRA_OBJC 1)
    if(IOS)
        set(IMEBRA_SHARED_STATIC "STATIC")
        if("${IOS}" MATCHES "PHONE")
            exec_program(xcrun ARGS --sdk iphoneos --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
            set (CMAKE_OSX_ARCHITECTURES armv7 armv7s arm64)
        elseif("${IOS}" MATCHES "SIMULATOR")
            exec_program(xcrun ARGS --sdk iphonesimulator --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
            set (CMAKE_OSX_ARCHITECTURES i386 x86_64)
        else("${IOS}" MATCHES "PHONE")
            message (FATAL_ERROR "Set IOS to PHONE or SIMULATOR")
        endif("${IOS}" MATCHES "PHONE")
        message ("IOS SDK ROOT: ${CMAKE_OSX_SYSROOT}")
    endif(IOS)
endif(APPLE)

add_definitions(-DIMEBRA_DLL)
add_definitions(-DIMEBRA_DLL_EXPORTS)
add_definitions(-DNOMINMAX)

file(GLOB imebra_interface "${CMAKE_CURRENT_SOURCE_DIR}/include/imebra/*.h")
file(GLOB imebra_include "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB imebra_src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB imebra_implementation_src "${CMAKE_CURRENT_SOURCE_DIR}/implementation/*.cpp")
file(GLOB imebra_implementation_include "${CMAKE_CURRENT_SOURCE_DIR}/implementation/*.h")

if(NOT "${ADDITIONAL_LIB_PATH}" STREQUAL "")
    link_directories(${LINK_DIRECTORIES} ${ADDITIONAL_LIB_PATH})
endif(NOT "${ADDITIONAL_LIB_PATH}" STREQUAL "")

# Add OBJC sources
#-----------------
set(imebra_obj_src "")
set(imebra_obj_include "")
if("${IMEBRA_OBJC}" STREQUAL "1")
    file(GLOB imebra_objc_src "${CMAKE_CURRENT_SOURCE_DIR}/objectivec/*.mm")
    file(GLOB imebra_objc_include "${CMAKE_CURRENT_SOURCE_DIR}/include/imebra/objectivec/*.h")
endif("${IMEBRA_OBJC}" STREQUAL "1")

# Add jpeg2000 if specified
if("${JPEG2000}" STREQUAL "1")
    set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} openjpeg)
    add_definitions(-DJPEG2000)
    add_definitions(-DJPEG2000_V1)
    message(STATUS "Adding experimental Jpeg2000 codec (based on OpenJpeg v1.X)")
endif("${JPEG2000}" STREQUAL "1")

if("${JPEG2000}" STREQUAL "2")
    set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} openjp2)
    add_definitions(-DJPEG2000)
    add_definitions(-DJPEG2000_V2)
    message(STATUS "Adding experimental Jpeg2000 codec (based on OpenJpeg v2.X)")
endif("${JPEG2000}" STREQUAL "2")

add_library(imebra ${IMEBRA_SHARED_STATIC}
    ${imebra_implementation_src}
    ${imebra_implementation_include}
    ${imebra_include}
    ${imebra_objc_src}
    ${imebra_objc_include}
    ${imebra_src}
    ${imebra_interface}
)

if(NOT "${ADDITIONAL_INCLUDE_PATH}" STREQUAL "")
    target_include_directories(imebra PRIVATE ${ADDITIONAL_INCLUDE_PATH})
endif(NOT "${ADDITIONAL_INCLUDE_PATH}" STREQUAL "")

if("${IMEBRA_OBJC}" STREQUAL "1")
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework Foundation -framework CoreServices")
        if(IOS)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework UIKit")
        else(IOS)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework AppKit")
        endif(IOS)
    else(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstant-string-class=NSConstantString")
        target_include_directories(imebra PRIVATE "/usr/include/GNUstep")
        set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} gnustep-base objc)
    endif(APPLE)
endif("${IMEBRA_OBJC}" STREQUAL "1")

target_link_libraries(imebra ${IMEBRA_LIBRARIES})

target_include_directories(imebra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add flags specific to the compiler
#-----------------------------------
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal -fvisibility=hidden -fvisibility-inlines-hidden")
    set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} pthread)

    if(CMAKE_BUILD_TYPE MATCHES DEBUG)
        message("debug mode")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} gcov)
    endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    message(STATUS "CLANG detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal -fvisibility=hidden -fvisibility-inlines-hidden")
    set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} pthread)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    message(STATUS "MSVC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wp64")
    set(IMEBRA_LIBRARIES ${IMEBRA_LIBRARIES} Kernel32)

endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")




# Set the library version from IMEBRA_MAJOR_VERSION, IMEBRA_MINOR_VERSION,
#  & IMEBRA_REVISION
#---------------------------------------------------------------------------------
if ("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")
    set(IMEBRA_MAJOR_VERSION "1")
else("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")
    set(IMEBRA_MAJOR_VERSION "$ENV{IMEBRA_MAJOR_VERSION}")
endif("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")

if ("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")
    set(IMEBRA_MINOR_VERSION "0")
else("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")
    set(IMEBRA_MINOR_VERSION "$ENV{IMEBRA_MINOR_VERSION}")
endif("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")

if ("$ENV{IMEBRA_REVISION}" STREQUAL "")
    set(IMEBRA_REVISION "0")
else("$ENV{IMEBRA_REVISION}" STREQUAL "")
    set(IMEBRA_REVISION "$ENV{IMEBRA_REVISION}")
endif("$ENV{IMEBRA_REVISION}" STREQUAL "")

set(IMEBRA_VERSION "${IMEBRA_MAJOR_VERSION}.${IMEBRA_MINOR_VERSION}.${IMEBRA_REVISION}")

set_target_properties(imebra PROPERTIES VERSION ${IMEBRA_VERSION}
                                          SOVERSION ${IMEBRA_MAJOR_VERSION})

# Set packaging variables
#------------------------
set(CPACK_PACKAGE_NAME "libimebra${IMEBRA_MAJOR_VERSION}")
set(CPACK_PACKAGE_CONTACT "paolo@binarno.com")
set(CPACK_PACKAGE_VERSION ${IMEBRA_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Imebra library for DICOM medical files")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../readme.rst")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://imebra.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Paolo Brandoli <paolo@binarno.com>")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")

set(CPACK_RPM_PACKAGE_REQUIRES "libc6")

# Generate the control scripts
#-----------------------------
set(SHLIBS_FILE     "${CMAKE_CURRENT_BINARY_DIR}/shlibs")
set(POSTINST_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/postinst")
set(PRERM_SCRIPT    "${CMAKE_CURRENT_BINARY_DIR}/prerm")
set(POSTRM_SCRIPT   "${CMAKE_CURRENT_BINARY_DIR}/postrm")

# Generate postinst, prerm and postrm hooks
file(WRITE "${POSTINST_SCRIPT}" "#!/bin/sh\n\nset -e\n")
file(WRITE "${PRERM_SCRIPT}"    "#!/bin/sh\n\nset -e\n")
file(WRITE "${POSTRM_SCRIPT}"   "#!/bin/sh\n\nset -e\n")

# Generate shlibs
file(WRITE "${SHLIBS_FILE}" "libimebra ${IMEBRA_MAJOR_VERSION} ${CPACK_PACKAGE_NAME}\n")
file(APPEND "${POSTINST_SCRIPT}"
     "if [ \"$1\" = \"configure\" ]; then
        ldconfig
fi
")
file(APPEND "${POSTRM_SCRIPT}"
     "if [ \"$1\" = \"remove\" ]; then
        ldconfig
fi")

execute_process(COMMAND chmod 644 "${SHLIBS_FILE}")
execute_process(COMMAND chmod 755 "${POSTINST_SCRIPT}" "${PRERM_SCRIPT}" "${POSTRM_SCRIPT}")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${SHLIBS_FILE};${POSTINST_SCRIPT};${PRERM_SCRIPT};${POSTRM_SCRIPT}")

# Write license file
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/copyright"
     "Copyright (C) 2015 ${CPACK_DEBIAN_PACKAGE_MAINTAINER}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright"
        DESTINATION "share/doc/${CPACK_PACKAGE_NAME}/"
        PERMISSIONS
        OWNER_WRITE OWNER_READ
        GROUP_READ
        WORLD_READ)

install(TARGETS imebra LIBRARY DESTINATION lib
                                                ARCHIVE DESTINATION lib
                                                LIBRARY DESTINATION lib
                                                RUNTIME DESTINATION bin
                                                COMPONENT "Shared library")

install(FILES ${imebra_interface} DESTINATION include/imebra COMPONENT "Include files")

include(CPack)
