cmake_minimum_required(VERSION 3.22)
project(DM-assist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
        ${PROJECT_SOURCE_DIR}
)

#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/Release)
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.12.12/5.12.12/mingw73_64/lib/cmake")
#set(CMAKE_PREFIX_PATH "/usr/local/qt/Qt-5.12.12/5.12.12")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

set(HEADER_FILES
        creature.h
        dndcharacter.h
        dndcreature.h
        dndspellwidget.h
        encounter.h
        mainwindow.h
        dndmonster.h
        qdndcharacterwidget.h
        qdndinitiativeentityeditwidget.h
        qinitiativetrackerwidget.h
        qplayer.h
        qplayerinitiativeview.h
        qplaylistedit.h
)

set(SOURCE_FILES
        dndcharacter.cpp
        dndcampaign.cpp
        creature.cpp
        dndcharacter.cpp
        dndcreature.cpp
        dndspellwidget.cpp
        encounter.cpp
        mainwindow.cpp
        dndmonster.cpp
        qdndcharacterwidget.cpp
        qdndinitiativeentityeditwidget.cpp
        qinitiativetrackerwidget.cpp
        qplayer.cpp
        qplayerinitiativeview.cpp
        qplaylistedit.cpp
)

set(UI_FILES
        mainwindow.ui
        dndspellwidget.ui
        qdndcharacterwidget.ui
        qdndinitiativeentityeditwidget.ui
        qplayer.ui
        qplaylistedit.ui
        qinitiativetrackerwidget.ui
        qplayerinitiativeview.ui
)

set(TS_FILES
        translations/dm-assist_en_US.ts
        translations/dm-assist_ru_RU.ts
)


set(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static/app_resources.qrc)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        Xml
        #LinguistTools
        REQUIRED)

qt5_add_resources(RESOURCES_ADD ${RESOURCES})

include_directories(
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5PrintSupport_INCLUDE_DIRS}
        ${Qt5Sql_INCLUDE_DIRS}
        ${Qt5Multimedia_INCLUDE_DIRS}  #the directories
        translations
        static
)

#qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})

add_executable(${PROJECT_NAME} main.cpp
        ${HEADER_FILES}
        ${SOURCE_FILES}
        ${UI_FILES}
        ${RESOURCES_ADD}
)

add_dependencies(${PROJECT_NAME} translations)

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Multimedia
        Qt5::Xml
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
                "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()