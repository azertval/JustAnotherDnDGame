# TACHE-04 — Panneau Qt « Textures » (section Skins) {#lot-42-tache-04-panneau-textures}

**Lot :** [LOT-42](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/Interface`, `Source/HMI/CMakeLists.txt` · **Statut :** fait

## Contexte
C'est le **premier** panneau du programme d'habillage, et le seul : LOT-44 (Fond), LOT-45 (Objets),
LOT-47 (Animations) et LOT-50 (Décors) y ajouteront leur section plutôt que de créer chacun leur
dock. Sa structure doit donc être sectionnée dès maintenant, sinon chaque lot suivant créera le sien.

Le patron d'enregistrement d'un dock existe déjà et ne doit pas être réinventé : `PalettePanel`,
`ToolPanel`, `LevelBrowserPanel` et `LinkPanel` sont tous instanciés et rattachés par
`hmi::MainWindow`, avec persistance de la disposition via `QSettings` (`EX-IHM-011`).

## Travail à réaliser
- **`TexturePanel`** (`QDockWidget`), organisé en **sections** (onglets ou groupes repliables) :
  - un sélecteur du **jeu de skins courant** parmi ceux de `skins.json` ;
  - une section **« Skins »** listant chaque `core::TileType`, avec son asset assigné et son mode
    (`single` / `bitmask16`), et un sélecteur parmi les fichiers de `Assets/Skins/`.
- **Balayage de dossier** pour peupler le sélecteur — aucune saisie de chemin par l'utilisateur.
- **Écriture** : toute modification enregistre `skins.json` au chemin déployé (TACHE-01).
- **Mise en page hors code** : `Source/Elements/UI/TexturePanel.ui`, conformément à la règle du
  projet (l'IHM doit rester modifiable depuis Qt Designer par un non-développeur).
- **Traduction** : toutes les chaînes via `hmi::Localization`, clés ajoutées à `fr.lang` et
  `en.lang`, méthode `retranslateUi` appelée en cascade depuis `MainWindow::retranslateUi`.
- **Dossier `Assets/Skins/`** + copie `POST_BUILD` dans `Source/HMI/CMakeLists.txt`, sur le patron
  des copies existantes (`Levels`, `Localization`, `Assets`).

## Fichiers impactés
- `Source/HMI/Editor/TexturePanel.{h,cpp}` (nouveau).
- `Source/Elements/UI/TexturePanel.ui` (nouveau).
- `Source/HMI/Interface/MainWindow.{h,cpp}` (enregistrement du dock, retraduction).
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Elements/Assets/Skins/` (nouveau dossier), `Source/HMI/CMakeLists.txt`.

## Tests (obligatoires)
- La logique alimentant le panneau (liste des types à afficher, association courante, effet d'une
  assignation sur le catalogue) est **séparée du widget** et testée sans Qt, sur le modèle de
  `tileTaxonomy` (`TileTaxonomy.{h,cpp}`) déjà testé pour la palette.
- Vérification que chaque clé de traduction utilisée existe dans les deux catalogues.

## Points d'attention
- **Aucun libellé en dur** (`EX-REN-033`) : c'est une règle bloquante du projet, pas une préférence.
- Le panneau ne doit être visible **qu'en mode édition**, comme les autres docks d'édition.
- Concevoir les sections comme un point d'extension : un lot suivant doit pouvoir ajouter la sienne
  sans toucher aux existantes ni au câblage de `MainWindow`.
- Ne pas afficher les trente types dans une liste plate : réutiliser la **taxonomie** de la palette
  (`tileTaxonomy`) pour les regrouper par catégorie, sinon la section est inutilisable.

## Définition de fait (DoD)
- Le dock « Textures » existe, permet de choisir un jeu et d'assigner un asset et un mode à chaque
  type, écrit `skins.json`, est entièrement traduit, et sa logique est testée sans Qt ;
  `/W4 /WX` propre.

## Exigences
`EX-EDIT-042` (association type → texture), `EX-EDIT-024` (jeux de skins) ; réutilise
`EX-REN-033` (traduction), `EX-IHM-010` (fenêtre à panneaux), `EX-IHM-011` (persistance de la
disposition), `EX-EDIT-018` (palette organisée par catégories).
