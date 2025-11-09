cmake_minimum_required(VERSION 3.10...3.24)

project(OpenConverter VERSION 1.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add position independent code flag
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add MSVC-specific compiler flags
if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

option(ENABLE_GUI "enable GUI" ON)
option(ENABLE_TESTS "enable unit tests" OFF)
# BMF is experimental feature, so we can't enable it by default
option(BMF_TRANSCODER "enable BMF Transcoder" OFF)
option(FFTOOL_TRANSCODER "enable FFmpeg Command Tool Transcoder" ON)
option(FFMPEG_TRANSCODER "enable FFmpeg Core Transcoder" ON)

# Common source files that don't depend on Qt
set(COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/main.cpp
    ${CMAKE_SOURCE_DIR}/common/src/encode_parameter.cpp
    ${CMAKE_SOURCE_DIR}/common/src/info.cpp
    ${CMAKE_SOURCE_DIR}/common/src/process_parameter.cpp
    ${CMAKE_SOURCE_DIR}/common/src/stream_context.cpp
    ${CMAKE_SOURCE_DIR}/engine/src/converter.cpp
)

# Common header files that don't depend on Qt
set(COMMON_HEADERS
    ${CMAKE_SOURCE_DIR}/common/include/encode_parameter.h
    ${CMAKE_SOURCE_DIR}/common/include/info.h
    ${CMAKE_SOURCE_DIR}/common/include/process_parameter.h
    ${CMAKE_SOURCE_DIR}/common/include/process_observer.h
    ${CMAKE_SOURCE_DIR}/common/include/stream_context.h
    ${CMAKE_SOURCE_DIR}/engine/include/converter.h
    ${CMAKE_SOURCE_DIR}/transcoder/include/transcoder.h
)

# Add transcoder sources based on options
if(FFMPEG_TRANSCODER)
    add_definitions(-DENABLE_FFMPEG)
    list(APPEND COMMON_SOURCES
        ${CMAKE_SOURCE_DIR}/transcoder/src/transcoder_ffmpeg.cpp
    )
    list(APPEND COMMON_HEADERS
        ${CMAKE_SOURCE_DIR}/transcoder/include/transcoder_ffmpeg.h
    )
endif()

if(FFTOOL_TRANSCODER)
    find_program(FFMPEG_EXECUTABLE NAMES ffmpeg)
    if(FFMPEG_EXECUTABLE)
        add_definitions(-DENABLE_FFTOOL)
        add_definitions(-DFFTOOL_PATH="${FFMPEG_EXECUTABLE}")
        list(APPEND COMMON_SOURCES
            ${CMAKE_SOURCE_DIR}/transcoder/src/transcoder_fftool.cpp
        )
        list(APPEND COMMON_HEADERS
            ${CMAKE_SOURCE_DIR}/transcoder/include/transcoder_fftool.h
        )
    else()
        message(FATAL_ERROR "FFmpeg not found in the system PATH!")
    endif()
endif()

if(BMF_TRANSCODER)
    if(DEFINED ENV{BMF_ROOT_PATH})
        set(BMF_ROOT_PATH $ENV{BMF_ROOT_PATH})
    else()
        set(BMF_ROOT_PATH "/Users/jacklau/Documents/Programs/Git/Github/bmf/output/bmf")
    endif()
    include_directories(${BMF_ROOT_PATH}/include)
    link_directories(${BMF_ROOT_PATH}/lib)
    add_definitions(-DENABLE_BMF)
    list(APPEND COMMON_SOURCES
        ${CMAKE_SOURCE_DIR}/transcoder/src/transcoder_bmf.cpp
    )
    list(APPEND COMMON_HEADERS
        ${CMAKE_SOURCE_DIR}/transcoder/include/transcoder_bmf.h
    )
endif()

# FFmpeg setup
set(FFMPEG_ROOT_PATH "" CACHE PATH "Path to your FFmpeg installation")

if(FFMPEG_ROOT_PATH)
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_ROOT_PATH}/include)
    set(FFMPEG_LIBRARY_DIRS ${FFMPEG_ROOT_PATH}/lib)
elseif(DEFINED ENV{FFMPEG_ROOT_PATH})
    set(FFMPEG_ROOT_PATH $ENV{FFMPEG_ROOT_PATH})
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_ROOT_PATH}/include)
    set(FFMPEG_LIBRARY_DIRS ${FFMPEG_ROOT_PATH}/lib)
elseif(WIN32)
    set(FFMPEG_ROOT_PATH "D:/ffmpeg/ffmpeg-n5.1.6-16-g6e63e49496-win64-gpl-shared-5.1")
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_ROOT_PATH}/include)
    set(FFMPEG_LIBRARY_DIRS ${FFMPEG_ROOT_PATH}/lib)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavfilter libavutil libswresample libswscale)
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS})
    set(FFMPEG_LIBRARY_DIRS ${FFMPEG_LIBRARY_DIRS})
