# TACHE-01 — Modèle de tuile et formule de courbe {#lot-23-tache-01-modele-tuile-arrondie}

**Lot :** [LOT-23](epic.md) · **Emplacement :** `Core/Levels` · **Statut :** fait

## Contexte
Étend l'abstraction de suivi de surface posée par `LOT-22` (fonction de hauteur par type de
tuile) à un profil courbe, sans toucher à la passe de résolution physique elle-même.

## Travail à réaliser
- **`Core/Levels/TileType.h`** : deux nouvelles valeurs, `RoundedUpRight` et `RoundedUpLeft`.
  `isSolid` renvoie `false` pour les deux, comme pour les pentes (même raisonnement : la solidité
  est gérée par la passe de suivi de surface, pas par le balayage classique).
- **Fonction de hauteur** (le point d'extension posé en `LOT-22-TACHE-01`, ex.
  `slopeSurfaceHeight` ou renommée en quelque chose comme `tileSurfaceHeight` si le nom d'origine
  devient trop restrictif une fois deux familles de courbes gérées) : ajoute les deux nouveaux cas,
  formule du quart de cercle (voir décision de cadrage de l'épic).
- **`Core/Levels/LevelLoader.cpp`/`LevelWriter.cpp`** : reconnaît/sérialise
  `"roundedUpRight"`/`"roundedUpLeft"`.

## Fichiers impactés
- `Source/Core/Levels/TileType.h` (ou fichier de géométrie partagé avec `LOT-22`).
- `Source/Core/Levels/LevelLoader.cpp`, `LevelWriter.cpp`.
- Tests : nouveaux cas dans le test unitaire de la fonction de hauteur (introduit en
  `LOT-22-TACHE-01`), `test_level_loader.cpp`, `test_level_writer.cpp`.

## Tests (obligatoires)
- La formule du quart de cercle renvoie les hauteurs attendues aux bords et au centre, pour les
  deux orientations (valeurs calculées à la main, comparées avec tolérance).
- `isSolid` renvoie `false` pour les deux nouveaux types.
- Chargement et round-trip JSON d'un niveau contenant les deux types arrondis.

## Points d'attention
- **Ne pas dupliquer la fonction de hauteur** : l'étendre (nouveau `case` dans le même `switch`/la
  même fonction que les pentes), pas créer une seconde fonction parallèle — sinon la passe de
  résolution de `LOT-22-TACHE-02` devrait connaître les deux, dupliquant la logique qu'elle est
  censée factoriser.

## Définition de fait (DoD)
- Tuiles arrondies chargeables/sérialisables ; formule de courbe testée en isolation ; aucune
  modification de la passe de résolution physique (réutilisée telle quelle).

## Exigences
`EX-GP-004` (déjà déclarée dans `gameplay.md`, modèle de données).
