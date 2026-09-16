// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <string_view>
#include <vector>

/**
 * @file HMI/Presentation/CreditsCatalog.h
 * @brief Les crédits du jeu, lus depuis `credits.json` (`LOT-87`, T3.3).
 */

namespace hmi {

/// Une ligne de crédits : un rôle, et les noms qui le tiennent (un par ligne à l'écran).
struct CreditLine {
    std::string role;
    std::vector<std::string> names;
};

/// Une section de crédits, dans la colonne de l'écran où elle se pose.
struct CreditSection {
    std::string id;  ///< Identifiant stable, qui choisit l'icône (`ui/icon/credits-section/<id>`).
    int column = 0;  ///< `0` à gauche, `1` à droite.
    std::string title;  ///< Titre dans la langue demandée.
    std::vector<CreditLine> lines;
};

/// Résultat de lecture : les sections, ou la raison de l'échec — jamais les deux.
struct CreditsResult {
    std::vector<CreditSection> sections;
    std::string error;  ///< Vide si la lecture a abouti.

    [[nodiscard]] bool ok() const {
        return error.empty();
    }
};

/**
 * @brief Lit les crédits dans une langue.
 *
 * **Pourquoi un fichier et non le formulaire.** Les attributions sont des données du projet — les
 * licences des assets et des bibliothèques exigent qu'on les cite — et la maquette en montre vingt
 * lignes quand le projet en a dix : les écrire dans le formulaire aurait mêlé des noms propres à la
 * mise en page, que la conception retouche. Le formulaire ne contient aucun nom.
 *
 * Titres et rôles portent une traduction par langue (`{"fr": …, "en": …}`). Un nom est d'ordinaire
 * une chaîne, qui ne se traduit pas ; il peut aussi être un libellé traduit, pour une mention qui
 * n'est pas un nom propre (« fan game non officiel »). Une langue absente d'un libellé retombe sur
 * le français, langue source des écrans : un crédit s'affiche toujours, même incomplètement
 * traduit.
 *
 * Logique **pure**, sans Qt ni disque (`EX-NFR-010`) : le modèle QML lit le fichier et passe le
 * texte. Une section malformée fait échouer toute la lecture, jamais des crédits partiels — une
 * attribution qui disparaîtrait sans erreur est précisément ce qu'une licence interdit.
 *
 * @param json Contenu de `credits.json`.
 * @param language Code de langue (`fr`, `en`).
 */
[[nodiscard]] CreditsResult readCredits(std::string_view json, std::string_view language);

}  // namespace hmi