endif()

# Function to detect FFmpeg version
function(find_ffmpeg_version FFMPEG_VERSION_INCLUDE_DIR FFMPEG_LIB_DIR VERSION_VAR)
    # First, attempt to find the version from ffversion.h
    find_file(FFMPEG_VERSION_FILE
            NAMES ffversion.h
            PATHS ${FFMPEG_VERSION_INCLUDE_DIR}
            PATH_SUFFIXES ffmpeg
            NO_DEFAULT_PATH)

    if(NOT FFMPEG_VERSION_FILE)
        message(WARNING "Unable to find FFmpeg version file in ${FFMPEG_VERSION_INCLUDE_DIR}. Attempting to get version from library files.")
    endif()

    if(FFMPEG_VERSION_FILE)
        # If we find ffversion.h, extract the version from it
        file(STRINGS ${FFMPEG_VERSION_FILE} FFMPEG_VERSION_DEFINE
            REGEX "#define FFMPEG_VERSION ")

        string(REGEX MATCH "FFMPEG_VERSION \"([^\"]+)\"" FFMPEG_VERSION_STRING ${FFMPEG_VERSION_DEFINE})
        set(FFMPEG_VERSION_MATCHED ${CMAKE_MATCH_1})

        if(FFMPEG_VERSION_MATCHED AND NOT FFMPEG_VERSION_MATCHED MATCHES "N-")
            string(REGEX MATCHALL "([0-9]+)" VERSION_PARTS ${FFMPEG_VERSION_MATCHED})
            list(GET VERSION_PARTS 0 VERSION_MAJOR)
            list(GET VERSION_PARTS 1 VERSION_MINOR)
            set(FFMPEG_VERSION_CLEAN "${VERSION_MAJOR}${VERSION_MINOR}")
            set(${VERSION_VAR} ${FFMPEG_VERSION_CLEAN} PARENT_SCOPE)
            message(STATUS "Detected FFmpeg version from ffversion.h: ${FFMPEG_VERSION_CLEAN}")
            return()
        else()
            message(WARNING "Unable to parse FFmpeg version from ${FFMPEG_VERSION_FILE}. Try the next step...")
        endif()
    endif()

    # If the version from ffversion.h is not found or is a dev version, fallback to libraries
    # Search for FFmpeg libraries to detect version
    file(GLOB FFMPEG_LIBRARIES "${FFMPEG_LIB_DIR}/libavcodec.*.*")

    if(FFMPEG_LIBRARIES)
        foreach(lib ${FFMPEG_LIBRARIES})
            string(REGEX MATCH "libavcodec\\.([0-9]+)" FFMPEG_VERSION_MATCH ${lib})
            if(FFMPEG_VERSION_MATCH)
                string(REGEX REPLACE "libavcodec\\.([0-9]+)" "\\1" FFMPEG_VERSION ${FFMPEG_VERSION_MATCH})

                # Map library version to FFmpeg version
                if(FFMPEG_VERSION STREQUAL "58")
                    set(FFMPEG_VERSION_CLEAN "40")  # FFmpeg v4.x
                elseif(FFMPEG_VERSION STREQUAL "59")
                    set(FFMPEG_VERSION_CLEAN "50")  # FFmpeg v5.x
                elseif(FFMPEG_VERSION STREQUAL "60")
                    set(FFMPEG_VERSION_CLEAN "60")  # FFmpeg v6.x
                elseif(FFMPEG_VERSION STREQUAL "61")
                    set(FFMPEG_VERSION_CLEAN "70")  # FFmpeg v7.x (dev)
                elseif(FFMPEG_VERSION STREQUAL "62")
                    set(FFMPEG_VERSION_CLEAN "71")
                else()
                    message(FATAL_ERROR "Unsupported FFmpeg library version: ${FFMPEG_VERSION}")
                endif()

                set(${VERSION_VAR} ${FFMPEG_VERSION_CLEAN} PARENT_SCOPE)
                message(STATUS "Detected FFmpeg version from libraries: ${FFMPEG_VERSION_CLEAN}")
                return()
            endif()
        endforeach()
    endif()

    message(FATAL_ERROR "Unable to determine FFmpeg version from both ffversion.h and libraries.")
endfunction()

