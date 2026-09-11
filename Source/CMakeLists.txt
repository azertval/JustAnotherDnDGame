# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

# Build order is intentionally explicit. Each layer creates only the targets that belong to it;
# later layers consume those targets and never modify an earlier target's source list.
add_subdirectory(Core)
add_subdirectory(HMI)
add_subdirectory(Ui/Jadg/Ui)
add_subdirectory(HMI/Jadg/Runtime)
add_subdirectory(App)

# Core -> reusable HMI/editor -> reusable design QML -> runtime QML types -> game application.
