# LOT-66 — Reproductibilité et référentiel {#lot-66}

> Statut : **fait** (vérification automatisée : build Debug/Release, `ctest` à 100 %, lint
> d'exigences, cahier de test régénéré, séquence démo synchronisée, Doxygen vert ; l'essai réel du
> zip Release hors machine de développement reste à faire avant de poser le tag, voir TACHE-04).
> Prérequis : tous les autres lots du programme `0.1.0` ([LOT-58](@ref lot-58) → [LOT-65](@ref
> lot-65), et [LOT-53](@ref lot-53)) — c'est le **dernier lot avant le tag `v0.1.0`**.

## Objectif
Remettre le dépôt en état d'être publié : que la version se construise à l'identique partout, que
son numéro n'existe qu'à un seul endroit, et que le référentiel d'exigences dise la vérité sur ce
qui est livré.

Trois écarts, tous mineurs pris isolément, tous gênants au moment de tagger :

- **Qt n'est épinglé qu'en CI.** `ci.yml` et `release.yml` installent `6.8.1` ; en local,
  `Source/HMI/CMakeLists.txt` fait un `file(GLOB "C:/Qt/*/msvc*_64")` puis retient la version **la
  plus récente installée**. Un poste avec Qt 6.10 construit contre un autre Qt que la CI, sans que
  rien ne le signale. `EX-BUILD-010` exige une version épinglée sur les **trois** environnements.
- **Le numéro de version est dupliqué.** Le `CMakeLists.txt` racine porte un commentaire affirmant
  que `project(VERSION)` est la « source unique de vérité » — et `Documentation/Doxyfile` contient
  un `PROJECT_NUMBER` qu'il faut aligner **à la main**. `scripts/build_docs.py` ne fait que
  constater la divergence, et `CONTRIBUTING.md` en fait une étape manuelle du processus de
  publication, donc une étape oubliable.
- **Toutes les spécifications sont marquées « brouillon »** : `architecture.md`, `gameplay.md`,
  `vision.md`, `rendu-technique.md`, `controles.md`, `niveaux.md`, `interface-ihm.md`,
  `exigences-non-fonctionnelles.md`. Deux fichiers ont pourtant su adopter un statut de livraison
  (`decors.md`, `editeur-niveaux.md`) : le modèle existe, il n'a pas été appliqué. Publier une
  `0.1.0` sur un référentiel intégralement en brouillon est incohérent.

S'y ajoutent six exigences déclarées et jamais référencées, et la bascule de version elle-même.

## Périmètre

### Inclus
- **Qt épinglé et vérifié** sur les trois environnements, avec un message clair en cas de
  divergence.
- **Numéro de version généré** plutôt que recopié : `PROJECT_NUMBER` dérivé de `project(VERSION)`,
  et retrait de l'étape manuelle du processus de publication.
- **Statuts de spécification** : chaque fichier porte un statut reflétant son état réel de
  livraison, sur le modèle de `decors.md` et `editeur-niveaux.md`.
- **Rattachement ou requalification** des exigences orphelines restantes.
- **Bascule `0.1.0`** : `project(VERSION)`, chapeau de jalon dans le `CHANGELOG.md`, vérification
  des notes de release.

### Exclus (hors périmètre de ce lot)
- Montée de version de Qt : ce lot **épingle** la version en usage, il ne la change pas.
- Réécriture des spécifications : on met à jour les **statuts** et les mentions devenues fausses,
  pas le fond.
- Signature de code, installeur, distribution hors GitHub.
- Le tag lui-même, qui suit le merge, selon le processus de `CONTRIBUTING.md`.

## Décisions de cadrage
- **Dernier lot du programme.** Les statuts de spécification ne peuvent être arrêtés qu'une fois
  connu ce qui est réellement livré ; les figer plus tôt garantirait de les refaire.
- **Épingler, pas verrouiller.** Exiger `6.8.1` au bit près empêcherait de construire sur un poste
  disposant d'une version voisine. Le contrôle vise une **version minimale connue** et un
  **avertissement explicite** en cas d'écart avec la CI — l'objectif est qu'une divergence soit
  visible, pas qu'elle soit interdite.
