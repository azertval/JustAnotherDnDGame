# TACHE-06 — Écrans cibles (jeu démo + éditeur placeholder) {#lot-06-tache-06-ecrans-cibles}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
Les deux options non triviales du menu doivent mener quelque part de tangible, alors que le
chargement de niveaux et l'éditeur sont des lots futurs. « Charger niveau » ouvre la **scène
de démonstration** du LOT-05 (niveau provisoire) ; « Mode Edition » ouvre un écran **« à
venir »**. Depuis ces deux écrans, **Échap** revient au menu.

## Travail à réaliser
- `GameScreen` (implémente `IScreen`) : encapsule la scène de démo du LOT-05 (un
  `core::World` avec le `MovementSystem`, `SpriteRenderer`, `Camera2D`) — la construction de
  scène **quitte `main`** pour vivre ici. Met à jour la simulation à pas fixe et rend la
  scène ; **Échap** demande le retour au menu.
- `EditorScreen` (implémente `IScreen`) : affiche un texte « à venir » (via `BitmapFont`),
  **issu du catalogue de traduction** (clé `editeur.a_venir`, TACHE-03) et non en dur ;
  **Échap** revient au menu.
- Passage des dépendances de rendu (device, `SpriteBatch`, atlas, police) aux écrans qui en
  ont besoin.

## Fichiers impactés
- `Source/HMI/Interface/GameScreen.h`, `GameScreen.cpp` (nouveau).
- `Source/HMI/Interface/EditorScreen.h`, `EditorScreen.cpp` (nouveau).
- `Source/HMI/CMakeLists.txt`.

## Vérifications (obligatoires)
- « Charger niveau » ouvre la scène de démo animée (sprite mobile) ; Échap revient au menu.
- « Mode Edition » affiche l'écran « à venir » ; Échap revient au menu.
- Aucune fuite au changement d'écran (ressources d'écran libérées à la transition).

## Points d'attention
- Le `GameScreen` reprend la logique de démo **sans la dupliquer inutilement** ; la scène
  reste codée en dur (le chargement de fichier est un lot ultérieur).
- Rendu de la scène **découplé** de la simulation ; lecture seule de l'ECS (`EX-ARCH-012`).
- Écrans propriétaires de leurs ressources (RAII) ; le retour menu recrée le menu proprement.

## Définition de fait (DoD)
- Les deux écrans fonctionnent avec retour menu par Échap ; la scène codée en dur a quitté
  `main` ; build `/W4 /WX` sans avertissement, documenté.

## Exigences
`EX-REN-030`, `EX-REN-032`, `EX-ARCH-012`, `EX-ARCH-030`.