# Detect FFmpeg version and set it as a definition
list(GET FFMPEG_LIBRARY_DIRS 0 FFMPEG_LIBRARY_DIRS_FIRST)
find_ffmpeg_version(${FFMPEG_INCLUDE_DIRS}/libavutil ${FFMPEG_LIBRARY_DIRS_FIRST} OC_FFMPEG_VERSION)
add_definitions(-DOC_FFMPEG_VERSION=${OC_FFMPEG_VERSION})

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/transcoder/include
    ${FFMPEG_INCLUDE_DIRS}
)

link_directories(
    ${FFMPEG_LIBRARY_DIRS}
)

# Create library target for core functionality
add_library(OpenConverterCore STATIC
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)

# Common library dependencies
target_link_libraries(OpenConverterCore PRIVATE
    avcodec
    avformat
    avfilter
    avutil
    swresample
    swscale
)

if(BMF_TRANSCODER)
    target_link_libraries(OpenConverterCore PRIVATE
        engine
        bmf_module_sdk
        hmp
    )
endif()

# Add include directories
target_include_directories(OpenConverterCore PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/transcoder/include
    ${FFMPEG_INCLUDE_DIRS}
)

# Handle GUI mode
if(ENABLE_GUI)
    add_definitions(-DENABLE_GUI)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

    # Add GUI-specific sources
    list(APPEND GUI_SOURCES
        ${CMAKE_SOURCE_DIR}/builder/src/base_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/file_selector_widget.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/converter_runner.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/placeholder_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/info_view_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/compress_picture_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/create_gif_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/extract_audio_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/cut_video_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/simple_video_player.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/remux_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/transcode_page.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/shared_data.cpp
        ${CMAKE_SOURCE_DIR}/builder/src/open_converter.cpp
    )

    # Add GUI-specific headers
    list(APPEND GUI_HEADERS
        ${CMAKE_SOURCE_DIR}/builder/include/base_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/file_selector_widget.h
        ${CMAKE_SOURCE_DIR}/builder/include/converter_runner.h
        ${CMAKE_SOURCE_DIR}/builder/include/placeholder_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/info_view_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/compress_picture_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/create_gif_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/extract_audio_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/cut_video_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/simple_video_player.h
        ${CMAKE_SOURCE_DIR}/builder/include/remux_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/transcode_page.h
        ${CMAKE_SOURCE_DIR}/builder/include/shared_data.h
        ${CMAKE_SOURCE_DIR}/builder/include/open_converter.h
    )

    # Add UI files
    list(APPEND UI_FILES
        ${CMAKE_SOURCE_DIR}/builder/src/open_converter.ui
    )

    # Add resource files
    list(APPEND RESOURCE_FILES
        ${CMAKE_SOURCE_DIR}/resources/lang.qrc
    )

    # Add GUI-specific include directories
    include_directories(
        ${CMAKE_SOURCE_DIR}/builder/include
        ${Qt${QT_VERSION_MAJOR}Widgets_INCLUDE_DIRS}
        ${Qt${QT_VERSION_MAJOR}Core_INCLUDE_DIRS}
        ${Qt${QT_VERSION_MAJOR}Gui_INCLUDE_DIRS}
    )

    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_executable(OpenConverter
            MANUAL_FINALIZATION
            ${GUI_SOURCES}
            ${GUI_HEADERS}
            ${UI_FILES} ${RESOURCE_FILES}
        )
    else()
        add_executable(OpenConverter
            ${GUI_SOURCES}
            ${GUI_HEADERS}
            ${UI_FILES} ${RESOURCE_FILES}
        )
    endif()

    target_link_libraries(OpenConverter PRIVATE
        OpenConverterCore
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
    )

    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_finalize_executable(OpenConverter)
    endif()

    # Set translation files
    set(TRANSLATIONS ${CMAKE_SOURCE_DIR}/resources/lang_chinese.ts)

    # Set application icon
    set(MACOSX_BUNDLE_ICON_FILE OpenConverter.icns)
    set(APP_ICON_MACOSX ${CMAKE_SOURCE_DIR}/resources/OpenConverter.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(OpenConverter PRIVATE ${APP_ICON_MACOSX})

    set_target_properties(OpenConverter PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.openconverter.app
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_ICON_FILE OpenConverter.icns
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    install(TARGETS OpenConverter
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    # Non-GUI mode
    add_executable(OpenConverter
        ${CMAKE_SOURCE_DIR}/main.cpp
    )
    target_link_libraries(OpenConverter PRIVATE OpenConverterCore)

    install(TARGETS OpenConverter
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Test dependencies
if(ENABLE_TESTS)
    # Use system GTest if available, otherwise use submodule
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        # add_subdirectory(${CMAKE_SOURCE_DIR}/../3rd_party/gtest ${CMAKE_BINARY_DIR}/gtest)
    endif()

    # Add test directory
    add_subdirectory(tests)
endif()