- **Générer plutôt que vérifier.** `build_docs.py` sait déjà lire les deux valeurs et les comparer :
  lui faire écrire la seconde à partir de la première supprime la classe entière d'erreurs, au lieu
  de la signaler après coup.
- **Un statut par fichier de spécification**, pas un statut global : `architecture.md` est stable
  depuis longtemps, `vision.md` contient encore des propositions à valider. Un statut unique serait
  faux pour l'un ou pour l'autre.
- **Les exigences orphelines ne sont pas toutes des trous.** `EX-ARCH-001`, `EX-ARCH-060`,
  `EX-ARCH-070` et `EX-NFR-032` sont des **invariants transverses** que tout lot respecte sans avoir
  à les citer ; `EX-VIS-002` à `EX-VIS-007` sont des objectifs de vision, réalisés par des lots qui
  référencent les exigences détaillées plutôt que l'objectif. La bonne réponse est de le **dire**
  dans le référentiel, pas de fabriquer des références artificielles. `EX-DEC-031` (paramètres de
  conversion photo → pixel art) est en revanche un vrai reste de post-MVP, à marquer comme tel.

## Exigences couvertes
- Aucune nouvelle exigence.
- Réutilisées : `EX-BUILD-010` (provisionnement reproductible et épinglé de Qt), `EX-NFR-030`
  (construction exclusivement via CMake, reproductible), `EX-NFR-031` (dépendances épinglées),
  `EX-NFR-032` (cible Windows), `EX-NFR-012` (conventions), `EX-ARCH-001` (sens des dépendances),
  `EX-VIS-002` à `EX-VIS-007` (objectifs, requalifiés), `EX-DEC-031` (post-MVP, marqué).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-qt-epingle.md) | Qt épinglé et vérifié sur les trois environnements | `Source/HMI/CMakeLists.txt`, `.github/workflows` | ✅ |
| [TACHE-02](tache-02-version-source-unique.md) | Numéro de version généré, plus jamais recopié | `scripts/build_docs.py`, `Documentation/Doxyfile` | ✅ |
| [TACHE-03](tache-03-statuts-referentiel.md) | Statuts des spécifications et exigences orphelines | `Documentation/Specification` | ✅ |
| [TACHE-04](tache-04-bascule-0-1-0.md) | Bascule `0.1.0` et vérification finale | `CMakeLists.txt`, `CHANGELOG.md` | ✅ |

## Critères d'acceptation du lot
1. Construire sur un poste dont la version de Qt diffère de celle de la CI produit un message
   explicite, et non un échec silencieux ou une divergence invisible.
2. Le numéro de version n'est écrit **qu'une seule fois** dans tout le dépôt ; le modifier à un seul
   endroit suffit, et `build_docs.py` ne peut plus échouer sur une divergence.
3. Chaque fichier de spécification porte un statut reflétant son état réel ; plus aucun n'est
   « brouillon » sans raison.
4. `python scripts/lint_exigences.py` ne signale plus d'exigence orpheline **inexpliquée** : chacune
   est soit référencée, soit explicitement qualifiée d'invariant ou de post-MVP.
5. `python scripts/extract_release_notes.py v0.1.0` produit des notes complètes.
6. La CI complète est verte, jobs Release, ASan, analyse statique et format compris.

## Dépendances
Clôt le programme `0.1.0`. Suppose livrés [LOT-58](@ref lot-58), [LOT-59](@ref lot-59),
[LOT-60](@ref lot-60), [LOT-53](@ref lot-53), [LOT-61](@ref lot-61), [LOT-62](@ref lot-62),
[LOT-63](@ref lot-63), [LOT-64](@ref lot-64) et [LOT-65](@ref lot-65) — ou, pour ceux qui auraient
été rognés, la mise à jour du référentiel en conséquence.

## Navigation des tâches
- @subpage lot-66-tache-01-qt-epingle
- @subpage lot-66-tache-02-version-source-unique
- @subpage lot-66-tache-03-statuts-referentiel
- @subpage lot-66-tache-04-bascule-0-1-0
