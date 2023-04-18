cmake_minimum_required (VERSION 3.8)
set (project "json-utils")
project(${project} CXX C)

# make cache variables for install destinations
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(cJSON)

set(SOURCES
   json_utils.h
   json_utils.c
)

#todo:
set (INC_DIRS
    ${PROJECT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/cJSON/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/cJSON/
    ${CMAKE_CURRENT_SOURCE_DIR}/debug-logger
    )

if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(LINK_DIRECTORIES
        ${LINK_DIRECTORIES}
        ${CMAKE_SOURCE_DIR}/arm-build/utils/cJSON
    )
else()
    set(LINK_DIRECTORIES
        ${LINK_DIRECTORIES}
        ${CMAKE_SOURCE_DIR}/x86-build/utils/cJSON
    )
endif()

set(LIBS
    cjson 
)
add_library (${project} SHARED ${SOURCES})
target_include_directories(${project} PUBLIC ${INC_DIRS})
target_link_directories(${project} PRIVATE ${LINK_DIRECTORIES})
target_link_libraries(${project} PUBLIC ${LIBS})

if(MINGW)
    add_compile_definitions(MINGW)
endif()

