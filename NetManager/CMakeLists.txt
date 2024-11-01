cmake_minimum_required(VERSION 3.5)

project(NetManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_definitions(-DHAVE_REMOTE)

include_directories("${PROJECT_SOURCE_DIR}/../3rd/")
include_directories("${PROJECT_SOURCE_DIR}/../3rd/wincap/")
#include_directories("${PROJECT_SOURCE_DIR}/Protocol/arp")
#include_directories("${PROJECT_SOURCE_DIR}/Protocol/NetTransmit")
#include_directories("${PROJECT_SOURCE_DIR}/NetManager")
#include_directories("${PROJECT_SOURCE_DIR}/Analyse")
link_directories("${PROJECT_SOURCE_DIR}/../Libs/Windows/")

#file(GLOB_RECURSE SRC_DIR_LIST "./*.cpp" "./*.c" "./*.ui" "./*.h")
#aux_source_directory(NetManager NetManager_Src)
#aux_source_directory(Protocol/arp Protocol_Src)
#aux_source_directory(Protocol/NetTransmit Protocol_Src)
#aux_source_directory(Analyse Protocol_Src)
message(STATUS "SRC_DIR_LIST1 = ${Protocol_Src}")	
message(STATUS "SRC_DIR_LIST = ${NetManager_Src}")
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS  "${PROJECT_SOURCE_DIR}/*.h")
set(PROJECT_SOURCES
#   ${Protocol_Src}
#   ${NetManager_Src}
    ${SOURCES}
    ${HEADERS}
    mainwindow.ui
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NetManager
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET NetManager APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(NetManager SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(NetManager
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(NetManager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Packet wpcap)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.NetManager)
endif()
set_target_properties(NetManager PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS NetManager
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NetManager)
endif()
