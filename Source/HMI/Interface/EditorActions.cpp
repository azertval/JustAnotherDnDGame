// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/EditorActions.h"

#include <QAction>
#include <QActionGroup>
#include <QKeyCombination>
#include <QKeySequence>
#include <QSignalBlocker>
#include <QString>
#include <QToolBar>

#include "HMI/Input/EditorKeyBindings.h"
#include "HMI/Input/QtKeyMap.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/ThemeIcons.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Le modificateur Ctrl de Save/Undo/Redo/Copy/Paste reste cable en dur (EditorKeyBindings.h) :
// seule la touche-lettre associee est remappable. Les autres actions n'ont pas de modificateur.
[[nodiscard]] bool carriesImplicitCtrl(EditorAction action) {
    switch (action) {
        case EditorAction::Save:
        case EditorAction::Undo:
        case EditorAction::Redo:
        case EditorAction::Copy:
        case EditorAction::Paste:
            return true;
        default:
            return false;
    }
}

}  // namespace

EditorActions::EditorActions(const DesignTokens& tokens, QObject* parent)
    : QObject(parent), _toolGroup(new QActionGroup(this)), _pixelToolGroup(new QActionGroup(this)) {
    _toolGroup->setExclusive(true);
    _pixelToolGroup->setExclusive(true);

    for (std::size_t i = 0; i < editorActionCatalog().size(); ++i) {
        const EditorActionSpec& spec = editorActionCatalog()[i];
        auto* const act = new QAction(this);
        act->setIcon(themeIcon(spec.id, tokens.size.iconMedium, tokens));
        act->setCheckable(spec.checkable);
        if (spec.shortcut[0] != '\0') {
            act->setShortcut(QKeySequence(QString::fromLatin1(spec.shortcut)));
        }
        if (spec.group == EditorActionGroup::LevelTools) {
            act->setActionGroup(_toolGroup);
        } else if (spec.group == EditorActionGroup::PixelTools) {
            act->setActionGroup(_pixelToolGroup);
        }
        _actions[i] = act;
    }
    // Pinceau actif par défaut (même défaut que l'ancien panneau Outils, EX-EDIT-014) ; pinceau
    // du canevas pixel art également, même raisonnement (LOT-54 TACHE-04).
    action(IconId::ToolPaint)->setChecked(true);
    action(IconId::PixelBrush)->setChecked(true);
}

QAction* EditorActions::action(IconId id) const {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog =
        editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        if (catalog[i].id == id) {
            return _actions[i];
        }
    }
    return nullptr;
}

QAction* EditorActions::toolAction(EditorTool tool) const {
    return action(editorActionForTool(tool));
}

QAction* EditorActions::pixelToolAction(PixelTool tool) const {
    return action(editorActionForPixelTool(tool));
}

void EditorActions::populateToolBar(QToolBar& toolBar) const {
    // Outils de niveau puis commandes, SANS les outils/commandes de canevas pixel art (groupes
    // distincts, barre d'outils dediee -- populatePixelToolBar, LOT-54 TACHE-04/TACHE-05).
    //
    // Le filtre sur `surface` (LOT-68, EX-IHM-074) est ce qui ramene la barre a l'essentiel : les
    // commandes ponctuelles (grille, recadrage, copier/coller, renommer, apercu des raccourcis)
    // restent atteignables par le menu et leur raccourci, sans occuper l'ecran en permanence.
    bool separatorInserted = false;
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        if (spec.group == EditorActionGroup::PixelTools ||
            spec.group == EditorActionGroup::PixelCommands ||
            spec.surface != ActionSurface::ToolBarAndMenu) {
            continue;
        }
        if (!separatorInserted && spec.group != EditorActionGroup::LevelTools) {
            toolBar.addSeparator();
            separatorInserted = true;
        }
        toolBar.addAction(action(spec.id));
    }
}

void EditorActions::populatePixelToolBar(QToolBar& toolBar) const {
    // Outils du canevas puis commandes de fichier (LOT-54 TACHE-05), separateur entre les deux --
    // meme disposition que populateToolBar (outils de niveau puis commandes).
    bool separatorInserted = false;
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        if ((spec.group != EditorActionGroup::PixelTools &&
             spec.group != EditorActionGroup::PixelCommands) ||
            spec.surface != ActionSurface::ToolBarAndMenu) {
            continue;
        }
        if (!separatorInserted && spec.group == EditorActionGroup::PixelCommands) {
            toolBar.addSeparator();
            separatorInserted = true;
        }
        toolBar.addAction(action(spec.id));
    }
}

void EditorActions::retranslateUi(const Localization& loc) {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog =
        editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        const QString label = QString::fromStdString(loc.text(catalog[i].labelKey));
        _actions[i]->setText(label);
        // Infobulle = libelle + raccourci de l'ACTION elle-meme (jamais une chaine traduite
        // decrivant la touche) : le remappage ne peut donc jamais la rendre fausse.
        const QKeySequence shortcut = _actions[i]->shortcut();
        _actions[i]->setToolTip(shortcut.isEmpty()
                                    ? label
                                    : label + QStringLiteral(" (") +
                                          shortcut.toString(QKeySequence::NativeText) +
                                          QStringLiteral(")"));
    }
}

void EditorActions::setActiveTool(EditorTool tool) const {
    QAction* const act = toolAction(tool);
    if (act == nullptr || act->isChecked()) {
        return;
    }
    const QSignalBlocker blocker(act);
    act->setChecked(true);
}

void EditorActions::setActivePixelTool(PixelTool tool) const {
    QAction* const act = pixelToolAction(tool);
    if (act == nullptr || act->isChecked()) {
        return;
    }
    const QSignalBlocker blocker(act);
    act->setChecked(true);
}

void EditorActions::setEditingCommandsEnabled(bool enabled) const {
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        // Le mode de rendu reste toujours actif : en edition, en essai et en jeu reel
        // (EX-REN-046) -- jamais desactive, contrairement aux autres commandes. Les commandes de
        // fichier de l'atelier (LOT-54 TACHE-05) suivent la meme regle que celles du niveau.
        const bool gated =
            (spec.group == EditorActionGroup::None && spec.id != IconId::ToggleRenderMode) ||
            spec.group == EditorActionGroup::PixelCommands;
        if (gated) {
            action(spec.id)->setEnabled(enabled);
        }
    }
}

void EditorActions::refreshIcons(const DesignTokens& tokens) {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog =
        editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        _actions[i]->setIcon(themeIcon(catalog[i].id, tokens.size.iconMedium, tokens));
    }
}

void EditorActions::applyShortcuts(const EditorKeyBindings& bindings, const Localization& loc) {
    for (const KeyBindingIconEntry& entry : keyBindingIconCatalog()) {
        const auto qtKey = static_cast<Qt::Key>(hmiKeyToQtKey(bindings.key(entry.action)));
        const Qt::KeyboardModifiers modifiers =
            carriesImplicitCtrl(entry.action) ? Qt::ControlModifier : Qt::NoModifier;
        action(entry.id)->setShortcut(QKeySequence(QKeyCombination(modifiers, qtKey)));
    }
    retranslateUi(loc);  // les infobulles incluent le raccourci : les refaire pour rester exactes.
}

}  // namespace hmi
