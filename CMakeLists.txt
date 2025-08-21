cmake_minimum_required(VERSION 3.18)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

# add_compile_options(/Zc:__cplusplus /permissive- /W4)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


set(PROJECT_NAME UniDeskCppExt)
set(MODULE_VERSION_MAJOR 0)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 1)
set(MODULE_VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set(TARGET ${PROJECT_NAME}${MODULE_VERSION_MAJOR})

# Set this to allow indexing when we are using Qt Creator
set(QML_IMPORT_PATH ${CMAKE_BINARY_DIR}/temp CACHE STRING "" FORCE)

project(${PROJECT_NAME} VERSION ${MODULE_VERSION})

option(BUILD_PYTHON "Build Python bindings" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(QT_DEFAULT_MAJOR_VERSION 6)
set(REQUIRED_QT_VERSION 6.5.0)

find_package(ECM REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
message(STATUS "ECM version: ${ECM_VERSION}")



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



# Get the installation directory from qmake
if(ECM_VERSION VERSION_LESS 5.93.0)
    include(ECMQueryQmake)
    query_qmake(INSTALL_QMLDIR QT_INSTALL_QML)
else()
    include(ECMQueryQt)
    ecm_query_qt(INSTALL_QMLDIR QT_INSTALL_QML)
endif()

# Handle Python build status
if (BUILD_PYTHON)
    message(STATUS "Python bindings will be built")
    set(INSTALL_QMLDIR ${CMAKE_INSTALL_PREFIX}/qml)
    set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${TARGET}")
endif ()

if (INSTALL_QMLDIR)
    message(STATUS "QML directory:" "${INSTALL_QMLDIR}")
else ()
    message(FATAL_ERROR "QML directory cannot be detected.")
endif ()

# Install
if (_VCPKG_INSTALLED_DIR)
    set(CMAKECONFIG_INSTALL_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/${TARGET}")
    set(CMAKE_INSTALL_PREFIX "${_VCPKG_INSTALLED_DIR}")
else () 
    set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}")
endif ()

message(STATUS "CMake Config install directory:" "${CMAKECONFIG_INSTALL_DIR}")



add_subdirectory(src)

add_subdirectory(bindings)




# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.


