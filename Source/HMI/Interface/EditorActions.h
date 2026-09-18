// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QObject>
#include <array>

#include "HMI/Interface/ActionCatalog.h"

class QAction;
class QActionGroup;
class QToolBar;

/**
 * @file HMI/Interface/EditorActions.h
 * @brief Actions Qt de l'éditeur : outils et commandes principales (`LOT-56` TACHE-04).
 */

namespace hmi {

class EditorKeyBindings;
class Localization;
struct DesignTokens;

/**
 * @brief Construit et possède les `QAction` du catalogue (`HMI/Interface/ActionCatalog.h`) :
 *        chaque outil et chaque commande principale n'existe qu'une fois, placée simultanément
 *        dans la barre d'outils, un menu et un menu contextuel sans duplication.
 *
 * Les icônes sont dessinées par code (`hmi::themeIcon`) et recolorées depuis les jetons ;
 * `refreshIcons` les régénère lors d'un changement de thème (`LOT-56` TACHE-06).
 */
class EditorActions : public QObject {
    Q_OBJECT

public:
    /// @param tokens Jetons du châssis d'édition, pour la première génération des icônes.
    /// @param parent Parent Qt (propriétaire des `QAction` construits), optionnel.
    explicit EditorActions(const DesignTokens& tokens, QObject* parent = nullptr);

    /// @return L'action portant @p id.
    [[nodiscard]] QAction* action(IconId id) const;
    /// @return L'action de l'outil @p tool (raccourci vers `action(editorActionForTool(tool))`).
    [[nodiscard]] QAction* toolAction(EditorTool tool) const;
    /// @return Le groupe exclusif des six actions d'outil de niveau.
    [[nodiscard]] QActionGroup* toolGroup() const noexcept {
        return _toolGroup;
    }

    /// Ajoute les actions d'outils de niveau et les commandes principales à @p toolBar, dans
    /// l'ordre du catalogue, avec un séparateur entre les deux.
    void populateToolBar(QToolBar& toolBar) const;

    /// Applique la langue active : libellé de chaque action, et infobulle incluant son raccourci
    /// (jamais saisi séparément).
    void retranslateUi(const Localization& loc);

    /// Coche l'action de l'outil actif **sans** émettre `triggered` : resynchronisation quand le
    /// canevas change d'outil de lui-même (choisir une famille d'entité arme l'outil Entité).
    void setActiveTool(EditorTool tool) const;

    /// Active/désactive les six commandes qui n'ont de sens qu'en édition (Enregistrer, Essayer,
    /// Annuler, Refaire, Grille, Recadrer). Le mode de rendu reste toujours actif : il s'applique
    /// aussi en essai et en jeu réel (`EX-REN-046`).
    void setEditingCommandsEnabled(bool enabled) const;

    /// Reconstruit les icônes depuis un nouveau jeu de jetons (bascule de thème, `LOT-56`
    /// TACHE-06) : sans quoi elles resteraient aux anciennes couleurs après le changement.
    void refreshIcons(const DesignTokens& tokens);

    /**
     * @brief Fait refléter @p bindings sur le raccourci effectif de chaque commande concernée
     *        (`hmi::keyBindingIconCatalog`, `LOT-57` TACHE-04).
     *
     * `ActionCatalog` reste sans dépendance Qt (valeurs par défaut littérales) ; c'est ici, côté
     * Qt, que le raccourci **réellement actif** d'un `QAction` est synchronisé avec les touches
     * remappables — sans quoi un remappage dans les Options ne changerait rien à l'action.
     * Refait aussi les infobulles (`retranslateUi`), pour que « libellé + raccourci » reste vrai.
     * @param bindings Touches d'éditeur courantes.
     * @param loc      Catalogue de traduction courant, pour les infobulles.
     */
    void applyShortcuts(const EditorKeyBindings& bindings, const Localization& loc);

private:
    std::array<QAction*, EDITOR_ACTION_CATALOG_COUNT> _actions{};
    QActionGroup* _toolGroup;
};

}  // namespace hmi
