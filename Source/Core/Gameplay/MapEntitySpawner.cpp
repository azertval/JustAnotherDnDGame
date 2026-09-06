// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/MapEntitySpawner.h"

#include <algorithm>

#include "Core/Ecs/Components/Interactable.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Gameplay/WorldFlags.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/MapEntity.h"

namespace core {

const std::vector<InteractableKind>& knownInteractableKinds() {
    // Construite une seule fois. Les cles d'invite sont des CLES, jamais du francais : le Core
    // n'ecrit pas de texte affichable (EX-NFR-011).
    static const std::vector<InteractableKind> familles = {
        // Le coffre ne se prend qu'une fois -- c'est le piege que ce lot doit tenir.
        {"chest", true, "interaction.chest"},
        // Le panneau se relit indefiniment : rien a consommer.
        {"sign", false, "interaction.sign"},
    };
    return familles;
}

std::size_t spawnMapEntities(World& world, const Level& level, std::string_view mapName,
                             const std::function<void(Entity, const MapEntity&)>& onEntity) {
    std::size_t creees = 0;
    for (const MapEntity& objet : level.entities()) {
        const Entity entite = world.createEntity();
        world.addComponent(entite, Transform{{static_cast<float>(objet.position.column),
                                              static_cast<float>(objet.position.row)}});

        const auto famille =
            std::ranges::find(knownInteractableKinds(), objet.type, &InteractableKind::type);
        if (famille != knownInteractableKinds().end()) {
            Interactable interactif;
            interactif.type = objet.type;
            interactif.position = objet.position;
            interactif.promptKey = std::string{famille->promptKey};
            if (famille->consumable) {
                // La cle est FABRIQUEE, jamais ecrite a la main : deux coffres d'une meme carte se
                // distinguent par leur case, et deux cartes ne se marchent pas dessus.
                interactif.consumedFlag =
                    keyForEntity(mapName, objet.type, objet.position.column, objet.position.row);
            }
            world.addComponent(entite, interactif);
        }
        // Un type INCONNU produit tout de meme une entite : c'est une erreur de conception toleree,
        // pas une carte invalide (EX-NFR-040). La refuser ferait disparaitre un objet de la carte
        // sans que son auteur comprenne pourquoi.

        if (onEntity) {
            onEntity(entite, objet);
        }
        ++creees;
    }
    return creees;
}

}  // namespace core
