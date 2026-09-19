// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/DataRoot.h"

#include <cstddef>
#include <system_error>
#include <utility>

namespace hmi {

namespace {

[[nodiscard]] std::filesystem::path& currentRoot() {
    static std::filesystem::path root;
    return root;
}

}  // namespace

std::filesystem::path resolveDataRoot(const std::vector<std::string>& arguments,
                                      const std::filesystem::path& executableDirectory,
                                      const std::filesystem::path& sourceData) {
    for (std::size_t index = 0; index + 1 < arguments.size(); ++index) {
        if (arguments[index] == "--data") {
            return arguments[index + 1];
        }
    }
    std::error_code error;
    if (!sourceData.empty() && std::filesystem::is_directory(sourceData / "Levels", error)) {
        return sourceData;
    }
    return executableDirectory;
}

void setEditorDataRoot(std::filesystem::path root) {
    currentRoot() = std::move(root);
}

const std::filesystem::path& editorDataRoot() {
    return currentRoot();
}

}  // namespace hmi
