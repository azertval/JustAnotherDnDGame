# TACHE-01 — Fenêtre de tuiles centrée, encodage catégoriel par TileType {#lot-annexe-06-tache-01-fenetre-tuiles-encodage-categoriel}

**Lot :** [LOT-ANNEXE-06](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
Un réseau de neurones n'a aucune notion de `core::TileType` : il ne consomme que des nombres. Cette
tâche pose la première moitié de l'observation — une fenêtre carrée de tuiles centrée sur le
personnage, encodée en one-hot — sur laquelle TACHE-03 superposera l'état dynamique des mécanismes.

## Travail à réaliser
- **`aisolver::TileWindowEncoder`** (`Source/AiSolver/Env/TileWindowEncoder.h/.cpp`), namespace
  `aisolver` :
  - Constructeur `explicit TileWindowEncoder(int radius);` (`radius ≥ 0`, rayon en cases ; une
    fenêtre de rayon `r` couvre `(2r + 1) × (2r + 1)` cases).
  - `[[nodiscard]] aisolver::Tensor encode(const core::TileMap& tiles, core::GridPosition center)
    const;` — renvoie un tenseur de forme `(29, 2·radius + 1, 2·radius + 1)` (canaux =
    catégories de `core::TileType`, dans l'ordre de déclaration de l'énumération ; hauteur/largeur =
    fenêtre), une case `(dc, dr)` relative au centre valant `1.0f` sur le canal
    `static_cast<std::size_t>(tiles.tile(center.column + dc, center.row + dr))` et `0.0f` ailleurs.
  - Case hors limites (`!tiles.inBounds(...)`) : les 29 canaux valent `0.0f` (aucune catégorie
    active), voir décision de cadrage de l'épic.
  - `[[nodiscard]] int channelCount() const noexcept { return 29; }` et `[[nodiscard]] int
    windowSize() const noexcept { return 2 * _radius + 1; }` — dimensions dérivées, exposées pour
    que l'appelant (assemblage du tenseur complet, TACHE-03, et le réseau, LOT-ANNEXE-03) connaisse
    la forme sans la recalculer.
- **Case centrale** : `center` est la case de grille où se trouve le personnage, calculée par
  l'appelant à partir de sa boîte (`static_cast<int>(std::floor(playerBox.centerX()))`, cohérent avec
  l'origine haut-gauche de `TileMap`) — cette conversion n'est **pas** la responsabilité de
  `TileWindowEncoder` (donnée pure, testable sans `HeadlessLevelEnvironment`).

## Fichiers impactés
- `Source/AiSolver/Env/TileWindowEncoder.h` (nouveau).
- `Source/AiSolver/Env/TileWindowEncoder.cpp` (nouveau).
- Tests : `Source/Test/Unit/AiSolver/Env/test_tile_window_encoder.cpp` (nouveau).

## Tests (obligatoires)
- **Forme du tenseur** : `radius = 2` produit un tenseur `(29, 5, 5)`, `radius = 0` produit `(29, 1,
  1)` (uniquement la case du personnage).
- **One-hot correct** : une fenêtre construite sur une `TileMap` où chaque case a un type connu
  produit, pour chaque case, exactement un canal à `1.0f` et les 28 autres à `0.0f`, au bon indice de
  canal (`static_cast<std::size_t>(TileType::Solid)`, etc.).
- **Bord de carte** : un centre proche d'un coin (`column = 0, row = 0`) avec `radius = 3` produit
  toujours un tenseur `(29, 7, 7)` complet, les cases hors grille en vecteur nul, les cases dans la
  grille correctement encodées.
- **Déterminisme** : deux appels `encode` sur la même `TileMap`/`center` produisent des tenseurs
  bit-à-bit identiques.

## Points d'attention
- **L'ordre des canaux suit l'ordre de déclaration de `enum class TileType`** (`Empty` = 0, `Solid` =
  1, …, `DangerBlink` = 28) : un changement de cet ordre dans `Core/Levels/TileType.h` (peu probable,
  mais possible) romprait silencieusement tout modèle déjà entraîné sur l'ancien ordre — documenté
  ici comme risque connu, pas mitigé par ce lot (`Core` n'est pas modifié pour l'éviter).
  Si `TileType` gagne une valeur, `channelCount()` doit être mis à jour en même temps (pas de
  dérivation automatique depuis `Core`, qui n'expose pas de compte de catégories).
- **`radius = 0` reste un cas valide** (fenêtre réduite à la case du personnage), utile pour les
  tests et pour un premier modèle volontairement pauvre en génération 2 (recherche évolutionniste,
  LOT-ANNEXE-10).
- **Aucune allocation dépendante de la position du personnage** : la forme du tenseur ne dépend que
  de `radius`, jamais de `center` — condition nécessaire à une forme d'entrée stable pour un réseau
  de neurones à topologie fixe (LOT-ANNEXE-03).

## Définition de fait (DoD)
- `TileWindowEncoder` compile et est testé (`ctest` vert) sur les cas ci-dessus ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement (état et observation, conception d'une observation :
one-hot, normalisation, observation partielle), en particulier ses sections 3.2 (encodage one-hot
d'une grandeur catégorielle) et 3.1 (fenêtre centrée sur le personnage, invariance par translation).

## Exigences
`EX-IA-006` (nouvelle).
