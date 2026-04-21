cmake_minimum_required(VERSION 3.27)
project(DM-assist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(BASS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bass/include)
set(TEMPLATE_DIR resources/file-templates)
set(ICON_DIR resources/iconset)
set(TEXTURES_DIR resources/textures)
if (WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt6/6.9.0/mingw_64/lib/cmake")
    set(BASS_DLL "${CMAKE_SOURCE_DIR}/3rdparty/bass/win/bass.dll")
    set(BASS_LIBRARY ${CMAKE_SOURCE_DIR}/lib/bass/win/bass.lib)
elseif (UNIX AND NOT APPLE)
    set(CMAKE_PREFIX_PATH "/usr/local/qt/Qt-6.6.1")
    set(BASS_LIBRARY ${CMAKE_SOURCE_DIR}/3rdparty/bass/linux/libbass.so)
    set(BASS_DLL "${CMAKE_SOURCE_DIR}/3rdparty/bass/linux/libbass.so")
endif ()

include(${PROJECT_SOURCE_DIR}/src/core/dialogs/dialogs.cmake)
include(${PROJECT_SOURCE_DIR}/src/core/update-manager/updater.cmake)
include(${PROJECT_SOURCE_DIR}/src/core/main-window/mainwindow.cmake)
include(${PROJECT_SOURCE_DIR}/src/modules/bestiary/bestiary.cmake)
include(${PROJECT_SOURCE_DIR}/src/modules/campaign/campaign.cmake)
include(${PROJECT_SOURCE_DIR}/src/modules/characters/characters.cmake)
include(${PROJECT_SOURCE_DIR}/src/modules/encounter/encounter.cmake)
include(${PROJECT_SOURCE_DIR}/src/modules/map/map.cmake)
include(${PROJECT_SOURCE_DIR}/src/modules/music/music.cmake)
include(${PROJECT_SOURCE_DIR}/src/modules/rolls/rolls.cmake)
include(${PROJECT_SOURCE_DIR}/src/utils/utils.cmake)

set(TS_FILES
        resources/translations/dm-assist_en_US.ts
        resources/translations/dm-assist_ru_RU.ts
)

set(RESOURCES resources/static/app_resources.qrc)

add_subdirectory(AdaptiveThemeLib)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        Svg
        Xml
        Qml
        LinguistTools
        REQUIRED)

qt6_add_resources(RESOURCES_ADD ${RESOURCES})

include_directories(
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Gui_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
        ${Qt6PrintSupport_INCLUDE_DIRS}
        ${Qt6Sql_INCLUDE_DIRS}
        ${Qt6Multimedia_INCLUDE_DIRS}
        ${BASS_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        resources/translations
        resources/static
        src/core/update-manager
        src/utils/widgets/roll-text-browser
        src/utils/widgets/text-clickable-checkbox
)

qt_add_translation(QM_FILES ${TS_FILES})
#add_custom_target(translations DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}
        WIN32
        src/core/main.cpp
        src/core/settings.h
        ${DIALOGS_SOURCES}
        ${UPDATE_MANAGER_SOURCES}
        ${MAINWINDOW_SOURCES}
        ${BESTIARY_SOURCES}
        ${CAMPAIGN_SOURCES}
        ${CHARACTERS_SOURCES}
        ${ENCOUNTER_SOURCES}
        ${MAP_SOURCES}
        ${MUSIC_SOURCES}
        ${ROLLS_SOURCES}
        ${UTILS_SOURCES}
        ${RESOURCES_ADD}
)

target_sources(${PROJECT_NAME} PRIVATE ${QM_FILES})
#add_dependencies(${PROJECT_NAME} translations)

target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Svg
        Qt6::Xml
        ${BASS_LIBRARY}
        Qt6::Qml
        AdaptiveThemeLib::AdaptiveThemeLib
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
            if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
            set(DEBUG_SUFFIX "d")
            endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)


    set(APP_ICON_RESOURCE_WINDOWS "resources/static/win_resources.rc")
    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
endif ()

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E echo "Current config: $<CONFIG>"
#        COMMAND ${CMAKE_COMMAND} -E echo "Cleaning up unnecessary files if Release..."
#
#        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/CMakeFiles,ignore>"
#        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/DM-assist_autogen,ignore>"
#        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/Testing,ignore>"
#        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/.cmake,ignore>"
#
#        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/CMakeCache.txt,ignore>"
#        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/Makefile,ignore>"
#        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/cmake_install.cmake,ignore>"
#        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/build.ninja,ignore>"
#
#        COMMENT "Stripping build directory for Release (only if config is Release)..."
#)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${BASS_DLL}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "Copying bass..."
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_DIR}"
        "${CMAKE_BINARY_DIR}/file-templates"
        COMMENT "Copying file templates..."
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/${ICON_DIR}"
        "${CMAKE_BINARY_DIR}/iconset"
        COMMENT "Copying icons..."
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/${TEXTURES_DIR}"
        "${CMAKE_BINARY_DIR}/textures"
        COMMENT "Copying textures..."
)


if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${QT_INSTALL_PATH}/bin/windeployqt.exe"
            "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Running windeployqt to copy Qt6 runtime dependencies..."
    )
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/translations"
)

foreach(qm ${QM_FILES})
    get_filename_component(qm_filename "${qm}" NAME)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename "${qm}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/translations/${qm_filename}"
            COMMAND ${CMAKE_COMMAND} -E echo "Moving ${qm_filename} into /translations"

    )
endforeach()

# Команда для обновления .ts файлов (переводов) с помощью lupdate
find_program(LUPDATE_EXECUTABLE lupdate HINTS "${QT_INSTALL_PATH}/bin")

if (LUPDATE_EXECUTABLE)
    add_custom_target(update_translations
            COMMAND ${LUPDATE_EXECUTABLE}
            ${FILES} ${UI_FILES}
            -ts ${TS_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Updating translation files with lupdate..."
    )
else()
    message(WARNING "lupdate not found! Translation update target will not work.")
endif()