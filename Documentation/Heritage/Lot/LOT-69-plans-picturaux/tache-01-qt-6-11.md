# TACHE-01 — Passage à Qt 6.11 et provisionnement reproductible {#lot-69-tache-01-qt-6-11}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/HMI`, `.github/workflows`, `Documentation` ·
**Statut :** fait

## Contexte
Le lot repose sur **Qt Canvas Painter**, peinture 2D accélérée sur cible QRhi, disponible pour Qt
Widgets via `QCanvasPainterWidget`. Elle n'existe **qu'à partir de Qt 6.11**, alors que le projet
était épinglé à **6.8.1**. Le portage du rendu (`TACHE-02`) a par ailleurs besoin de `qsb`, le
compilateur de shaders fourni par le module `qtshadertools`.

Quitter Qt 6.8 **LTS** méritait d'être pesé. Le point qui tranche : au-delà de **6.8.3**, les
correctifs de cette branche ne sont publiés que sous **licence commerciale**. Le « support jusqu'en
2029 » ne bénéficiait donc pas à ce projet, et l'argument LTS tombe.

Un obstacle imprévu a été rencontré et levé. Qt a changé la **disposition de son dépôt** à partir de
6.11 : `qt6_6112/qt6_6112_msvc2022_64/` — un sous-dossier par architecture — au lieu de
`qt6_6103/qt6_6103/`. `aqtinstall 3.3.0`, dernière version **publiée sur PyPI**, code en dur
l'ancienne disposition et échoue sur `Failed to download checksum for the file 'Updates.xml'`. Comme
la CI provisionne Qt par `jurplel/install-qt-action`, qui n'est qu'un enrobage d'aqt, la CI héritait
du même blocage.

## Travail à réaliser
- Porter `QT_VERSION_MINIMUM` (`Source/HMI/CMakeLists.txt`) à **6.11.2**, et `env.QT_VERSION` de
  `ci.yml` et `release.yml` à l'identique — le garde-fou `scripts/check_qt_version_pin.py` refuse
  toute divergence.
- Sur les **six** emplois de `jurplel/install-qt-action`, ajouter l'entrée **`aqtsource`**
  (prioritaire sur `aqtversion`) pointant le dépôt git d'aqtinstall, où le correctif est mergé
  (PR #1000, issues #959/#1007). **Épingler le SHA**, jamais `@master` : une branche mobile ferait
  dériver silencieusement le provisionnement, ce que `EX-BUILD-010` et `EX-NFR-031` interdisent.
  Passer par une variable d'environnement unique (`AQT_SOURCE`) pour ne pas répéter l'URL six fois.
- Ajouter les modules `qtshadertools` et `qtcanvaspainter` aux installations.
- Amender `EX-BUILD-010` : l'exigence de provisionnement reproductible porte aussi sur l'**outil de
  provisionnement lui-même**, épinglé à une révision précise, avec motif et condition de sortie
  écrits là où le détour est déclaré.
- Mettre à jour `External/README.md` (référence du provisionnement), `README.md` (prérequis) et
  `CONTRIBUTING.md` (consigne de bump, condition de retrait d'`aqtsource`).

## Fichiers impactés
`Source/HMI/CMakeLists.txt`, `.github/workflows/ci.yml`, `.github/workflows/release.yml`,
`External/README.md`, `README.md`, `CONTRIBUTING.md`,
`Documentation/Specification/exigences-non-fonctionnelles.md`, `CHANGELOG.md`.

## Tests (obligatoires)
Aucun test unitaire : la tâche ne touche aucune ligne de code source. La vérification est la **gate
CI rejouée en local** — `check_qt_version_pin.py`, `lint_exigences.py`,
`generate_cahier_test.py --check`, `check_demo_sequence.py` — puis build Debug et `ctest` complets
contre 6.11.2, qui doivent passer **sans modification du code**. Le YAML des deux workflows est
revalidé après édition.

Signe que le pin a réellement pris : l'avertissement CMake de divergence de version
(`Source/HMI/CMakeLists.txt`, comparaison à `QT_VERSION_MINIMUM`) **disparaît**.

## Points d'attention
Les composants `CanvasPainter` et `ShaderTools` ne sont **pas** ajoutés à `find_package` par cette
tâche, contrairement à ce qu'une lecture rapide du cadrage suggérerait. Dans ce `CMakeLists`, un
composant manquant ne produit pas une erreur mais un `return()` avec avertissement — donc **aucun
exécutable**. Exiger un module qu'aucun code ne consomme encore ferait courir ce risque pour zéro
bénéfice. Ils sont en revanche installés en CI, et comme aqt échoue sur un nom de module inconnu, une
CI verte prouve déjà que le provisionnement les fournit. `TACHE-02` les ajoutera à `find_package` en
même temps que son premier consommateur.

Le détour par git est **temporaire**. Dès qu'`aqtinstall 3.3.1` paraît sur PyPI, remplacer
`aqtsource` par `aqtversion: '==3.3.1'` et supprimer `AQT_SOURCE`. La condition de sortie est écrite
dans les workflows, `External/README.md` et `CONTRIBUTING.md` — sans quoi l'URL git y resterait
des années.

Deux points ne sont vérifiables que par un vrai run CI : que le runner installe bien 6.11.2 via
`aqtsource`, et que le `windeployqt` de 6.11 déploie correctement les DLL à côté du binaire packagé.

## Definition de fait (DoD)
Qt 6.11.2 épinglé de façon identique dans les trois écritures et vérifié automatiquement. Build
Debug et `ctest` à 100 % contre 6.11.2, sans avertissement CMake. Les quatre linters passent.

## Exigences
`EX-BUILD-010` (amendée), `EX-NFR-031`, `EX-NFR-022`.
