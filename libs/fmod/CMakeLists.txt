# Refer to  https://www.fmod.com/docs/2.03/api/studio-api-getting-started.html
# and https://www.fmod.com/docs/2.03/api/platforms.html

###
## 1 - Define and add the FMOD core and studio libraries
###

project(fmod_core)
project(fmod_studio)

###
## 2 - Set the include paths to find the header files
###

set(FMOD_CORE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/core/inc")
set(FMOD_STUDIO_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/studio/inc")
add_library(fmod_core SHARED IMPORTED GLOBAL "${FMOD_CORE_INCLUDE_PATH}/fmod.hpp")
add_library(fmod_studio SHARED IMPORTED GLOBAL "${FMOD_STUDIO_INCLUDE_PATH}/fmod_studio.hpp")

###
## 3 - Set the Logging libraries for the Debug config
###

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FMOD_LIB_NAME "fmodL")
    set(FMOD_STUDIO_LIB_NAME "fmodstudioL")
else()
    set(FMOD_LIB_NAME "fmod")
    set(FMOD_STUDIO_LIB_NAME "fmodstudio")
endif()

###
## 4 - Set the location of the dynamic libraries
###

# Define the path of the dynamic libraries for each platform
# Add more target platforms here with an elseif ()
if (WIN32 AND MSVC)
    set_property(TARGET fmod_core PROPERTY IMPORTED_LOCATION
            "${CMAKE_CURRENT_SOURCE_DIR}/core/lib/x64/${FMOD_LIB_NAME}.dll")
    set_property(TARGET fmod_core PROPERTY IMPORTED_IMPLIB
            "${CMAKE_CURRENT_SOURCE_DIR}/core/lib/x64/${FMOD_LIB_NAME}_vc.lib")
    set_property(TARGET fmod_studio PROPERTY IMPORTED_LOCATION
            "${CMAKE_CURRENT_SOURCE_DIR}/studio/lib/x64/${FMOD_STUDIO_LIB_NAME}.dll")
    set_property(TARGET fmod_studio PROPERTY IMPORTED_IMPLIB
            "${CMAKE_CURRENT_SOURCE_DIR}/studio/lib/x64/${FMOD_STUDIO_LIB_NAME}_vc.lib")
    set(FMOD_PLUGINS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/plugins/x64")
    target_compile_definitions(fmod_studio INTERFACE "AUDIO_PLATFORM=\"Desktop\"")

elseif (APPLE)
    set_property(TARGET fmod_core PROPERTY IMPORTED_LOCATION
            "${CMAKE_CURRENT_SOURCE_DIR}/core/lib/apple/lib${FMOD_LIB_NAME}.dylib")
    set_property(TARGET fmod_studio PROPERTY IMPORTED_LOCATION
            "${CMAKE_CURRENT_SOURCE_DIR}/studio/lib/apple/lib${FMOD_STUDIO_LIB_NAME}.dylib")
    set(FMOD_PLUGINS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/plugins/apple")
    target_compile_definitions(fmod_studio INTERFACE "AUDIO_PLATFORM=\"Desktop\"")
endif ()

###
## 5 - Find and install all the FMOD additional dlls to the build folder (ex. google resonance or custom plugins)
###

file(GLOB FMOD_PLUGINS "${FMOD_PLUGINS_PATH}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")

if (DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(FMOD_PLUGINS_DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/fmod")
else ()
    set(FMOD_PLUGINS_DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/plugins/fmod")
endif ()

file(INSTALL ${FMOD_PLUGINS} DESTINATION ${FMOD_PLUGINS_DESTINATION}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")

###
## 6 - Prepare FMOD for linking with the main application
###

set_property(TARGET fmod_studio PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${FMOD_STUDIO_INCLUDE_PATH} ${FMOD_CORE_INCLUDE_PATH})