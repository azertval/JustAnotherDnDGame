// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/LevelNameValidation.h"

#include <string_view>

namespace hmi {

namespace {
constexpr std::string_view FORBIDDEN_CHARACTERS = R"(\/:*?"<>|)";
}  // namespace

bool isValidLevelName(const std::string& name) {
    const std::string trimmed = trimLevelName(name);
    if (trimmed.empty()) {
        return false;
    }
    return name.find_first_of(FORBIDDEN_CHARACTERS) == std::string::npos;
}

std::string trimLevelName(const std::string& name) {
    const std::size_t first = name.find_first_not_of(' ');
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = name.find_last_not_of(' ');
    return name.substr(first, last - first + 1);
}

}  // namespace hmi
