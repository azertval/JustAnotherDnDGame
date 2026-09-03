# TACHE-05 — Enregistrement, validation, essai immédiat {#lot-14-tache-05-enregistrement-validation-essai}

**Lot :** [LOT-14](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Interface` · **Statut :** fait

> **Ajustement de périmètre à l'implémentation** : le **chargement d'un fichier existant** dans
> l'éditeur (choix nouveau niveau / niveau existant) est finalement traité en TACHE-06, qui
> possède déjà cette responsabilité (« sélection nouveau/existant ») dans son découpage — évite de
> la dupliquer entre deux tâches. Cette tâche livre : enregistrement (`Ctrl+S`), validation avec
> message non-codeur, et **essai immédiat** (`P`), implémenté par une session de jeu **intégrée**
> (un `GameScreen` embarqué dans `EditorScreen`, piloté par délégation d'`update`/`render`) plutôt
> que par une transition `ScreenManager` — `ScreenManager` détruit l'écran quitté à chaque
> transition (`_current = factory(target)`), ce qui aurait perdu le brouillon et l'historique
> undo/redo à chaque essai ; la délégation interne les préserve intégralement, sans changement
> d'architecture de `ScreenManager`/`ScreenTransition`.

## Contexte
Éditer un niveau ne sert à rien s'il ne peut pas être **enregistré** de façon fiable, ni **essayé**
sans quitter l'éditeur. Cette tâche relie l'écran éditeur à la sérialisation/validation de TACHE-01
et au `GameScreen` existant (LOT-09) pour l'essai immédiat (`EX-EDIT-008`).

## Travail à réaliser
- **Enregistrement** : une action (raccourci clavier + éventuel bouton) convertit le `LevelDraft`
  courant via `toLevel()` (TACHE-01) ; en cas d'**échec de validation**, affiche le message d'erreur
  **à l'écran**, en langage compréhensible par un non-codeur (pas de jargon technique — traduire les
  causes de `EX-LVL-004` en phrases claires : « il manque une sortie », « l'interrupteur X n'est
  relié à aucune porte », …) ; en cas de succès, écrit le JSON (TACHE-01) dans
  `Source/Elements/Levels/<nom>.json`.
- **Chargement** : ouvrir un fichier existant de `Source/Elements/Levels` construit un `LevelDraft`
  via `LevelDraft::fromLevel` (après passage par `LevelLoader`) — un échec de chargement (fichier
  corrompu) est **récupérable** (`EX-NFR-040`), affiché sans faire planter l'éditeur.
- **Essai immédiat** : une action (raccourci dédié) fait basculer l'application vers un écran de jeu
  chargeant le niveau **actuellement édité** (converti à la volée, pas nécessairement déjà
  enregistré) ; à la sortie de cet essai (Échap ou fin de niveau), **retour à l'éditeur** avec le
  `LevelDraft` et son historique undo/redo **intacts**.

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.h`/`.cpp`.
- Éventuel ajustement de `Source/HMI/Interface/GameScreen.h`/`.cpp` ou `IScreen`/`ScreenTransition`
  si l'essai immédiat nécessite de transmettre un niveau **en mémoire** plutôt qu'un chemin de
  fichier (`GameScreen` charge aujourd'hui des niveaux par **chemin** uniquement, LOT-09) —
  à trancher : ajouter une construction de `GameScreen` acceptant un `core::Level` déjà en mémoire,
  plutôt que forcer un enregistrement temporaire sur disque avant l'essai.
- Tests d'intégration (`Source/Test/Integration/`).

## Tests (obligatoires)
- Enregistrer un `LevelDraft` **valide** produit un fichier qui se **recharge à l'identique**
  (round-trip bout-en-bout, au-delà du test unitaire de TACHE-01 — ici via le vrai chemin
  enregistrement/chargement de l'éditeur).
- Enregistrer un `LevelDraft` **invalide** (ex. sans sortie) est **refusé**, message affiché,
  **aucun fichier écrit/altéré**.
- Charger un fichier de niveau **corrompu** affiche une erreur récupérable, sans plantage.
- Lancer l'essai immédiat charge bien le niveau en cours d'édition dans le jeu ; revenir à l'éditeur
  restitue l'état d'édition (grille, historique) précédent.

## Points d'attention
- **Aucune perte de travail non enregistré** lors d'un essai immédiat : l'aller-retour
  éditeur → jeu → éditeur ne doit jamais réinitialiser le `LevelDraft` en cours.
- Les messages de validation affichés doivent être **actionnables** pour un non-développeur — éviter
  de simplement afficher `LevelLoadResult::error` brut si son libellé est trop technique ; prévoir une
  traduction/reformulation ciblée si nécessaire.

## Définition de fait (DoD)
- Enregistrement, chargement, validation et essai immédiat opérationnels et testés (`ctest` vert),
  vérifiés dans l'application ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-006`, `EX-EDIT-007`, `EX-EDIT-008`, `EX-EDIT-011`, `EX-LVL-004`, `EX-NFR-040`.
