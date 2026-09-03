# TACHE-01 — Plafond de taille et validation « largeur x hauteur » {#lot-16-tache-01-plafond-validation-taille}

**Lot :** [LOT-16](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Input` · **Statut :** fait

## Contexte
`Core` (`TileMap`, `LevelDraft`) n'impose aucune taille maximale — seule l'ergonomie du
redimensionnement (une case par appui de flèche) limitait de fait la taille atteignable. La boîte
de dialogue de TACHE-02 a besoin d'analyser un texte libre (`largeur x hauteur`) et de le valider ;
un plafond est nécessaire pour éviter qu'une saisie erronée (ou des flèches maintenues trop
longtemps) ne produise une grille démesurée.

## Travail à réaliser
- **`hmi::LevelSizeValidation`** (nouveau, `Source/HMI/Editor/`, sur le modèle de
  `LevelNameValidation` de LOT-15) :
  - `constexpr int MAX_LEVEL_DIMENSION = 100;` — plafond par axe, très au-delà des tailles livrées
    à ce jour (12×8 à 14×8) ; commenté comme un choix produit ajustable, pas une contrainte de
    `Core`.
  - `std::optional<std::pair<int, int>> parseLevelSize(const std::string& text)` — analyse le
    format `largeur x hauteur` (séparateur `x`/`X`, espaces tolérés autour), renvoie les deux
    entiers si le texte est bien formé et que chaque valeur est dans `[1, MAX_LEVEL_DIMENSION]` ;
    `std::nullopt` sinon (format incorrect, valeur non entière, nulle/négative, ou hors plafond).
  - `bool isValidLevelSize(const std::string& text)` — `parseLevelSize(text).has_value()`, pour
    servir directement de validateur à `TextInputField` (TACHE-02).
- **`EditorScreen::requestResize`** (existant, LOT-15) : borne systématiquement `width`/`height` à
  `[1, MAX_LEVEL_DIMENSION]` avant tout appel à `LevelDraft::resize`/`wouldResizeDropContent` — les
  flèches ne doivent plus pouvoir dépasser le plafond (elles s'arrêtent silencieusement à la
  borne, comme elles s'arrêtent déjà silencieusement à `1` en réduisant).

## Fichiers impactés
- `Source/HMI/Editor/LevelSizeValidation.h`/`.cpp` (nouveau).
- `Source/HMI/Interface/EditorScreen.cpp` (bornage dans `requestResize`).
- Tests unitaires (`Source/Test/Unit/HMI/Editor/test_level_size_validation.cpp`).

## Tests (obligatoires)
- `parseLevelSize` : `"40x30"`, `"40 x 30"`, `"40X30"` acceptés et renvoient `(40, 30)` ; texte
  vide, sans séparateur, avec une valeur non numérique, nulle, négative, ou dépassant
  `MAX_LEVEL_DIMENSION` refusés (`nullopt`).
- `isValidLevelSize` cohérent avec `parseLevelSize` (vrai/faux selon la présence d'une valeur).
- `requestResize` appelé avec une largeur/hauteur dépassant `MAX_LEVEL_DIMENSION` la borne à cette
  valeur avant d'agir (vérifiable via l'état résultant de `LevelDraft` — voir TACHE-02/03 pour le
  câblage complet de la boîte de dialogue qui consomme ce validateur).

## Points d'attention
- `MAX_LEVEL_DIMENSION` est **un seul** point de vérité, importé aussi bien par le validateur de
  la boîte de dialogue que par `requestResize` — ne pas dupliquer la constante.
- Le plafond ne touche à aucune règle de `Core` : `LevelDraft::resize`/`TileMap` restent inchangés
  et sans limite (`EX-NFR-010`), cohérent avec la décision de cadrage de l'épic (le plafond est un
  garde-fou d'usage porté par `HMI`, pas une invariante de donnée).

## Définition de fait (DoD)
- Plafond et validation du format disponibles et testés (`ctest` vert) ; build `/W4 /WX` ; Doxygen
  à jour.

## Exigences
`EX-EDIT-017` (partie « plafond »).
