add_library(raylib SHARED IMPORTED GLOBAL)

if (WIN32 AND MSVC)
    set_property(TARGET raylib PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/raylib.dll")
    set_property(TARGET raylib PROPERTY IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/raylib.lib")
elseif (APPLE)
    set_property(TARGET raylib PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/apple/libraylib.dylib")
endif()

target_include_directories(raylib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

###
## Link additional required platform SDKs and frameworks to raylib:
###

# Refer to the "A Note On Dependencies" section here:
# https://github.com/raysan5/raylib/wiki/Working-on-Windows
if (WIN32 AND MSVC)
    set(PLATFORM_EXT_LIBS "winmm" "gdi32")
else ()
    set(PLATFORM_EXT_LIBS)
endif ()

target_link_libraries(raylib INTERFACE ${PLATFORM_EXT_LIBS})