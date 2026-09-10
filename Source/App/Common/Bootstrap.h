// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string_view>

namespace core {
class MemoryLogSink;
}

/**
 * @file App/Common/Bootstrap.h
 * @brief Amorçage partagé par les **deux** applications (`LOT-86`).
 *
 * Depuis que le jeu et l'éditeur sont deux binaires distincts, la journalisation, le niveau de log
 * et les traductions de Qt lui-même sont les seules choses qu'ils font à l'identique. Les écrire
 * ici plutôt que deux fois évite qu'elles divergent — un défaut qui ne se verrait qu'en lisant les
 * deux fichiers côte à côte.
 *
 * Ce qui n'est **pas** ici : le style, le thème, la fenêtre. Ce sont précisément les points où les
 * deux applications diffèrent, et les mettre en commun demanderait un paramètre par différence.
 */

namespace app {

/**
 * @brief Installe les puits de journalisation et résout le niveau minimum.
 *
 * En développement : console et capture mémoire (exportable depuis les Options). Dans les deux
 * configurations : fichier horodaté sous `Logs/`, vidé à chaque message — c'est ce qui reste
 * lisible après un arrêt brutal, y compris en release où il n'y a ni console ni bouton d'export à
 * atteindre après coup.
 *
 * Le niveau vient de `JADG_LOG_LEVEL` puis, prioritairement, de `--log-level=<trace|info|warning|
 * error>`. Une valeur non reconnue est ignorée et signalée dans le journal, jamais fatale
 * (`EX-NFR-040`).
 *
 * @param argc            Nombre d'arguments de la ligne de commande.
 * @param argv            Arguments de la ligne de commande.
 * @param applicationName Nom porté par la bannière de démarrage (« JustAnotherDnDGame », «
 * LevelEditor »).
 * @return Le puits mémoire de la session en développement, `nullptr` en release.
 */
core::MemoryLogSink* installLogging(int argc, char** argv, std::string_view applicationName);

/**
 * @brief Installe le catalogue de traduction de **Qt lui-même** pour la langue de l'IHM.
 *
 * Les boutons standard des boîtes de dialogue — « Oui »/« Non »/« Annuler » — ne viennent pas du
 * catalogue du projet mais de celui de Qt, et restent en anglais tant qu'aucun `QTranslator` n'est
 * installé. Règle : ce catalogue doit suivre la **même** langue que l'IHM, sans quoi une boîte
 * entièrement rédigée en français porterait des boutons anglais — ce qui s'est vu à l'essai.
 *
 * Fichier absent (Qt déployé sans ses traductions) : repli silencieux sur l'anglais, jamais une
 * erreur bloquante (`EX-NFR-040`).
 *
 * @param language Code de langue de l'IHM (« fr », « en »).
 */
void installQtTranslations(std::string_view language);

/// @return La valeur d'un argument `--<nom>=…`, ou `std::nullopt` si l'argument est absent.
/// Un `optional` plutôt qu'une chaîne vide : `--export-atlas=` sans valeur est une erreur de
/// l'appelant, pas une absence d'option.
[[nodiscard]] std::optional<std::string_view> commandLineOption(int argc, char** argv,
                                                                std::string_view name);

}  // namespace app
