# LOT-36 — Refonte IHM (Qt) : gestion des niveaux (liste, recherche, fichiers) {#lot-36}

> Statut : **implémenté** (build/tests verts ; vérification IHM manuelle en cours). Prérequis : [LOT-35](@ref lot-35) (éditeur Qt à docks).

## Objectif
Remplacer le sélecteur de niveau rudimentaire (`LevelPicker` : simple liste défilante, seulement
« Nouveau » / « ouvrir ») par un véritable **panneau de gestion des niveaux** docké, répondant
directement au point de douleur du demandeur (« la liste des niveaux va vite devenir illisible »).
L'utilisateur doit pouvoir **parcourir, rechercher/filtrer, créer, renommer, dupliquer et supprimer**
les fichiers de niveaux confortablement, sans quitter l'éditeur.

## Périmètre

### Inclus
- **Panneau « Niveaux »** (`QDockWidget`) listant les fichiers `.json` du dossier `Levels` de
  l'application, via `QFileSystemModel` (ou un modèle dédié) + `QListView`/`QTreeView`.
- **Recherche / filtre** incrémental par nom (`QLineEdit` + tri/filtre), pour rester lisible quand le
  nombre de niveaux grandit.
- **Opérations de fichiers** depuis le panneau (menu contextuel + boutons) :
  - **Créer** un niveau (nom validé via `hmi::LevelNameValidation`, taille initiale).
  - **Renommer** (validation de nom, gestion des collisions).
  - **Dupliquer** (copie + suffixe unique).
  - **Supprimer** (avec confirmation ; corbeille applicative optionnelle).
- **Ouverture** d'un niveau dans l'éditeur = double-clic / Entrée → `LevelLoader::loadFromFile` puis
  `LevelDraft::fromLevel`, avec **garde-fou des modifications non enregistrées** (proposer
  d'enregistrer avant de changer de niveau).
- **Indicateur d'état** : niveau courant, marque « modifié » (`dirty`), erreurs de chargement
  affichées lisiblement (réutilise `LevelValidationError`).
- Documentation (guide éditeur — section gestion des niveaux) et tests de la logique nouvelle
  (nommage, unicité de duplication, filtrage) découplée de Qt.

### Exclus (hors périmètre de ce lot)
- **Dossiers/catégories de niveaux, campagnes, ordre de séquence de jeu** — la séquence démo reste
  définie dans `main.cpp` (`LOT-25`) ; l'organisation en collections est un lot futur éventuel.
- **Miniatures/aperçu rendu** de chaque niveau dans la liste — raffinement ultérieur (dépend
  potentiellement du LOT-39 textures).
- **Édition simultanée de plusieurs niveaux (onglets multi-documents)** — un seul niveau ouvert à la
  fois, comme aujourd'hui.
- **Liens visuels, textures, menus** — LOT-37/38/39 respectivement.

## Décisions de cadrage
- **S'appuyer sur `QFileSystemModel`/`QSortFilterProxyModel`** plutôt qu'une liste maison : recherche,
  tri et rafraîchissement à chaud viennent gratuitement, cohérent avec l'objectif de maintenabilité.
- **Réutiliser les validateurs et le format existants** : `LevelNameValidation`, `LevelLoader`,
  `LevelWriter` restent l'autorité — aucune règle de nommage ni de sérialisation dupliquée.
- **Opérations de fichiers dans une couche testable** : la logique (nom valide, nom de duplication
  unique, résolution de collision) est isolée de Qt et couverte par tests ; Qt ne fait que la
  déclencher et afficher le résultat.
- **Garde-fou `dirty` obligatoire au changement de niveau** : ne jamais perdre silencieusement un
  brouillon — reprend la sémantique de confirmation de l'éditeur historique.

## Exigences couvertes
- Nouvelles : `EX-IHM-020` (panneau de gestion des niveaux avec recherche), `EX-IHM-021` (opérations
  fichiers : créer/renommer/dupliquer/supprimer avec validation et confirmation).
- Reconduite (présentation Qt, comportement conservé) : `EX-EDIT-001` (choix d'un niveau existant ou
  nouveau à l'ouverture de l'éditeur).
- Réutilisées : `EX-EDIT-006`/`EX-EDIT-007` (enregistrement/validation), `EX-NFR-010` (logique
  testable).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-panneau-niveaux-recherche.md) | Panneau « Niveaux » (modèle fichiers + liste + recherche/filtre) | `Source/Editor` | ✅ |
| [TACHE-02](tache-02-operations-fichiers.md) | Opérations fichiers (créer/renommer/dupliquer/supprimer) + couche testable | `Source/Editor` | ✅ |
| [TACHE-03](tache-03-ouverture-garde-fou-doc.md) | Ouverture avec garde-fou `dirty`, indicateurs d'état/erreurs ; doc & vérification | `Source/Editor`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Le panneau « Niveaux » liste tous les niveaux du dossier `Levels` et reste **lisible et
   filtrable** même avec de nombreux fichiers (recherche par nom fonctionnelle).
2. Créer, renommer, dupliquer et supprimer un niveau fonctionnent depuis le panneau, avec
   **validation de nom** et **confirmation de suppression** ; la liste se rafraîchit.
3. Ouvrir un niveau le charge dans l'éditeur ; si le brouillon courant est **modifié**, l'utilisateur
   est invité à enregistrer avant de changer — aucune perte silencieuse.
4. Les erreurs de chargement d'un fichier invalide sont **affichées lisiblement**, sans crash.
5. La logique de nommage/duplication/filtrage est **couverte par des tests** et découplée de Qt ;
   aucune règle de nommage ou de format dupliquée.
6. Build `/W4 /WX`, Doxygen et lint verts ; IHM Qt **vérifiée visuellement**.

## Dépendances
- Bâtit sur [LOT-35](@ref lot-35). Réutilise `core::LevelLoader`/`LevelWriter`/`LevelDraft`
  (`LOT-07`/`14`), `hmi::LevelNameValidation` (`LOT-14`), et `hmi::executableDirectory()` pour
  localiser `Levels`. Ne modifie pas `Core`.

## Navigation des tâches
- @subpage lot-36-tache-01-panneau-niveaux-recherche
- @subpage lot-36-tache-02-operations-fichiers
- @subpage lot-36-tache-03-ouverture-garde-fou-doc
