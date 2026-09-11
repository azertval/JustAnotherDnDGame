# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Build order is intentionally explicit. Each layer creates only the targets that belong to it;
# later layers consume those targets and never modify an earlier target's source list.
option(BUILD_EDITOR_QT "Build the Qt editor, QML modules, and game application" ON)

if(BUILD_EDITOR_QT)
    set(QT_VERSION_MINIMUM "6.8.7")
    set(_qt_hints "")
    foreach(_qt_var QT_ROOT_DIR Qt6_ROOT QTDIR)
        if(DEFINED ENV{${_qt_var}})
            list(APPEND _qt_hints "$ENV{${_qt_var}}")
        endif()
    endforeach()

    if(WIN32)
        file(GLOB _qt_installs "$ENV{SystemDrive}/Qt/*/msvc*_64" "C:/Qt/*/msvc*_64" "D:/Qt/*/msvc*_64")
    else()
        file(GLOB _qt_installs "$ENV{HOME}/Qt/*/gcc_64" "/opt/Qt/*/gcc_64")
    endif()
    list(SORT _qt_installs COMPARE NATURAL ORDER DESCENDING)
    list(APPEND _qt_hints ${_qt_installs})
    if(_qt_hints)
        list(REMOVE_DUPLICATES _qt_hints)
        list(APPEND CMAKE_PREFIX_PATH ${_qt_hints})
    endif()

    find_package(Qt6 ${QT_VERSION_MINIMUM} REQUIRED COMPONENTS
        Widgets Gui GuiPrivate Multimedia ShaderTools Quick Qml QuickControls2)

    # Qt QML policies are directory-scoped. This must be initialized here, before sibling
    # subdirectories such as Ui/Jadg/Ui and HMI/Jadg/Runtime call qt_add_qml_module().
    # REQUIRES 6.8 enables QTP0001 through QTP0005, including QTP0004 and QTP0005.
    qt_standard_project_setup(REQUIRES 6.8)
endif()

add_subdirectory(Core)
add_subdirectory(HMI)

if(BUILD_EDITOR_QT)
    add_subdirectory(Ui/Jadg/Ui)
    add_subdirectory(HMI/Jadg/Runtime)
    add_subdirectory(App)
endif()

# Core -> reusable HMI/editor -> reusable design QML -> runtime QML types -> game application.
