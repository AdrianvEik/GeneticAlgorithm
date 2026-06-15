cmake_minimum_required(VERSION 3.14)

project(GeneticAlgortihm VERSION 1.0 LANGUAGES C)

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<IF:$<CONFIG:Debug,RelWithDebInfo>,ProgramDatabase,Embedded>"
    )
endif()

include(CTest)
include(FetchContent)

option(BUILD_GA_EXE "Build the GA executable" ON)
option(BUILD_TESTING "Build tests" ON)

file(GLOB_RECURSE GA_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE GA_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(ga STATIC ${GA_SOURCES} ${GA_HEADERS})

target_include_directories(ga
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(ga PRIVATE c_std_11)

if(NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(ga PRIVATE Threads::Threads)
endif()

if(MSVC)
    target_compile_options(ga PRIVATE /W4)
else()
    target_compile_options(ga PRIVATE -Wall -Wextra -Werror)
endif()

if(BUILD_GA_EXE)
    add_executable(ga_main
        ${CMAKE_CURRENT_SOURCE_DIR}/app/ga_exe.c
    )

    target_link_libraries(ga_main PRIVATE ga)

    target_include_directories(ga_main PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_compile_features(ga_main PRIVATE c_std_11)

    if(MSVC)
        target_compile_options(ga_main PRIVATE /W4)
    else()
        target_compile_options(ga_main PRIVATE -Wall -Wextra -Werror)
    endif()
endif()

if(BUILD_TESTING)
    message(STATUS "Fetching cmocka as static library")

    FetchContent_Declare(
        cmocka
        GIT_REPOSITORY https://gitlab.com/cmocka/cmocka.git
        GIT_TAG cmocka-1.1.7
    )

    set(WITH_STATIC_LIB ON CACHE BOOL "Build cmocka static library" FORCE)
    set(WITH_SHARED_LIB OFF CACHE BOOL "Do not build cmocka shared library" FORCE)
    set(WITH_EXAMPLES OFF CACHE BOOL "Do not build cmocka examples" FORCE)
    set(UNIT_TESTING OFF CACHE BOOL "Do not build cmocka self tests" FORCE)
    set(WITH_CMOCKERY_SUPPORT OFF CACHE BOOL "" FORCE)
    set(PICKY_DEVELOPER OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(cmocka)

    if(TARGET cmocka::cmocka)
        add_library(project_cmocka INTERFACE)
        target_link_libraries(project_cmocka INTERFACE cmocka::cmocka)
    elseif(TARGET cmocka-static)
        add_library(project_cmocka INTERFACE)
        target_link_libraries(project_cmocka INTERFACE cmocka-static)
    elseif(TARGET cmocka)
        add_library(project_cmocka INTERFACE)
        target_link_libraries(project_cmocka INTERFACE cmocka)
    else()
        message(FATAL_ERROR "No usable cmocka target was created")
    endif()

    target_include_directories(project_cmocka INTERFACE
        ${cmocka_SOURCE_DIR}/include
    )

    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmocka)
    include(AddCMockaTest)

    add_subdirectory(Tests)

endif()