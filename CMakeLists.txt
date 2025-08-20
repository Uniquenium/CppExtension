cmake_minimum_required(VERSION 3.25)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(UniDeskCppExt)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(QT_DEFAULT_MAJOR_VERSION 6)
set(REQUIRED_QT_VERSION 6.5.0)

find_package(ECM REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
message(STATUS "ECM version: ${ECM_VERSION}")

set(QT Core Widgets Quick QuickControls2 DBus Core5Compat)
find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED ${QT})
if (UNIX)
    if (NOT APPLE)
        list(APPEND QT GuiPrivate)
        find_package(KF6WindowSystem )
        # If not found, warn and continue
        if (NOT KF6WindowSystem_FOUND)
            message(WARNING "KF6WindowSystem not found. Some features may not work.")
        endif ()
    endif ()
endif ()

# Set Qt Policy
if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

# QHotkey
option(QHOTKEY_INSTALL OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/QHotkey)
set(QHOTKEY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/QHotkey/QHotkey)

include(GenerateExportHeader)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ECMGenerateHeaders)

# Get the installation directory from qmake
if(ECM_VERSION VERSION_LESS 5.93.0)
    include(ECMQueryQmake)
    query_qmake(INSTALL_QMLDIR QT_INSTALL_QML)
else()
    include(ECMQueryQt)
    ecm_query_qt(INSTALL_QMLDIR QT_INSTALL_QML)
endif()

add_subdirectory(pybind11)
pybind11_add_module(udcpptools src/udcpptools/udcpptools.cpp)


# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(udcpptools
                           PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})

