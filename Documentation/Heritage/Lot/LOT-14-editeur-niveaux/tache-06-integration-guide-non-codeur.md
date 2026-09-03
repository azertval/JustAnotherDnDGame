# TACHE-06 — Intégration menu, tests système, guide non-codeur Git {#lot-14-tache-06-integration-guide-non-codeur}

**Lot :** [LOT-14](epic.md) · **Emplacement :** `HMI`, `Documentation/Manuel` · **Statut :** fait

> **Ajustement de périmètre à l'implémentation** : le sous-menu nouveau/existant est un
> **sélecteur interne à `EditorScreen`** (`hmi::LevelPicker`), affiché avant l'édition, plutôt
> qu'un état de `MenuScreen` — évite d'exposer une liste de fichiers (donnée d'édition) dans la
> logique du menu principal. Navigation **clavier uniquement** (`↑`/`↓` + `Entrée`, comme
> `MenuModel`) : pas de liste cliquable à la souris pour ce sélecteur, cohérent avec le nombre
> réduit d'entrées attendu et évite une nouvelle géométrie de clic à maintenir pour un gain
> marginal.

## Contexte
Dernière tâche du lot : brancher le mode éditeur au **menu réel** (l'entrée `menu.mode_edition`
existe déjà, LOT-06, mais mène au placeholder), prouver le parcours complet par un test **système**,
et fournir aux non-codeurs le mode d'emploi du partage de niveaux par Git (`EX-EDIT-022`) —
seul élément de ce lot qui n'est **pas** du code.

## Travail à réaliser
- **Menu → éditeur** : `menu.mode_edition` bascule vers un `EditorScreen` qui propose le choix
  **nouveau niveau** (dimensions par défaut ou saisies) ou **niveau existant** (liste des fichiers de
  `Source/Elements/Levels`) avant d'entrer en édition — remplace la construction « toujours vierge »
  provisoire de TACHE-02.
- **Guide non-codeur** (`Documentation/Manuel/`, nouveau fichier, ex. `partager-un-niveau.md`) :
  pas-à-pas **sans ligne de commande** — installer une interface Git graphique (type GitHub
  Desktop), cloner le dépôt, ouvrir l'éditeur, enregistrer un niveau (TACHE-05), **publier** (commit +
  push via l'interface) et **récupérer** les niveaux des autres (pull) — illustré de captures
  d'écran si possible.
- **Tests système** : un parcours bout-en-bout dans `Source/Test/Systeme/` — ouvrir l'éditeur,
  peindre un niveau minimal jouable (entrée, sortie, quelques tuiles), l'enregistrer, le recharger,
  l'essayer, revenir à l'éditeur — sans couche GPU, dans le même esprit que les tests système
  existants du jeu (LOT-09).
- **`CHANGELOG.md`** : entrée décrivant le lot, une fois les six tâches terminées.

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.h`/`.cpp`, `Source/HMI/Interface/MenuScreen.h`/`.cpp` (ou
  équivalent pour le sous-menu nouveau/existant).
- `Documentation/Manuel/partager-un-niveau.md` (nouveau), référencé depuis `Documentation/Manuel/
  manuel.md`.
- `Source/Test/Systeme/` (nouveau parcours éditeur).
- `CHANGELOG.md`.

## Tests (obligatoires)
- Test système : peindre → enregistrer → recharger → essayer → retour éditeur, sans écart d'état
  (assertions sur le `LevelDraft`/fichier produit à chaque étape).
- Sélection **nouveau niveau** produit un `LevelDraft` vierge des dimensions demandées ; sélection
  d'un fichier **existant** produit un `LevelDraft` fidèle à son contenu.

## Points d'attention
- Le guide non-codeur est un **livrable documentaire**, pas du code — mais fait partie des critères
  d'acceptation du lot (`EX-EDIT-022`) au même titre que les tâches précédentes.
- Vérifier qu'aucune régression n'affecte le parcours **jeu** existant (menu → jeu → menu) en
  branchant le nouveau sous-menu de l'éditeur.

## Définition de fait (DoD)
- Parcours menu → éditeur → jeu → éditeur opérationnel et couvert par un test système ; guide
  non-codeur publié ; `CHANGELOG.md` à jour ; build `/W4 /WX` sans avertissement ; Doxygen et `lint`
  des exigences verts — **critères d'acceptation du lot** (voir `epic.md`) tous satisfaits.

## Exigences
`EX-EDIT-001`, `EX-EDIT-020`, `EX-EDIT-022`, `EX-EDIT-030`, `EX-LVL-010`.
