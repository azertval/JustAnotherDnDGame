// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/WorldFlags.h"

namespace core {

bool WorldFlags::isSet(std::string_view key) const {
    return _flags.find(key) != _flags.end();
}

bool WorldFlags::set(std::string_view key) {
    // `insert().second` dit si le fait etait NEUF. C'est cette valeur qui repond a << le coffre
    // a-t-il deja ete ouvert ? >>, et la rendre evite au gameplay de faire un `isSet` puis un
    // `set` -- deux appels entre lesquels un autre pourrait se glisser.
    return _flags.emplace(key).second;
}

void WorldFlags::clear(std::string_view key) {
    const auto trouve = _flags.find(key);
    if (trouve != _flags.end()) {
        _flags.erase(trouve);
    }
}

std::vector<std::string> WorldFlags::all() const {
    return {_flags.begin(), _flags.end()};
}

std::string keyForEntity(std::string_view mapName, std::string_view entityType, int column,
                         int row) {
    std::string cle;
    cle.reserve(mapName.size() + entityType.size() + 16);
    cle.append(mapName).append("/").append(entityType).append("@");
    cle.append(std::to_string(column)).append(",").append(std::to_string(row));
    return cle;
}

}  // namespace core
