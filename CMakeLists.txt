cmake_minimum_required(VERSION 3.30)
project(FmodCmake VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available configurations: Debug, Release" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

add_executable(FmodCmake
        src/main.cpp
        src/audio/audio_engine.cpp
        src/audio/audio_engine.h
        src/core/game.h
        src/core/game.cpp
        src/audio/audio_config.h
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

function(CopyLibraryToTarget LIB_NAME TARGET_NAME)
    add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${LIB_NAME}>
            $<TARGET_FILE_DIR:${TARGET_NAME}>
    )
endfunction()

# Search path for additional libraries
add_subdirectory(libs)

# LIBRARIES

# raylib
target_link_libraries(${PROJECT_NAME} PUBLIC raylib)
CopyLibraryToTarget(raylib ${PROJECT_NAME})

# FMOD Studio and FMOD Core (You need both!)
target_link_libraries(${PROJECT_NAME} PUBLIC fmod_core)
target_link_libraries(${PROJECT_NAME} PUBLIC fmod_studio)
CopyLibraryToTarget(fmod_core ${PROJECT_NAME})
CopyLibraryToTarget(fmod_studio ${PROJECT_NAME})

# Add more libraries here:
#.............................


# ............................

# CONTENT AND CONFIG

function(CopyFolderToTarget DIR_NAME SOURCE_DIR)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND
            ${CMAKE_COMMAND} -E copy_directory
            ${SOURCE_DIR}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DIR_NAME}
    )
endfunction()

# Define the assets folder path and copy it to the build directory
set(ASSETS_FOLDER_NAME "assets")
set(ASSETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}")
CopyFolderToTarget(${ASSETS_FOLDER_NAME} ${ASSETS_SOURCE_DIR})

# Define the config folder path and copy it to the build directory
set(CONFIG_FOLDER_NAME "config")
set(CONFIG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_FOLDER_NAME}")
CopyFolderToTarget(${CONFIG_FOLDER_NAME} ${CONFIG_SOURCE_DIR})