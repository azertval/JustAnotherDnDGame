# TACHE-01 — Données : réglages de *feel* + taille/placement du personnage {#lot-11-tache-01-donnees}

**Lot :** [LOT-11](epic.md) · **Emplacement :** `Source/Core` · **Statut :** à faire

## Contexte
On déclare d'abord les **réglages** de la gravité asymétrique et la **taille** du personnage, en
données pures (`EX-ARCH-011`), avant la logique et le rendu.

## Travail à réaliser
- **`PhysicsConfig`** (Core/Physics) — réglages ⚠️ à affiner :
  - `fallGravityMultiplier` (> 1) : gravité en **chute** = base × ce facteur ;
  - `apexThreshold` (unités/s) : seuil de vitesse verticale sous lequel on est « à l'apex » ;
  - `apexGravityMultiplier` (< 1) : gravité **réduite** près de l'apex (flottement) ;
  - `fastFallMultiplier` (> 1) : facteur **supplémentaire** en chute quand « bas » est maintenu.
- **`Core/Physics/PlayerSpawn.h`** (nouveau) : dimensions du personnage et placement centré,
  partagés par le jeu et les tests :
  - `kPlayerWidth = 0.4f`, `kPlayerHeight = 0.8f` ;
  - `playerSize()` → `Vector2{largeur, hauteur}` ;
  - `playerSpawnPosition(colonne, ligne)` → coin haut-gauche **centré** dans la tuile.

## Fichiers impactés
- `Source/Core/Physics/PhysicsConfig.h`, `Source/Core/Physics/PlayerSpawn.h` (nouveau).
- Tests unitaires (valeurs par défaut, placement centré).

## Tests (obligatoires)
- `PhysicsConfig` : `fallGravityMultiplier > 1`, `0 < apexGravityMultiplier < 1`,
  `apexThreshold > 0`, `fastFallMultiplier > 1`.
- `PlayerSpawn` : `playerSize()` = (0,4 ; 0,8) ; `playerSpawnPosition(c, r)` **centre** la boîte
  dans la tuile (marges symétriques : `x = c + (1-0,4)/2`, `y = r + (1-0,8)/2`).

## Points d'attention
- **Données pures** : aucune logique, aucun DirectX. Unités monde, `y` vers le bas.
- `Vector2` n'est pas `constexpr` : exposer des `constexpr float` + fonctions **inline** renvoyant
  des `Vector2` (pas de `constexpr Vector2`).
- Rétrocompatible : n'ajouter que des champs/constantes.

## Définition de fait (DoD)
- Réglages et constantes de spawn ajoutés, documentés et **testés** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-018`, `EX-ARCH-011`, `EX-NFR-010`.
