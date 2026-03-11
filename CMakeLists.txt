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


set(FILES
        src/modules/bestiary/bestiary-widget/fvttparser.h src/modules/bestiary/bestiary-widget/fvttparser.cpp
        src/modules/bestiary/bestiary-widget/dndbestiarypage.h src/modules/bestiary/bestiary-widget/dndbestiarypage.cpp
        src/modules/bestiary/bestiary-widget/dndbeaststructure.h
        src/modules/bestiary/bestiary-widget/rolltextbrowser.h src/modules/bestiary/bestiary-widget/rolltextbrowser.cpp
        src/modules/campaign/campaign-tree-widget/campaigntreewidget.cpp src/modules/campaign/campaign-tree-widget/campaigntreewidget.h
        src/modules/campaign/campaign-tree-widget/hoverwidget.cpp src/modules/campaign/campaign-tree-widget/hoverwidget.h
        src/modules/characters/charsheet-widget/abstractcharsheetwidget.h
        src/modules/characters/charsheet-widget/characterparser.h src/modules/characters/charsheet-widget/characterparser.cpp
        src/modules/characters/charsheet-widget/dndcharsheetwidget.h src/modules/characters/charsheet-widget/dndcharsheetwidget.cpp
        src/modules/characters/charsheet-widget/dndcharsheetdialogs.h src/modules/characters/charsheet-widget/dndcharsheetdialogs.cpp
        src/modules/characters/charsheet-widget/dndmodels.h src/modules/characters/charsheet-widget/dndmodels.cpp
        src/modules/characters/charsheet-widget/textclickablecheckbox.h src/modules/characters/charsheet-widget/textclickablecheckbox.cpp
        src/utils/error-handler/errorhandler.h src/utils/error-handler/errorhandler.cpp
        src/modules/encounter/initiative-tracker/initiativemodel.h src/modules/encounter/initiative-tracker/initiativemodel.cpp
        src/modules/encounter/initiative-tracker/initiativestructures.h
        src/modules/encounter/initiative-tracker/statuseditdialog.h src/modules/encounter/initiative-tracker/statuseditdialog.cpp
        src/modules/encounter/initiative-tracker/statusmanager.cpp src/modules/encounter/initiative-tracker/statusmanager.h
        src/core/mainwindow.h src/core/mainwindow.cpp
        src/modules/map/map-widget/abstractmaptool.h
        src/modules/map/map-widget/brushtool.h src/modules/map/map-widget/brushtool.cpp
        src/modules/map/map-widget/calibrationtool.h src/modules/map/map-widget/calibrationtool.cpp
        src/modules/map/map-widget/fixedsizetextitem.h src/modules/map/map-widget/fixedsizetextitem.cpp
        src/modules/map/map-widget/fogtool.h src/modules/map/map-widget/fogtool.cpp
        src/modules/map/map-widget/griditem.h src/modules/map/map-widget/griditem.cpp
        src/modules/map/map-widget/heightmaptool.h src/modules/map/map-widget/heightmaptool.cpp
        src/modules/map/map-widget/lighttool.h src/modules/map/map-widget/lighttool.cpp
        src/modules/map/map-widget/mapview.h src/modules/map/map-widget/mapview.cpp
        src/modules/map/map-widget/mapscene.h src/modules/map/map-widget/mapscene.cpp
        src/modules/map/map-widget/maptabwidget.h src/modules/map/map-widget/maptabwidget.cpp
        src/modules/map/map-widget/rulertool.h src/modules/map/map-widget/rulertool.cpp
        src/modules/map/map-widget/sharedmapwindow.h src/modules/map/map-widget/sharedmapwindow.cpp
        src/modules/map/map-widget/spellshapetool.h src/modules/map/map-widget/spellshapetool.cpp
        src/modules/map/map-widget/tokenitem.h src/modules/map/map-widget/tokenitem.cpp
        src/modules/map/map-widget/undostack.h
        src/modules/music/music-widget/musicwidget.h src/modules/music/music-widget/musicwidget.cpp
        src/dialogs/icon-picker/iconpickerdialog.h src/dialogs/icon-picker/iconpickerdialog.cpp
        src/modules/encounter/initiative-tracker/initiativetrackerwidget.h src/modules/encounter/initiative-tracker/initiativetrackerwidget.cpp
        src/modules/rolls/roll-widget/rollwidget.h src/modules/rolls/roll-widget/rollwidget.cpp
        src/modules/encounter/initiative-tracker/initiativedelegates.h src/modules/encounter/initiative-tracker/initiativedelegates.cpp
        src/core/dialogs/save-config-dialog/saveconfigdialog.h src/core/dialogs/save-config-dialog/saveconfigdialog.cpp
        src/core/dialogs/settings-dialog/settingsdialog.h src/core/dialogs/settings-dialog/settingsdialog.cpp
        src/core/settings.h
        src/core/update-manager/updatemanager.h src/core/update-manager/updatemanager.cpp
        src/core/dialogs/startup-dialog/recentcampaigndialog.cpp
        src/core/dialogs/startup-dialog/recentcampaigndialog.h
        src/modules/map/map-widget/texturepickerdialog.h
        src/modules/map/map-widget/effectgrapchicsitem.cpp
        src/modules/map/map-widget/effectgrapchicsitem.h
        src/modules/map/map-widget/areashapetool.cpp
        src/modules/map/map-widget/areashapetool.h
)

set(UI_FILES
        src/modules/characters/charsheet-widget/dndcharsheetwidget.ui
        src/modules/characters/charsheet-widget/attackdialog.ui
        src/modules/characters/charsheet-widget/resourcedialog.ui
        src/core/mainwindow.ui
        src/modules/music/music-widget/musicplayer.ui
        src/modules/music/music-widget/playlisteditdialog.ui
        src/modules/encounter/initiative-tracker/initiativetrackerwidget.ui
        src/modules/rolls/roll-widget/rollwidget.ui
        src/modules/encounter/initiative-tracker/statuseditdialog.ui
        src/core/dialogs/settings-dialog/settingsdialog.ui
        src/modules/bestiary/bestiary-widget/dndbestiarypage.ui
        src/modules/characters/charsheet-widget/dndCharacterData.h
)

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
        src/modules/bestiary/bestiary-widget
        src/utils/error-handler
        resources/translations
        resources/static
        theme-manager
        src/dialogs/icon-picker
        src/modules/characters/charsheet-widget
)

qt_add_translation(QM_FILES ${TS_FILES})
#add_custom_target(translations DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}
        WIN32
        src/core/main.cpp
        ${FILES}
        ${UI_FILES}
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
        "${TEMPLATE_DIR}"
        "${CMAKE_BINARY_DIR}/file-templates"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ICON_DIR}"
        "${CMAKE_BINARY_DIR}/iconset"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TEXTURES_DIR}"
        "${CMAKE_BINARY_DIR}/textures"
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