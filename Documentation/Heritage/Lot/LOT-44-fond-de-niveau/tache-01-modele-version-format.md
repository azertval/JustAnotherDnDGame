# TACHE-01 — Champs de niveau et version du format {#lot-44-tache-01-modele-version-format}

**Lot :** [LOT-44](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** fait

## Contexte
C'est la **première** extension de `core::Level` du programme d'habillage, et elle en installe le
patron pour les deux suivantes (texture par case en LOT-45, décors en LOT-49).

Deux champs y sont ajoutés ensemble parce qu'ils ont exactement la même nature — une chaîne
optionnelle désignant une ressource de présentation : l'asset de **fond** et le nom du **jeu de
skins** du niveau (`EX-EDIT-024`, dont le format a été posé en LOT-42 sans pouvoir être rattaché à
un niveau).

S'y ajoute le **numéro de version** du format (`EX-LVL-005`) : trois lots vont modifier le schéma
JSON, et l'introduire au premier changement coûte un champ, l'introduire au troisième coûte une
migration.

## Travail à réaliser
- **`core::Level`** : deux accesseurs `std::optional<std::string>` — asset de fond, nom du jeu de
  skins. Le `Level` reste immuable.
- **`core::LevelDraft`** : mutateurs correspondants, champs ajoutés à `LevelDraft::State` pour que
  l'annulation/rétablissement les couvre **sans mécanisme supplémentaire** (patron
  `jumpBudget`/`dashBudget`).
- **`LevelWriter`** : écrit `"version"` et les deux champs racine, ces derniers **omis** quand ils
  sont absents (ne pas polluer les niveaux qui n'en ont pas).
- **`LevelLoader`** : lit `"version"` ; un fichier **sans** version est traité comme la version
  initiale, **sans erreur ni avertissement**. Une version **supérieure** à celle gérée est signalée
  comme une erreur exploitable (`LevelValidationError`) — c'est l'unique raison d'être du champ.
- **`toLevel()`** de `LevelDraft` propage les deux champs.

## Fichiers impactés
- `Source/Core/Levels/Level.h`, `LevelDraft.{h,cpp}`, `LevelLoader.cpp`, `LevelWriter.cpp`.
- `Source/Test/Unit/Core/Levels/` — tests de round-trip, de version et d'annulation.

## Tests (obligatoires)
- **Rétrocompatibilité** : les quinze niveaux de démonstration existants, sans `"version"` ni
  nouveaux champs, se chargent à l'identique — aucune erreur, aucun avertissement, même contenu.
- Round-trip : écrire puis relire un niveau avec fond et jeu de skins restitue les deux champs ;
  sans eux, les clés sont absentes du JSON produit.
- Version supérieure à celle gérée → erreur exploitable, pas une lecture au mieux.
- Annulation/rétablissement d'un changement de fond et de jeu de skins.
- Tout dans `Core`, sans GPU.

## Points d'attention
- **Ce sont des chaînes, jamais des handles.** `Core` ne doit connaître ni dossier d'assets, ni
  format d'image, ni mode de rendu (`EX-NFR-011`).
- La validation de niveau (`EX-LVL-004`) ne doit **pas** vérifier l'existence du fichier de fond :
  `Core` n'a pas accès au dossier d'assets, et un niveau dont le fond est introuvable reste un
  niveau valide (le rendu affichera le repli). C'est une distinction à ne pas confondre.
- Ne pas renuméroter ni réordonner les champs existants du JSON : la lisibilité des diffs de
  niveaux compte pour le suivi de version.

## Définition de fait (DoD)
- Les deux champs existent, transitent par le JSON de façon rétrocompatible, sont couverts par
  l'annulation, et le format porte une version dont l'incompatibilité est détectée ; tests `Core`
  verts ; `/W4 /WX` propre.

## Exigences
`EX-LVL-005` (version du format), `EX-REN-044` (fond de niveau), `EX-EDIT-024` (jeu de skins par
niveau) ; réutilise `EX-LVL-003` (format JSON), `EX-LVL-004` (validation), `EX-EDIT-005`
(annuler/refaire), `EX-NFR-011` (pas de dépendance `Core → HMI`).
