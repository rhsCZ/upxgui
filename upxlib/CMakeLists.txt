cmake_minimum_required(VERSION 3.20)

project(upxlib LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(UPX_ROOT "${CMAKE_CURRENT_LIST_DIR}/../upx")

file(GLOB UPX_CORE_SOURCES CONFIGURE_DEPENDS
    "${UPX_ROOT}/src/*.cpp"
    "${UPX_ROOT}/src/[cfu]*/*.cpp"
)

list(REMOVE_ITEM UPX_CORE_SOURCES
    "${UPX_ROOT}/src/console/c_screen.cpp"
    "${UPX_ROOT}/src/console/s_djgpp2.cpp"
    "${UPX_ROOT}/src/console/s_object.cpp"
    "${UPX_ROOT}/src/console/s_vcsa.cpp"
    "${UPX_ROOT}/src/console/s_win32.cpp"
)

file(GLOB UPX_UCL_SOURCES CONFIGURE_DEPENDS "${UPX_ROOT}/vendor/ucl/src/*.c")
file(GLOB UPX_ZLIB_SOURCES CONFIGURE_DEPENDS "${UPX_ROOT}/vendor/zlib/*.c")

add_library(upxlib STATIC
    ${UPX_CORE_SOURCES}
    ${UPX_UCL_SOURCES}
    ${UPX_ZLIB_SOURCES}
)

target_include_directories(upxlib PRIVATE
    "${UPX_ROOT}/src"
    "${UPX_ROOT}/vendor"
    "${UPX_ROOT}/vendor/ucl"
    "${UPX_ROOT}/vendor/ucl/include"
    "${UPX_ROOT}/vendor/zlib"
)

target_compile_definitions(upxlib PRIVATE
    _CRT_NONSTDC_NO_DEPRECATE
    _CRT_NONSTDC_NO_WARNINGS
    _CRT_SECURE_NO_WARNINGS
    _SCL_SECURE_NO_DEPRECATE
    _SCL_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    WITH_GUI=1
    DOCTEST_CONFIG_DISABLE
    UPX_LZMA_COMPAT=1
    LZMA_RESULT_INPUT_OVERRUN=-4
    LZMA_RESULT_OUTPUT_OVERRUN=-5
    HAVE_VSNPRINTF=1
    $<$<CONFIG:Debug>:DEBUG=1>
)

if(MSVC)
    target_compile_options(upxlib PRIVATE
        /J
        /Zc:__cplusplus
        /EHsc
        /W3
    )
endif()

foreach(OUTPUT_CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(upxlib PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} "${CMAKE_BINARY_DIR}/lib"
    )
endforeach()
