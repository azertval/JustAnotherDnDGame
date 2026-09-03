# TACHE-01 — `skins.json` : format versionné et jeux de skins {#lot-42-tache-01-skins-json}

**Lot :** [LOT-42](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
L'association type de tuile → apparence est aujourd'hui **codée en dur** dans
`hmi::regionForTile` (`TileVisuals.cpp`) : une table de trente entrées pointant vers des cases de
l'atlas procédural. C'est la bonne solution pour des couleurs plates ; c'en est une mauvaise pour
des textures, que le level designer doit pouvoir changer sans recompiler.

Deux exigences structurent le format dès sa création, parce qu'aucune des deux ne peut être ajoutée
après coup sans migration : le regroupement en **jeux nommés** (`EX-EDIT-024`) et le **mode de
rendu** par type (`EX-EDIT-025`).

## Travail à réaliser
- **Format `skins.json`** : un numéro de `version`, puis des jeux de skins nommés, chacun associant
  un `core::TileType` à une entrée `{ asset, mode }` où `mode` vaut `single` ou `bitmask16`.
  Un jeu par défaut est désigné. Exemple de structure :
  ```json
  {
    "version": 1,
    "defaut": "foret",
    "jeux": {
      "foret": {
        "solid":        { "asset": "stone.png", "mode": "bitmask16" },
        "block":        { "asset": "crate.png", "mode": "single" },
        "slopeUpRight": { "asset": "stone_flat.png", "mode": "single" }
      }
    }
  }
  ```
- **Lecture et écriture** (nlohmann/json, déjà dépendance) avec la même politique d'erreurs que
  `core::LevelLoader` : **jamais d'exception**, un résultat porteur d'un code d'erreur exploitable.
  Fichier absent → configuration vide, pas une erreur (état de départ légitime).
- **Résolution** : `(jeu, TileType) → { asset, mode }`, avec repli sur le jeu par défaut si le jeu
  demandé n'existe pas, et absence d'entrée si le type n'est pas skinné (l'appelant affichera le
  damier).
- **Emplacement d'écriture** : `executableDirectory()/Assets/skins.json`, exactement comme
  l'enregistrement d'un niveau (`Source/HMI/Editor/LevelFileOperations.cpp`). Aucun nouveau
  mécanisme d'écriture.
- Version livrée par défaut dans `Source/Elements/Assets/skins.json`, copiée au build.

## Fichiers impactés
- `Source/HMI/Graphics/SkinCatalog.{h,cpp}` (nouveau) — format, lecture, écriture, résolution.
- `Source/Elements/Assets/skins.json` (nouveau), `Source/HMI/CMakeLists.txt` (copie `POST_BUILD`).
- `Source/Test/Unit/HMI/Graphics/test_skin_catalog.cpp` (nouveau).

## Tests (obligatoires)
- Round-trip lecture → écriture → lecture, jeux multiples compris.
- Fichier absent, JSON invalide, version inconnue, type de tuile inconnu, mode inconnu : chacun
  produit un résultat exploitable **sans exception**.
- Résolution : jeu existant, jeu inexistant → défaut, type non skinné → absence d'entrée.
- Tout est **pur** : aucun GPU, aucun Qt.

## Points d'attention
- **`Core` ne doit jamais voir ce fichier.** `skins.json` est une configuration de présentation ;
  seule la **désignation** du jeu par un niveau (LOT-44) est une donnée de niveau, et ce n'est
  qu'une chaîne.
- Réutiliser la conversion `core::TileType` ↔ chaîne déjà écrite pour le format de niveau
  (`parseTileType` dans `LevelLoader.cpp`) plutôt que d'en écrire une seconde : deux tables de
  noms divergeraient au premier nouveau type de tuile.
- Une version de format **inconnue** (supérieure à celle gérée) doit être signalée clairement, pas
  lue au mieux : c'est précisément ce que le champ sert à éviter.

## Définition de fait (DoD)
- Le format est lu, écrit et résolu sans exception ; les jeux et les modes sont pris en compte ;
  aucune dépendance `Core` ; tests purs verts ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-042` (association type → texture), `EX-EDIT-024` (jeux de skins) ; réutilise
`EX-NFR-040` (erreur récupérable), `EX-NFR-010` (testable sans GPU), `EX-LVL-003` (noms de types de
tuiles du format JSON).
