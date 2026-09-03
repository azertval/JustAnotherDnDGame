# TACHE-01 — Modèle de tuile et fonction de hauteur {#lot-22-tache-01-modele-tuile-pente}

**Lot :** [LOT-22](epic.md) · **Emplacement :** `Core/Levels` · **Statut :** fait

## Contexte
Avant de toucher à la physique (TACHE-02), poser le vocabulaire : les deux nouvelles tuiles et la
fonction pure qui décrit la hauteur de leur surface — sans laquelle TACHE-02 n'a rien à consommer.

## Travail à réaliser
- **`Core/Levels/TileType.h`** : deux nouvelles valeurs, `SlopeUpRight` et `SlopeUpLeft`.
  `isSolid(TileType)` renvoie `false` pour les deux (voir décision de cadrage de l'épic : une pente
  n'est pas solide pour le balayage horizontal classique — sa solidité est gérée par la nouvelle
  passe de suivi, pas par `isSolid`).
- **Nouvelle fonction pure**, par exemple `core::slopeSurfaceHeight(TileType type, float localX)`
  (`Core/Levels/TileType.h` ou un nouvel en-tête `Core/Physics/SlopeGeometry.h` si `TileType` ne
  doit pas dépendre de notions de collision) :
  - `localX` dans `[0, 1[` = position horizontale **à l'intérieur de la case** (0 = bord gauche).
  - Renvoie la hauteur de la surface (fraction de case, `[0, 1]`) à cette position : pour
    `SlopeUpRight`, `1 - localX` (haut à droite) ; pour `SlopeUpLeft`, `localX` (haut à gauche).
  - `std::nullopt` (ou une valeur sentinelle documentée) pour tout autre type — pas une pente.
- **`Core/Levels/LevelLoader.cpp`** : `parseTileType` reconnaît `"slopeUpRight"`/`"slopeUpLeft"`.
- **`Core/Levels/LevelWriter.cpp`** : `tileTypeName` sérialise les deux nouvelles valeurs.

## Fichiers impactés
- `Source/Core/Levels/TileType.h` (+ nouveau fichier de géométrie si séparé).
- `Source/Core/Levels/LevelLoader.cpp`, `LevelWriter.cpp`.
- Tests : `Source/Test/Unit/Core/Levels/test_level_loader.cpp`, `test_level_writer.cpp` (chargement/
  round-trip), nouveau test unitaire pour la fonction de hauteur elle-même.

## Tests (obligatoires)
- `slopeSurfaceHeight` renvoie la hauteur attendue aux deux extrémités (`localX = 0` et proche de
  `1`) et au centre, pour les deux orientations.
- `isSolid(SlopeUpRight)`/`isSolid(SlopeUpLeft)` renvoient `false`.
- Chargement et round-trip JSON d'un niveau contenant les deux types de pente.

## Points d'attention
- **Ne pas rendre les pentes solides dans `isSolid`** — c'est tentant par symétrie avec `Solid`,
  mais casserait le suivi de pente dès TACHE-02 (voir décision de cadrage de l'épic : le bord haut
  d'une pente solide agirait comme un mur invisible).
- Documenter clairement la **convention de repère** utilisée par `slopeSurfaceHeight` (origine de
  la case, sens de `localX`, unité de la hauteur renvoyée) : TACHE-02 en dépend directement et une
  ambiguïté ici se répercuterait en bug de collision, pas en erreur de compilation.

## Définition de fait (DoD)
- Tuiles de pente chargeables/sérialisables ; fonction de hauteur testée en isolation ; aucune
  dépendance physique introduite à ce stade (`Core/Physics` non modifié).

## Exigences
`EX-GP-003` (déjà déclarée dans `gameplay.md`, modèle de données uniquement — l'implémentation
complète de l'exigence n'est acquise qu'à l'issue de TACHE-02).
