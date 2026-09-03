# TACHE-04 — Bascule `0.1.0` et vérification finale {#lot-66-tache-04-bascule-0-1-0}

**Lot :** [LOT-66](epic.md) · **Emplacement :** `CMakeLists.txt`, `CHANGELOG.md` ·
**Statut :** fait (vérification automatisée ; essai réel du zip Release hors machine de
développement réservé à l'utilisateur avant de poser le tag, voir DoD)

## Contexte
Dernière tâche du programme. Le processus de publication est décrit dans `CONTRIBUTING.md` et il
fonctionne — la `0.0.5` en est la preuve. Ce qui change ici est la **nature** du jalon : les `0.0.x`
jalonnaient la construction du moteur, la `0.1.0` annonce un jeu. Le chapeau du `CHANGELOG.md` doit
le dire, et l'ensemble doit être vérifié sur le binaire réellement publié, pas sur le build local.

## Travail à réaliser
- **Bumper `project(VERSION)`** à `0.1.0` dans le `CMakeLists.txt` racine — désormais le seul
  endroit, grâce à la `TACHE-02`.
- **`CHANGELOG.md`** : transformer `## [Non publié]` en `## [0.1.0] - AAAA-MM-JJ`, rédiger le
  chapeau de jalon (ce que le programme a livré et pourquoi ce jalon change de nature), rouvrir un
  `## [Non publié]` vide au-dessus.
- **Vérifier les notes** : `python scripts/extract_release_notes.py v0.1.0` — le workflow lit cette
  section et **échoue** si elle manque ; mieux vaut le découvrir ici.
- **`README.md`** : la liste des fonctionnalités doit refléter le jeu livré — pause et progression,
  son, effets, mécanismes ajoutés. C'est la première page que voit un visiteur.
- **Recompter les tests** : le chapeau du `CHANGELOG.md` annonce traditionnellement le total (943 à
  la `0.0.5`). Le relever réellement plutôt que l'estimer.
- **Vérification de bout en bout sur le zip publié**, une fois la préversion roulante reconstruite :
  lancer le **Release** sur une machine sans outil de développement, jouer la séquence en quittant
  et relançant en cours de route, vérifier pause, fin de niveau, son, effets, progression, et
  l'existence d'un journal exploitable.

## Fichiers impactés
- `CMakeLists.txt`, `CHANGELOG.md`, `README.md`.

## Tests (obligatoires)
- `python scripts/extract_release_notes.py v0.1.0` produit des notes complètes et lisibles.
- `python scripts/build_docs.py` — la documentation affiche `0.1.0` sans qu'aucun autre fichier
  n'ait été modifié (preuve que la `TACHE-02` tient).
- `python scripts/lint_exigences.py`, `python scripts/generate_cahier_test.py --check`,
  `python scripts/check_demo_sequence.py`.
- `ctest --preset vs` à 100 %, **et** le job Release, le job ASan, l'analyse statique et le contrôle
  de format livrés par [LOT-58](@ref lot-58).
- `gh pr checks` — **tous** les contrôles verts avant merge, pas seulement ceux qu'on a l'habitude
  de regarder.
- Essai réel du zip Release, hors machine de développement.

## Points d'attention
- **Le tag vient après le merge**, sur le commit de merge, conformément à `CONTRIBUTING.md`. Poser
  le tag avant, c'est publier un état qui n'est pas celui de `main`.
- **Ne pas tagger sans avoir essayé le zip.** Jusqu'au `LOT-58`, le build Release n'existait qu'après
  le tag ; le job le rend vérifiable avant, mais seul l'essai du binaire empaqueté couvre le
  déploiement (bibliothèques Qt, module audio, dossiers d'assets).
- Le chapeau de jalon est lu par des non-développeurs : décrire ce que le jeu **fait** désormais,
  pas la liste des lots.
- Si un lot du programme a été rogné, le chapeau et le `README.md` décrivent le livré — pas le
  planifié.
- La préversion roulante `debug-latest` est reconstruite à chaque push sur `main` : elle est le
  moyen le plus simple d'essayer l'état final avant de tagger.

## Définition de fait (DoD)
- `project(VERSION)` vaut `0.1.0`, le `CHANGELOG.md` porte une section datée avec son chapeau de
  jalon et un `Non publié` vide rouvert, le `README.md` décrit le jeu livré, les notes de release
  sont vérifiées, toute la CI est verte, et le zip Release a été essayé de bout en bout sur une
  machine sans outil de développement.

## Exigences
Réutilise `EX-NFR-030` (build reproductible), `EX-NFR-031` (versions épinglées), `EX-NFR-022`
(CI verte), `EX-BUILD-010` (déploiement autonome), `EX-NFR-042` (journal exploitable).
