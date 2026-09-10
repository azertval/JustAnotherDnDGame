// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Presentation/RuleLabels.h"

#include <QSettings>
#include <QString>

#include "HMI/HmiLog.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {
namespace {

/**
 * @brief Le catalogue, charge UNE FOIS pour la langue demandee.
 *
 * Statique de fonction, et non membre d'une vue-modele : la fiche et l'inventaire demandent les
 * memes libelles, et deux catalogues liraient deux fois les memes fichiers pour rendre les memes
 * chaines. Rechargee seulement si la langue change -- ce qui n'arrive qu'au changement de langue,
 * pas a chaque lecture.
 */
[[nodiscard]] const Localization& catalog(const std::string& language) {
    static Localization loaded(executableDirectory() / "Localization");
    static std::string current;
    if (current != language) {
        // Le francais reste le repli : une cle absente d'un catalogue traduit doit rendre le terme
        // francais plutot que sa cle technique. C'est la regle de `Localization::text`, et c'est
        // pour cela que le defaut se charge en premier.
        if (!loaded.loadDefaultLanguage("fr")) {
            HMI_LOG_WARNING("Lexique des regles introuvable : les libelles resteront techniques.");
        }
        if (language != "fr" && !loaded.loadLanguage(language)) {
            HMI_LOG_INFO("Lexique des regles absent pour '" + language +
                         "' : repli sur le francais.");
        }
        current = language;
    }
    return loaded;
}

}  // namespace

std::string ruleLabel(std::string_view key, const std::string& language) {
    return catalog(language).text(key);
}

std::string activeLanguage() {
    return QSettings()
        .value(QStringLiteral("language"), QStringLiteral("fr"))
        .toString()
        .toStdString();
}

}  // namespace hmi
