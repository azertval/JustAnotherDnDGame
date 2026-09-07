// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Audio/SoundTriggers.h"

namespace hmi {

std::optional<std::string> soundForEvent(GameEvent event) {
    // switch exhaustif sans default : un GameEvent ajoute sans entree casse la compilation.
    switch (event) {
        case GameEvent::Jumped:
            return std::string{"saut"};
        case GameEvent::Landed:
            return std::string{"atterrissage"};
        case GameEvent::Dashed:
            return std::string{"dash"};
        case GameEvent::WallContactEnter:
            return std::nullopt;  // aucun bruitage dedie dans ce lot.
        case GameEvent::SwitchToggled:
            return std::string{"interrupteur"};
        // Porte et plaque : les deux fronts partagent le meme son (clang-tidy bugprone-branch-clone
        // exige un regroupement explicite plutot que deux `return` identiques cote a cote).
        case GameEvent::DoorOpened:
        case GameEvent::DoorClosed:
            return std::string{"porte"};
        case GameEvent::PressurePlatePressed:
        case GameEvent::PressurePlateReleased:
            return std::string{"plaque_pression"};
        case GameEvent::BlockPushed:
            return std::nullopt;  // aucun bruitage dedie dans ce lot.
        case GameEvent::Died:
            return std::string{"mort"};
        case GameEvent::ExitReached:
            return std::string{"sortie_atteinte"};
        // Navigation et retour partagent le meme son (meme raison que porte/plaque ci-dessus).
        case GameEvent::MenuNavigate:
        case GameEvent::MenuBack:
            return std::string{"menu_deplacement"};
        case GameEvent::MenuConfirm:
            return std::string{"menu_validation"};
        case GameEvent::PauseOpened:
            return std::nullopt;  // pas dans le perimetre sonore de l'epic (Inclus).
    }
    return std::nullopt;  // inatteignable : le switch couvre tout l'enum.
}

}  // namespace hmi
