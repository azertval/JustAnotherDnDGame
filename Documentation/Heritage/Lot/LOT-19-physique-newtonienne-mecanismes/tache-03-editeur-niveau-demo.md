# TACHE-03 — Intégration éditeur et niveau de démonstration {#lot-19-tache-03-editeur-niveau-demo}

**Lot :** [LOT-19](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics`, `Source/Elements` · **Statut :** fait

## Contexte
`PressurePlate` existe côté `Core` (TACHE-02) mais reste invisible et impeignable depuis l'éditeur.
Cette tâche l'ajoute à la palette, généralise la liaison Maj+clic (jusqu'ici câblée en dur sur
`Switch`/`Door`) et livre un niveau de démonstration exploitant le mécanisme.

## Travail à réaliser
- **`hmi::TilePalette`** (`TilePalette.cpp`) : ajoute `core::TileType::PressurePlate` à
  `PALETTE_TYPES` (juste après `Switch`, avant `Door` — regroupe les deux types de déclencheurs) ;
  libellé court `"Plaque"` dans `labelFor`.
- **`hmi::regionForTile`** (`TileVisuals.cpp`) : nouvelle couleur distincte pour `PressurePlate` —
  `atlas.tile(1, 1)` (cyan, libéré depuis LOT-17 : l'ancien placeholder du personnage n'est plus
  utilisé par aucune tuile).
- **`hmi::EditorScreen::handleLinkClick`** (`EditorScreen.cpp`) : généralise la détection de
  « case liable » — nouvelle fonction locale `isTriggerTile(TileType)` (`Switch` **ou**
  `PressurePlate`) remplaçant les comparaisons directes à `TileType::Switch` ; la case « même
  catégorie que la case en attente » (redémarrage du geste de liaison) compare désormais
  `isTriggerTile(pending) == isTriggerTile(type)` plutôt qu'une égalité stricte de type (une
  plaque et un interrupteur sont tous deux des déclencheurs, pas la même catégorie qu'une porte).
- **Niveau de démonstration** (`Source/Elements/Levels/demo5.json`) : un niveau court illustrant la
  plaque de pression — par exemple une porte qui ne reste ouverte que le temps de la franchir en
  restant sur la plaque, ou un choix entre interrupteur (porte qui reste ouverte) et plaque (porte
  qui se referme) pour le même passage, sur le modèle de `demo4.json` (bordures solides, budget de
  mouvements si pertinent).
- **`Source/HMI/main.cpp`** : ajoute `demo5.json` à la séquence de niveaux du mode Jouer.

## Fichiers impactés
- `Source/HMI/Editor/TilePalette.cpp`.
- `Source/HMI/Graphics/TileVisuals.cpp`.
- `Source/HMI/Interface/EditorScreen.cpp`.
- `Source/Elements/Levels/demo5.json` (nouveau).
- `Source/HMI/main.cpp`.

## Tests (obligatoires)
- Aucun test unitaire nouveau (édition/rendu, déjà couverts par les tests existants de
  `EditorScreen`/`TilePalette` qui n'énumèrent pas les types un par un) — vérification par usage
  direct dans l'application.
- **Vérification visuelle et fonctionnelle obligatoire** dans l'application compilée : peindre une
  plaque de pression depuis la palette, la lier à une porte (Maj+clic), constater l'ouverture/
  fermeture continue en jeu (essai immédiat de l'éditeur ou séquence de jeu) ; charger `demo5.json`
  depuis le menu Jouer et le terminer.
- `ctest --preset ninja` reste vert (aucune régression sur les tests existants d'éditeur).

## Points d'attention
- **Le niveau de démonstration doit rester franchissable avec les réglages de physique par défaut**
  (masse 1,0) — la plaque doit réagir au personnage seul, sans configuration additionnelle
  (cohérent avec `MIN_TRIGGER_MASS` calé sur la masse par défaut, TACHE-02).
- La couleur choisie (`atlas.tile(1, 1)`, cyan) doit rester visuellement distincte du jaune de
  l'interrupteur et de l'orange de la porte, pour que la palette reste lisible (`EX-EDIT-015`).
- **Bug réellement rencontré** : `EditorLayout::TOOLBAR_TOP` codait en dur « 7 lignes » de palette
  pour positionner la barre d'outils sous elle — ajouter `PressurePlate` (8ᵉ entrée) faisait
  chevaucher les deux panneaux (« Porte » et « Pinceau » superposés à l'écran). Corrigé en
  remplaçant le littéral par `PALETTE_TYPE_COUNT` (nouvelle constante), avec un `static_assert`
  dans `TilePalette.cpp` qui échoue à la compilation si `PALETTE_TYPES` et ce compte divergent à
  nouveau à l'avenir. Repéré uniquement par vérification visuelle — aucun test ne couvrait la
  disposition du panneau.
- **Constat annexe (hors périmètre)** : la grille de l'éditeur affiche `Switch`/`Door`/
  `PressurePlate` avec une couleur assourdie par rapport à `TextureAtlas`/`GameScreen` (vérifié
  pixel par pixel) — confirmé **préexistant** (même défaut sur `demo4.json`, non modifié par ce
  lot) et sans impact gameplay (le rendu en jeu réel est correct). Non corrigé ici ; noté en
  mémoire projet pour une investigation future.

## Définition de fait (DoD)
- Plaque de pression peignable et liable depuis l'éditeur ; niveau de démonstration jouable et
  intégré à la séquence ; build `/W4 /WX` sans avertissement ; `ctest` vert ; vérifié visuellement.

## Exigences
Aucune exigence propre — intégration de `EX-GP-025` (TACHE-02) à l'éditeur et au contenu.
