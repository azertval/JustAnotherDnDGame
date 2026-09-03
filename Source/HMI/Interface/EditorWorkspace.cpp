// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/EditorWorkspace.h"

namespace hmi {

WorkspaceDressing dressingForWorkspace(EditorWorkspace workspace) noexcept {
    switch (workspace) {
        case EditorWorkspace::Level:
            return WorkspaceDressing{.levelToolBarVisible = true,
                                     .pixelToolBarVisible = false,
                                     .workshopMenuVisible = false};
        // Les deux espaces de PEINTURE partagent le meme habillage, et c'est leur definition : on
        // y peint une image, on n'y pose pas de tuiles. D'ou les outils de peinture plutot que
        // ceux du niveau, et le menu de l'atelier dont les commandes valent dans les deux. Les
        // ecrire deux fois laisserait croire qu'ils peuvent diverger, alors que rien ne le
        // justifierait -- et clang-tidy le signale a juste titre (bugprone-branch-clone).
        case EditorWorkspace::Planes:
        case EditorWorkspace::PixelArt:
            return WorkspaceDressing{.levelToolBarVisible = false,
                                     .pixelToolBarVisible = true,
                                     .workshopMenuVisible = true};
    }
    return WorkspaceDressing{};
}

EditorWorkspaceMask workspacesForPanel(PanelId panel) noexcept {
    switch (panel) {
        case PanelId::Palette:
        case PanelId::Levels:
        case PanelId::Links:
        case PanelId::Textures:
            return workspaceBit(EditorWorkspace::Level);
        case PanelId::Planes:
            // Le panneau des plans sert a les gerer PENDANT l'edition du niveau (ordre, densite,
            // parallaxe) autant qu'a en peindre un : il vit dans les deux espaces.
            return workspaceBit(EditorWorkspace::Level) | workspaceBit(EditorWorkspace::Planes);
        case PanelId::PixelCanvas:
        case PanelId::PixelHistory:
        case PanelId::PixelPalette:
            // Canevas, historique et palette servent aux DEUX espaces de peinture. Dupliquer les
            // docks donnerait deux canevas et deux historiques a tenir synchronises : le masque
            // dit simplement la verite.
            return workspaceBit(EditorWorkspace::PixelArt) | workspaceBit(EditorWorkspace::Planes);
    }
    // Inatteignable : le switch couvre l'enumeration, et un test verifie qu'il la couvre encore
    // apres ajout d'un panneau. Le repli choisit l'espace d'edition, celui ou l'auteur travaille.
    return workspaceBit(EditorWorkspace::Level);
}

EditorWorkspace workspaceForTool(EditorTool /*tool*/) noexcept {
    // Tous les outils de EditorTool sont des outils de NIVEAU : le groupe est disjoint de PixelTool
    // par construction (deux QActionGroup distincts, ActionCatalog.h). La fonction existe pour que
    // MainWindow suive une table plutot qu'une condition ecrite en dur -- exactement le defaut qui
    // avait laisse l'outil « Parcours » debranche au LOT-67.
    return EditorWorkspace::Level;
}

EditorWorkspace workspaceForPixelTool(PixelTool /*tool*/) noexcept {
    return EditorWorkspace::PixelArt;
}

}  // namespace hmi
