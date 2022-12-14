cmake_minimum_required(VERSION 3.18)
include_guard(GLOBAL)

include(ExternalProject)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(g3log)

set(G3SINKS_TARGET_NAME external_g3sinks)
set(G3SINKS_PATH ${CMAKE_CURRENT_LIST_DIR}/../extern/g3sinks)
set(G3SINKS_INSTALL_DIR ${CMAKE_BINARY_DIR}/g3sinks)
set(G3SINKS_LIB_DIR ${G3SINKS_INSTALL_DIR}/lib)
set(G3SINKS_INCLUDE_DIR ${G3SINKS_INSTALL_DIR}/include)

set(FIND_LIBRARY_USE_LIB64_PATHS)

ExternalProject_add(
    ${G3SINKS_TARGET_NAME}
    SOURCE_DIR ${G3SINKS_PATH}
    PREFIX ${G3SINKS_INSTALL_DIR}
    CMAKE_ARGS -DCHOICE_BUILD_DEBUG=OFF
               -DCHOICE_BUILD_TESTS=OFF
               -DCHOICE_SINK_SNIPPETS=OFF
               -DCHOICE_SINK_SYSLOG=ON
               -DCHOICE_SINK_LOGROTATE=ON
               -DCHOICE_BUILD_EXAMPLES=OFF
               -DCMAKE_INSTALL_LIBDIR=lib
               -DCMAKE_INSTALL_PREFIX=${G3SINKS_INSTALL_DIR}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_PREFIX_PATH=${G3LOG_INSTALL_DIR}
)

set_target_properties(
    ${G3SINKS_TARGET_NAME}
    PROPERTIES
    ADDITIONAL_CLEAN_FILES ${G3SINKS_INSTALL_DIR}
)

add_dependencies(${G3SINKS_TARGET_NAME} ${G3LOG_INTERFACE})

set(G3SINKS_INTERFACE G3SINKS CACHE INTERNAL "${G3SINKS_TARGET_NAME} : Nom de l'interface." FORCE)

add_library(${G3SINKS_INTERFACE} INTERFACE)
add_dependencies(${G3SINKS_INTERFACE} ${G3SINKS_TARGET_NAME})
target_include_directories(${G3SINKS_INTERFACE} INTERFACE ${G3SINKS_INCLUDE_DIR})
target_link_directories(${G3SINKS_INTERFACE} INTERFACE ${G3SINKS_LIB_DIR})
target_link_libraries(${G3SINKS_INTERFACE} INTERFACE ${G3LOG_INTERFACE} g3syslog g3logrotate)
