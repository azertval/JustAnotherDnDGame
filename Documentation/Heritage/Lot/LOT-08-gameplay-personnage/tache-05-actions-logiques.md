# TACHE-05 — Actions logiques d'entrée (mapping touches → intention) {#lot-08-tache-05-actions-logiques}

**Lot :** [LOT-08](epic.md) · **Emplacement :** `Source/HMI/Input` · **Statut :** à faire

## Contexte
Le gameplay ne doit **jamais** dépendre d'une touche physique (`EX-CTRL-010`) : cette tâche pose
la couche qui traduit l'`InputState` (LOT-06) en une **intention** `core::PlayerInput` (TACHE-01),
échantillonnée une fois par frame en amont de la logique (`EX-CTRL-021`) et sans latence
supérieure à une frame (`EX-CTRL-020`). C'est une **logique pure** (aucun `<Windows.h>`),
testable en injectant un `InputState`.

## Travail à réaliser
- **Étendre `Key`** avec les touches lettres nécessaires (codes `VK_*`) : `A` (0x41), `D` (0x44),
  `Q` (0x51), `W` (0x57) — pour les schémas ZQSD/WASD ; les flèches existent déjà.
- **Mapping logique** (p. ex. `hmi::PlayerInputMapper` ou fonction `toPlayerInput(const InputState&)`):
  - **Aller à gauche** : `←` **ou** `Q`.
  - **Aller à droite** : `→` **ou** `D`.
  - Produit `PlayerInput::moveX` : `-1` (gauche), `+1` (droite), `0` si aucune ou les deux
    (opposition neutralisée).
- Le saut (`Espace`/`W`) figure dans la spec des contrôles mais **n'est pas mappé** ici (hors
  périmètre) — laisser la place sans le câbler.

## Fichiers impactés
- `Source/HMI/Input/InputState.h` (ajout d'énumérateurs `Key`).
- `Source/HMI/Input/PlayerInputMapper.h`/`.cpp` (nouveau) — ou en-tête seul si trivial.
- `Source/Test/CMakeLists.txt` (nouveau test unitaire).

## Tests (obligatoires)
- `←` seule → `moveX == -1` ; `→` seule → `moveX == +1`.
- `Q` → gauche ; `D` → droite (équivalence des touches alternatives).
- Aucune touche → `moveX == 0`.
- Gauche **et** droite simultanées → `moveX == 0` (opposition neutralisée, déterministe).

## Points d'attention
- **Dissociation stricte** touche ↔ action (`EX-CTRL-010`) : le reste du moteur ne connaît que
  `PlayerInput`. Un futur remappage/manette (`EX-CTRL-012`) ne touchera que cette couche.
- **Pur et testable** : le mapper ne lit pas la fenêtre, il lit un `InputState` injecté
  (`EX-NFR-010`).
- Un seul échantillonnage par frame, en amont de la mise à jour logique (`EX-CTRL-021`) : le
  mapper est appelé au début de l'`update` de l'écran, pas dans la boucle de rendu.

## Définition de fait (DoD)
- Mapping touches → `PlayerInput` documenté et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-CTRL-010`, `EX-CTRL-020`, `EX-CTRL-021`, `EX-NFR-010`.
