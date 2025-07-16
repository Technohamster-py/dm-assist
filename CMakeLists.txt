cmake_minimum_required(VERSION 3.27)
project(DM-assist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(BASS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/include)
set(TEMPLATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/file-templates)
set(ICON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/iconset)
if (WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt6/6.9.0/mingw_64/lib/cmake")
    set(BASS_DLL "${CMAKE_SOURCE_DIR}/lib/bass/win/bass.dll")
    set(BASS_LIBRARY ${CMAKE_SOURCE_DIR}/lib/bass/win/bass.lib)
elseif (UNIX AND NOT APPLE)
    set(CMAKE_PREFIX_PATH "/usr/local/qt/Qt-6.6.1")
    set(BASS_LIBRARY ${CMAKE_SOURCE_DIR}/lib/bass/linux/libbass.so)
    set(BASS_DLL "${CMAKE_SOURCE_DIR}/lib/bass/linux/libbass.so")
endif ()


set(FILES
        campaign-tree-widget/campaigntreewidget.cpp campaign-tree-widget/campaigntreewidget.h
        campaign-tree-widget/hoverwidget.cpp    campaign-tree-widget/hoverwidget.h
        charsheet-widget/abstractcharsheetwidget.h
        charsheet-widget/dndcharsheetwidget.h charsheet-widget/dndcharsheetwidget.cpp
        charsheet-widget/dndcharsheetdialogs.h charsheet-widget/dndcharsheetdialogs.cpp
        charsheet-widget/dndmodels.h charsheet-widget/dndmodels.cpp
        initiative-tracker/initiativemodel.h    initiative-tracker/initiativemodel.cpp
        initiative-tracker/initiativestructures.h
        initiative-tracker/statuseditdialog.h   initiative-tracker/statuseditdialog.cpp
        initiative-tracker/statusmanager.cpp    initiative-tracker/statusmanager.h
        mainwindow.h    mainwindow.cpp
        map-widget/abstractmaptool.h
        map-widget/brushtool.h  map-widget/brushtool.cpp
        map-widget/calibrationtool.h    map-widget/calibrationtool.cpp
        map-widget/fogtool.h    map-widget/fogtool.cpp
        map-widget/lighttool.h  map-widget/lighttool.cpp
        map-widget/mapview.h    map-widget/mapview.cpp
        map-widget/mapscene.h   map-widget/mapscene.cpp
        map-widget/maptabwidget.h   map-widget/maptabwidget.cpp
        map-widget/rulermaptool.h   map-widget/rulermaptool.cpp
        map-widget/sharedmapwindow.h    map-widget/sharedmapwindow.cpp
        map-widget/spellshapetool.h     map-widget/spellshapetool.cpp
        map-widget/undostack.h
        music-widget/musicwidget.h music-widget/musicwidget.cpp
        icon-picker/iconpickerdialog.h  icon-picker/iconpickerdialog.cpp
        initiative-tracker/initiativetrackerwidget.h initiative-tracker/initiativetrackerwidget.cpp
        initiative-tracker/initiativedelegates.h initiative-tracker/initiativedelegates.cpp
        saveconfigdialog.h saveconfigdialog.cpp
        settingsdialog.h    settingsdialog.cpp
        settings.h
        theme-manager/thememanager.h theme-manager/thememanager.cpp
        theme-manager/themediconmanager.h theme-manager/themediconmanager.cpp
)

set(UI_FILES
        charsheet-widget/dndcharsheetwidget.ui
        charsheet-widget/attackdialog.ui
        charsheet-widget/resourcedialog.ui
        mainwindow.ui
        music-widget/musicplayer.ui
        music-widget/playlisteditdialog.ui
        initiative-tracker/initiativetrackerwidget.ui
        initiative-tracker/statuseditdialog.ui
        settingsdialog.ui
)

set(TS_FILES
        translations/dm-assist_en_US.ts
        translations/dm-assist_ru_RU.ts
)


set(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static/app_resources.qrc)

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
        translations
        static
        theme-manager
        icon-picker
)

qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}
#        WIN32
        main.cpp
        ${FILES}
        ${UI_FILES}
        ${RESOURCES_ADD}
)

target_sources(${PROJECT_NAME} PRIVATE ${QM_FILES})
add_dependencies(${PROJECT_NAME} translations)

target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Svg
        Qt6::Xml
        ${BASS_LIBRARY}
        Qt6::Qml
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


    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/static/win_resources.rc")
    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Current config: $<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E echo "Cleaning up unnecessary files if Release..."

        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/CMakeFiles,ignore>"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/DM-assist_autogen,ignore>"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/Testing,ignore>"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/.cmake,ignore>"

        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/CMakeCache.txt,ignore>"
        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/Makefile,ignore>"
        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/cmake_install.cmake,ignore>"
        COMMAND ${CMAKE_COMMAND} -E remove -f "$<IF:$<CONFIG:Release>,${CMAKE_BINARY_DIR}/build.ninja,ignore>"

        COMMENT "Stripping build directory for Release (only if config is Release)..."
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${BASS_DLL}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "Копирование bass в папку сборки..."
)



add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TEMPLATE_DIR}"
        "${CMAKE_BINARY_DIR}/file-templates"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ICON_DIR}"
        "${CMAKE_BINARY_DIR}/iconset"
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