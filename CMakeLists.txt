cmake_minimum_required(VERSION 3.27)
project(DM-assist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(BASS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/bass/include)
if (WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt6/6.9.0/mingw_64/lib/cmake")
    set(BASS_DLL "${CMAKE_SOURCE_DIR}/lib/bass/win/bass.dll")
    set(BASS_LIBRARY lib/bass/win/bass.lib)
elseif (UNIX AND NOT APPLE)
    set(CMAKE_PREFIX_PATH "/usr/local/qt/Qt-6.6.1")
    set(BASS_LIBRARY ${CMAKE_SOURCE_DIR}/lib/bass/linux/libbass.so)
    set(BASS_DLL "${CMAKE_SOURCE_DIR}/lib/bass/linux/libbass.so")
endif ()


#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
set(FILES
        initiative-tracker/initiativemodel.h    initiative-tracker/initiativemodel.cpp
        mainwindow.h    mainwindow.cpp
        map-widget/abstractmaptool.h
        map-widget/brushtool.h  map-widget/brushtool.cpp
        map-widget/calibrationtool.h    map-widget/calibrationtool.cpp
        map-widget/fogtool.h    map-widget/fogtool.cpp
        map-widget/lighttool.h  map-widget/lighttool.cpp
        map-widget/mapview.h    map-widget/mapview.cpp
        map-widget/mapscene.h   map-widget/mapscene.cpp
        map-widget/maptabwidget.h   map-widget/maptabwidget.h
        map-widget/rulermaptool.h   map-widget/rulermaptool.cpp
        map-widget/sharedmapwindow.h    map-widget/sharedmapwindow.cpp
        map-widget/spellshapetool.h     map-widget/spellshapetool.cpp
        map-widget/undostack.h
        initiative-tracker/qinitiativetrackerwidget.h   initiative-tracker/qinitiativetrackerwidget.cpp
        initiative-tracker/hpprogressbardelegate.h  initiative-tracker/hpprogressbardelegate.cpp
        qplayerwidget.h qplayerwidget.cpp
        qsaveconfigdialog.h qsaveconfigdialog.cpp
        settingsdialog.h    settingsdialog.cpp
        settings.h
        thememanager.h  thememanager.cpp

)

set(UI_FILES
        mainwindow.ui
        qplayer.ui
        qplaylistedit.ui
        initiative-tracker/qinitiativetrackerwidget.ui
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
        initiative-tracker
        translations
        static
)

qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME}
#        WIN32
        main.cpp
        ${FILES}
        ${UI_FILES}
        ${RESOURCES_ADD}
)

target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
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
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/translations"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${BASS_DLL}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "Копирование bass в папку сборки..."
)


if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${QT_INSTALL_PATH}/bin/windeployqt.exe"
            "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Running windeployqt to copy Qt6 runtime dependencies..."
    )
endif ()


#foreach(qm ${QM_FILES})
#    get_filename_component(qm_filename "${qm}" NAME)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E rename "${qm}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/translations/${qm_filename}"
#            COMMAND ${CMAKE_COMMAND} -E echo "Moving ${qm_filename} into /translations"
#
#    )
#endforeach()