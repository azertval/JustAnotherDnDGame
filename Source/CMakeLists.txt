# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Build order is intentionally explicit. Each layer creates only the targets that belong to it;
# later layers consume those targets and never modify an earlier target's source list.
option(BUILD_EDITOR_QT "Build the Qt editor, QML modules, and game application" ON)

add_subdirectory(Core)
add_subdirectory(HMI)

if(BUILD_EDITOR_QT)
    add_subdirectory(Ui/Jadg/Ui)
    add_subdirectory(HMI/Jadg/Runtime)
    add_subdirectory(App)
endif()

# Core -> reusable HMI/editor -> reusable design QML -> runtime QML types -> game application.
