cmake_minimum_required(VERSION 3.18)

set(TEST_APP logMtApp CACHE INTERNAL "${PROJECT_NAME}: Test application name" FORCE)

add_executable(${TEST_APP} "")

include(g3sinks)
add_dependencies(${TEST_APP} ${G3SINKS_INTERFACE})
target_link_libraries(${TEST_APP} PUBLIC ${G3SINKS_INTERFACE})

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
target_link_libraries(${TEST_APP} PUBLIC Threads::Threads)

# Propriétés de la librairie
target_include_directories(${TEST_APP} 
	PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
)

target_sources(${TEST_APP}
    PRIVATE
		main.cpp
		Arguments.cpp
		Arguments.h
)

set(COMPILE_FLAGS "-Wall" "-Wpedantic" "-Wshadow" "-Werror" "-Wextra")
target_compile_options(${TEST_APP} BEFORE PRIVATE ${COMPILE_FLAGS})

include(GNUInstallDirs)
set_target_properties(${TEST_APP}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}
)

