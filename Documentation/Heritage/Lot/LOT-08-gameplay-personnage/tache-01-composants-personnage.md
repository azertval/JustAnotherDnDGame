# TACHE-01 — Composants du personnage & intention d'entrée {#lot-08-tache-01-composants-personnage}

**Lot :** [LOT-08](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** à faire

## Contexte
La physique (TACHE-03) opère sur des **données pures** (`EX-ARCH-011`). Avant de la coder, il
faut décrire le personnage comme un jeu de composants et définir la **frontière d'entrée** neutre
que `HMI` remplira (TACHE-05) et que `Core` consommera, sans que `Core` ne dépende de `HMI`.

## Travail à réaliser
- **`Collider`** (composant) : boîte englobante **AABB** en unités monde (`Vector2 size`, ancrée
  sur la position du `Transform`). Donnée pure.
- **`Player`** (composant marqueur) : identifie l'entité contrôlable ; porte l'état **« au sol »**
  (`bool grounded`) mis à jour par la physique (utile à la caméra et au futur saut).
- **`PlayerInput`** (struct pure, `Core/Ecs` ou `Core/Physics`) : **intention** de déplacement
  neutre — `float moveX` dans `[-1, 1]` (négatif = gauche, positif = droite). Aucune notion de
  touche ni de manette : c'est le contrat que `HMI` traduit (`EX-CTRL-010`). Un champ de saut
  n'est **pas** ajouté ici (hors périmètre), mais la structure est prévue pour évoluer.
- Réutilise `Transform` (position) et `Velocity` (vitesse) existants (LOT-03).

## Fichiers impactés
- `Source/Core/Ecs/Components/Collider.h`, `Player.h` (nouveaux).
- `Source/Core/Physics/PlayerInput.h` (nouveau) — ou `Core/Ecs/Components/` selon cohérence.
- `Source/Core/CMakeLists.txt` (si sources), `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Valeurs par défaut cohérentes (`Collider::size` nul, `Player::grounded == false`,
  `PlayerInput::moveX == 0`).
- Construction/agrégation des composants (structs agrégats), lecture des champs.

## Points d'attention
- **Données pures** : aucun type DirectX, aucune logique, aucun accès fichier ni `<Windows.h>`.
- `PlayerInput` doit rester **agnostique de l'entrée physique** : c'est ce qui rend la simulation
  testable et permettra manette/remappage plus tard sans toucher `Core`.
- Repère du projet : origine **haut-gauche**, `[colonne, ligne]`, `y` croissant vers le bas — la
  gravité pousse donc vers les `y` **positifs** (cohérent avec `EX-ARCH-020`).

## Définition de fait (DoD)
- Composants et `PlayerInput` définis, documentés (Doxygen en en-tête) et testés (`ctest` vert) ;
  build `/W4 /WX`.

## Exigences
`EX-ARCH-011`, `EX-CTRL-010`, `EX-NFR-010`.
