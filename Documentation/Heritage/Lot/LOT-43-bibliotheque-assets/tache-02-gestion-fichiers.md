# TACHE-02 — Import, renommage, duplication, suppression et détection des références {#lot-43-tache-02-gestion-fichiers}

**Lot :** [LOT-43](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Pour habiller un niveau, l'auteur doit aujourd'hui quitter l'application, copier un PNG dans le bon
sous-dossier avec l'explorateur de fichiers, puis revenir. C'est le genre de friction qui, répétée
des centaines de fois sur six lots, coûte bien plus que la fonctionnalité qui l'évite.

Le précédent exact existe pour les niveaux : `hmi::LevelFileOperations` (créer, renommer, dupliquer,
supprimer) et `hmi::LevelNameValidation`, tous deux **purs et testés**, pilotés par
`LevelBrowserPanel`. Cette tâche en est le pendant pour les assets.

## Travail à réaliser
- **`AssetFileOperations`** (pur, sur le modèle de `LevelFileOperations`) : importer un fichier
  externe dans un sous-dossier d'assets, renommer, dupliquer, supprimer. Chaque opération renvoie un
  résultat exploitable, **jamais** d'exception.
- **Validation du nom** sur le modèle de `LevelNameValidation` : caractères admis, extension,
  collision avec un fichier existant.
- **Validation à l'import** : l'image est décodée et ses dimensions vérifiées contre le contrat de
  la famille d'asset visée (LOT-40) **avant** la copie. Importer un asset invalide pour le
  découvrir en jeu serait une régression par rapport à la validation au chargement.
- **Détection des références** — le cœur de la tâche : avant un renommage ou une suppression,
  déterminer quels **niveaux** (fond, jeu de skins, textures par case, décors) et quelles entrées de
  **`skins.json`** citent l'asset. Les références se font par **nom de fichier** : sans cet
  avertissement, renommer casse silencieusement les niveaux.
- **Avertissement nommant les références**, avec confirmation explicite. On **avertit**, on ne migre
  pas (décision de cadrage) : réécrire toutes les références supposerait de rouvrir et réenregistrer
  chaque niveau.
- Intégration au panneau « Textures » : actions contextuelles sur la grille de vignettes.

## Fichiers impactés
- `Source/HMI/Editor/AssetFileOperations.{h,cpp}` (nouveau).
- `Source/HMI/Editor/AssetReferences.{h,cpp}` (nouveau) — recherche des références.
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/HMI/Editor/AssetThumbnailView.{h,cpp}`.
- `Source/Test/Unit/HMI/Editor/test_asset_file_operations.cpp`,
  `test_asset_references.cpp` (nouveaux).

## Tests (obligatoires)
- Chaque opération de fichier : cas nominal, nom invalide, collision, fichier source absent, cible
  en lecture seule — **sans exception**, sur un dossier temporaire.
- Validation à l'import : dimensions conformes acceptées, non conformes refusées avec un message
  nommant l'attendu.
- **Détection des références** : asset cité par un niveau, par `skins.json`, par les deux, par aucun ;
  asset au nom proche mais différent (pas de faux positif). Fonction pure, testée sans Qt.

## Points d'attention
- La détection des références doit lire les **fichiers de niveaux sur disque**, pas seulement le
  niveau ouvert dans l'éditeur — c'est précisément le cas où l'utilisateur ne peut pas savoir.
- Ne pas supprimer un fichier sans confirmation, même sans référence : la suppression est
  irréversible et l'atelier (LOT-54) produira des assets non sauvegardés ailleurs.
- L'import doit **copier**, jamais déplacer : le fichier source appartient à l'utilisateur.
- Réutiliser `hmi::AssetPaths` pour toute résolution de chemin ; aucun chemin en dur.

## Définition de fait (DoD)
- Importer, renommer, dupliquer et supprimer un asset depuis l'application fonctionne ; un asset
  invalide est refusé à l'import avec un message exploitable ; renommer ou supprimer un asset
  référencé avertit en nommant les références ; toute la logique est pure et testée ;
  `/W4 /WX` propre.

## Exigences
`EX-EDIT-026` (gestion des fichiers d'assets) ; réutilise `EX-REN-007` (contrat d'asset),
`EX-EDIT-042` (associations concernées), `EX-NFR-040` (erreur récupérable), `EX-NFR-010` (testable
sans GPU), `EX-EDIT-021` (garde-fous contre la perte de travail).
