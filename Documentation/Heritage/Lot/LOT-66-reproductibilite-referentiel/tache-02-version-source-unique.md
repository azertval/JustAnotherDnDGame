# TACHE-02 — Numéro de version généré {#lot-66-tache-02-version-source-unique}

**Lot :** [LOT-66](epic.md) · **Emplacement :** `scripts/build_docs.py`, `Documentation/Doxyfile` ·
**Statut :** fait

## Contexte
Le `CMakeLists.txt` racine porte un commentaire sans ambiguïté : `VERSION` est « la **source unique
de vérité** du numéro de version ». C'est vrai pour le code — `core::Engine::version()` la reçoit
par définition de compilation. Ce ne l'est pas pour la documentation :
`Documentation/Doxyfile` contient `PROJECT_NUMBER = 0.0.5`, une **seconde** écriture du même
nombre.

`scripts/build_docs.py` compare les deux et échoue si elles divergent. C'est mieux que rien, et
c'est le mauvais remède : le contrôle transforme un oubli en échec de CI au lieu de le rendre
impossible. `CONTRIBUTING.md` en fait d'ailleurs une étape manuelle du processus de publication
(« Aligner le `PROJECT_NUMBER` du `Documentation/Doxyfile` »), c'est-à-dire une étape qu'on peut
oublier — au moment précis où l'on est occupé à autre chose.

## Travail à réaliser
- **Retirer la valeur en dur** du `Doxyfile` : `PROJECT_NUMBER` est alimenté par `build_docs.py`
  depuis `project(VERSION)`, par substitution avant l'appel à Doxygen (fichier de configuration
  temporaire, ou paramètre passé sur l'entrée standard — Doxygen accepte les deux).
- **Remplacer la vérification par la génération** dans `build_docs.py` : la fonction qui compare
  aujourd'hui les deux valeurs disparaît au profit de celle qui écrit la seconde.
- **Simplifier `CONTRIBUTING.md`** : l'étape 1 de « Publier une version » se réduit à bumper
  `project(VERSION)`. Une étape de moins, et l'affirmation « seul endroit où le numéro est écrit »
  devient exacte.
- **Vérifier les autres écritures** du numéro dans le dépôt : s'assurer qu'il n'en reste aucune
  ailleurs (README, workflows, page d'accueil de la documentation).
- **Ne pas versionner d'artefact généré** : le `Doxyfile` reste versionné, sans le numéro ; c'est le
  fichier de configuration effectif, temporaire, qui le porte.

## Fichiers impactés
- `scripts/build_docs.py`.
- `Documentation/Doxyfile`.
- `CONTRIBUTING.md`, `CMakeLists.txt` (commentaire à mettre à jour).
- `.gitignore` si un fichier de configuration temporaire est produit dans l'arborescence.

## Tests (obligatoires)
- La documentation générée affiche le numéro de `project(VERSION)` — vérifié sur la sortie, pas
  supposé.
- Modifier `project(VERSION)` seul suffit : la documentation suit, sans autre édition.
- Aucun autre fichier versionné ne contient le numéro de version — vérifié par une recherche, et
  consigné.
- `python scripts/build_docs.py` reste vert avec la version Doxygen épinglée par `ci.yml`.
- Le job `docs` de la CI passe.

## Points d'attention
- **Le `Doxyfile` a d'autres consommateurs potentiels** : lancer `doxygen` à la main depuis
  `Documentation/` doit continuer de fonctionner, avec un numéro vide plutôt qu'un numéro faux. Un
  `PROJECT_NUMBER` vide est acceptable ; un numéro périmé ne l'est pas.
- Lancer Doxygen depuis la racine crée un dossier `generated/` parasite — déjà ignoré par
  `.gitignore`, mais à ne pas réintroduire par un changement de répertoire de travail dans le
  script.
- La version de Doxygen de la CI est épinglée (`DOXYGEN_VERSION`) et diffère de celle d'un poste
  typique : vérifier avec **celle de la CI**, pas avec la locale, sinon le contrôle ne prédit rien.
- Le script est déjà couvert par le job `docs` : le garder simple, il n'a pas de tests unitaires.

## Définition de fait (DoD)
- Le numéro de version n'est écrit qu'à un seul endroit du dépôt, la documentation le reçoit par
  génération, l'étape manuelle a disparu de `CONTRIBUTING.md`, le commentaire du `CMakeLists.txt`
  est devenu exact, et la génération est vérifiée avec la version Doxygen de la CI.

## Exigences
Réutilise `EX-NFR-030` (build reproductible via CMake), `EX-NFR-031` (versions épinglées),
`EX-NFR-012` (conventions).
