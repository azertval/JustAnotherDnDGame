# TACHE-01 — Panneau « Niveaux » (modèle fichiers + liste + recherche/filtre) {#lot-36-tache-01-panneau-niveaux-recherche}

**Lot :** [LOT-36](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
Le choix de niveau historique (`hmi::LevelPicker`) est une simple liste défilante affichée **au
démarrage** de l'éditeur. Le demandeur anticipe qu'elle « va vite devenir illisible ». Cette tâche
introduit un **panneau docké « Niveaux »** toujours visible, listant les fichiers du dossier `Levels`
avec **recherche/filtre**, base des opérations de fichiers (TACHE-02) et de l'ouverture (TACHE-03).

## Travail à réaliser
- **Dock « Niveaux »** (`QDockWidget`) contenant une vue liste (`QListView`, ou `QTreeView` si
  colonnes nom/taille/date) sur le dossier `Levels` (`hmi::executableDirectory() / "Levels"`).
- **Modèle** : `QFileSystemModel` filtré sur `*.json`, **ou** un modèle dédié listant les niveaux (nom
  lisible = nom du niveau, pas seulement le fichier). Un modèle dédié permet d'afficher le **nom
  interne** du niveau (lu via un chargement léger) — à arbitrer (coût de lecture vs simplicité).
- **Recherche/filtre incrémental** : `QLineEdit` + `QSortFilterProxyModel` (filtre insensible à la
  casse sur le nom), tri alphabétique par défaut. Reste lisible à grand nombre de fichiers.
- **Rafraîchissement à chaud** : refléter les créations/suppressions externes (surveillance
  `QFileSystemModel` native, ou rafraîchissement à l'activation de la fenêtre).
- **Sélection courante** mise en évidence ; double-clic/Entrée réservés à l'ouverture (TACHE-03).

## Fichiers impactés
- `Source/Editor/LevelBrowserPanel.{h,cpp}` (dock, vue, modèle, filtre).
- Éventuel `Source/Editor/LevelListModel.{h,cpp}` si modèle dédié.

## Tests (obligatoires)
- **Logique de filtrage testable** : si un modèle/filtre dédié est écrit, tester le filtrage (une
  requête ne retient que les niveaux correspondants) et le tri sur une liste de noms en mémoire, sans
  Qt fenêtré.
- **Vérification manuelle** : le panneau liste les niveaux du dossier ; taper dans la recherche filtre
  la liste ; ajouter/retirer un fichier hors de l'appli met la liste à jour.

## Points d'attention
- **Localiser `Levels`** via `hmi::executableDirectory()` (parité avec `LevelPicker::forDirectory`),
  pas un chemin en dur.
- **Coût de lecture des noms internes** : si on affiche le nom du niveau (pas le nom de fichier),
  éviter de recharger tous les niveaux en continu (cache, lecture paresseuse).
- **Le sélecteur au démarrage** (`LevelPicker`) peut être retiré au profit du panneau permanent, ou
  conservé transitoirement — décider dans le lot (cohérence UX).

## Définition de fait (DoD)
- Panneau « Niveaux » listant et filtrant les niveaux, rafraîchi à chaud ; logique de filtre testée ;
  `/W4 /WX` propre ; vérification manuelle OK.

## Exigences
`EX-IHM-020` (panneau de gestion des niveaux avec recherche) ; reconduit `EX-EDIT-001` (choix d'un
niveau existant/nouveau).
