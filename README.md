# JustAnotherDnDGame

[![CI](https://github.com/azertval/JustAnotherDnDGame/actions/workflows/ci.yml/badge.svg)](https://github.com/azertval/JustAnotherDnDGame/actions/workflows/ci.yml)
[![Documentation](https://github.com/azertval/JustAnotherDnDGame/actions/workflows/docs.yml/badge.svg)](https://github.com/azertval/JustAnotherDnDGame/actions/workflows/docs.yml)
[![Release](https://github.com/azertval/JustAnotherDnDGame/actions/workflows/release.yml/badge.svg)](https://github.com/azertval/JustAnotherDnDGame/releases/latest)
![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C)
![Qt QRhi](https://img.shields.io/badge/Qt%20QRhi-Direct3D%2011-8A2BE2)

RPG 2D **en vue de dessus** développé **from scratch** en **C++20** (Windows), sans moteur
tiers : exploration en temps réel façon Zelda, et rencontres en **combat tactique au tour par
tour** régi par un système **d20** maison. Le rendu passe par **QRhi**, la couche d'accès au GPU
de Qt, qui retient **Direct3D 11** par défaut sous Windows.

> **Dérivé de `ProjectGaming`.** Ce dépôt reprend le moteur d'un jeu de plateforme/puzzle en vue
> de côté livré en `0.1.3` (ECS, boucle à pas fixe, balayage AABB, chargeur de niveaux, rendu,
> éditeur, IHM Qt) et en a retiré tout le gameplay propre à la vue de côté. L'historique git est
> conservé ; le programme de lots d'origine reste consultable sous `Documentation/Heritage/`.
> État actuel : le moteur est nu — le déplacement top-down arrive au `LOT-06`.

- 📖 **Documentation en ligne** : <https://azertval.github.io/JustAnotherDnDGame/>
- ⬇️ **Télécharger la dernière version** : <https://github.com/azertval/JustAnotherDnDGame/releases/latest>
  (préversion roulante du dernier `main` : <https://github.com/azertval/JustAnotherDnDGame/releases/tag/debug-latest>)

## Description

JustAnotherDnDGame est un RPG en vue de dessus bâti sur un moteur de jeu 2D maison. Ses partis
pris :

- **Séparation stricte** entre la logique (`Core`) et la présentation (`HMI`) :
  `Core` est indépendant du GPU et de la fenêtre, donc **testable sans GPU**.
- **ECS maison** (sparse sets) : entités = identifiants, composants = données pures,
  systèmes = logique, exécutés à **pas de temps fixe déterministe**.
- **Rendu pixel art** via **QRhi** (tuiles 16 px, échantillonnage *nearest*).
- **Éditeur de cartes** intégré pour permettre à des non-développeurs de créer du
  contenu (peinture, mécanismes, undo/redo, essai immédiat).
- **Règles chiffrées en données** : classes, sorts, objets et ennemis se définissent en JSON,
  jamais en dur dans le C++ — l'équilibrage ne demande pas de recompiler.

Le *quoi* et le *pourquoi* sont décrits dans les
[spécifications](https://azertval.github.io/JustAnotherDnDGame/) ; le *comment* dans le
**Guide du développeur** et la référence de code Doxygen.

## Fonctionnalités du moteur (état actuel)

> **Le jeu n'est pas encore jouable.** Le `LOT-01` a retiré le gameplay de plateforme hérité ; le
> déplacement top-down 8 directions arrive au `LOT-06`. Le moteur ci-dessous est en revanche
> complet et éprouvé — c'est précisément ce que le fork conserve.

- **Collisions** par **balayage continu** (swept AABB) : aucune traversée à vitesse élevée, sans
  gravité ni axe privilégié — la primitive exacte dont un déplacement en vue de dessus a besoin.
- **Niveaux** en tuiles typées, de taille arbitraire, chargés depuis des fichiers **JSON**
  versionnés, avec **validation** et migration ascendante.
- **Mécanismes** interrupteur ↔ porte, **plaque de pression**, **clé et porte verrouillée**, action
  **Interagir** — vocabulaire de puzzle conservé tel quel pour le RPG.
- **Éditeur de niveaux** intégré : peinture à la souris, outils rectangle/sélection, liaison de
  mécanismes, undo/redo, essai immédiat ; le level designer choisit le **cadrage de caméra** (par
  salle, niveau entier ou suivi).
- **Menu** multilingue (fr/en), **menu d'options** (V-Sync, volume, langue), **pause**,
  **progression persistée**, navigable au **clavier, à la souris et à la manette** (XInput).
- **Bruitages** (Qt Multimedia) à volume réglable, jeu pleinement utilisable en silence sans
  périphérique audio. **Effets de particules** et secousse d'écran.
- **Diagnostics** : compteur de budget de rendu affichable (`F9`), journal de session sur disque.
- **Hasard déterministe** (splitMix64, graine dérivée par entité et par pas) : socle des futurs
  jets de dés reproductibles.

Le moteur est **habillé** :

- **Rendu texturé multicouche** avec culling : **fond** de niveau, **plans picturaux** avec
  **parallaxe**, **ombres**, premier plan au-dessus du personnage. Bascule Physique/Texture par
  `F8`, et mode d'inspection par calque pour auditer un habillage.
- **Skins de tuiles** avec **raccords automatiques** (16 voisinages), **texture par instance**,
  **repli procédural** déterministe quand un asset manque — le jeu reste lançable sans aucun
  fichier d'image.
- **Animation pilotée par données** (`nom-asset.anim.json`) : clips nommés, bouclés ou joués une
  fois, apparence des mécanismes suivant leur **état logique**.
- **Texte dans la scène** : police bitmap avec repli procédural, affichage tête haute.

Et l'**éditeur** est un poste de travail complet :

- **Bibliothèque d'assets** à vignettes (import, renommage, duplication, suppression) avec
  **rechargement à chaud**, et détection des assets encore référencés avant suppression.
- **Atelier pixel art intégré** : dessiner, remplir, pipetter, transformer une région, gérer des
  palettes et voir l'aperçu des raccords — sans quitter l'application.
- **Système de design** : barre d'outils à icônes, thème **clair/sombre** suivant le système,
  vignettes nettes à toute échelle d'affichage, barre d'état permanente, raccourcis d'éditeur
  remappables.

Toute la simulation vit dans `Core` (pure, déterministe au pas fixe) et est **couverte par des
tests** (unitaires, intégration, système) — voir le **Cahier de test**.

## Organisation du dépôt

| Dossier | Rôle |
|---------|------|
| `Documentation/` | Documentation publiée en site **Doxygen** : `Specification/` (specs & conventions), `Lot/` et `Lot-Annexe/` (lots de travail), `Manuel/` (manuel utilisateur), **Guide du développeur** et **Guide annexe** (notions d'apprentissage automatique), **Cahier de test**, et référence de code. |
| `Source/` | Code source, réparti par fonction. |
| `.github/workflows/` | Intégration continue (voir plus bas). |

### Découpage de `Source/`

| Sous-dossier | Contenu |
|--------------|---------|
| `Core/` | Logique et moteur : ECS, mathématiques, boucle à pas fixe, diagnostics — **sans dépendance au GPU ni à la fenêtre**. |
| `HMI/` | Présentation : l'application **Qt** (fenêtre, menu, options, éditeur), le rendu du jeu (via **QRhi**) embarqué dans un viewport, et les entrées. Dépend de `Core`, jamais l'inverse. |
| `AiSolver/` | Solveur **IA** maison, écrit sans framework d'apprentissage automatique : tenseurs, autodiff, réseaux de neurones, optimiseurs, environnement de simulation sans fenêtre, algorithmes d'entraînement et outil `aisolver-cli`. Dépend de `Core`, jamais l'inverse. |
| `Elements/` | Assets et éléments statiques (sprites, tuiles, sons, niveaux). |
| `Test/` | Tests **unitaires** (`Unit/`), **d'intégration** (`Integration/`) et **système** (`Systeme/`) — GoogleTest. |

## Build

Le projet se construit **exclusivement via CMake**. Visual Studio est utilisé comme IDE
grâce à son intégration CMake native (aucun `.vcxproj`/`.sln` versionné : ils sont
générés dans `build/`).

### Prérequis
- Visual Studio 2022+ avec la charge de travail **« Développement Desktop en C++ »**
  (inclut CMake, Ninja et le compilateur MSVC).
- **Qt6** (`Widgets`, `Gui`, `Multimedia` — module audio, `LOT-60`), version **6.11.2 ou
  supérieure** (celle validée par la CI — un écart produit un avertissement à la configuration,
  pas un échec), détecté automatiquement (`CMAKE_PREFIX_PATH`, cf.
  `Source/HMI/CMakeLists.txt`) s'il est installé à l'emplacement conventionnel de
  l'[installateur officiel](https://www.qt.io/download-qt-installer) ou via
  [`aqtinstall`](https://github.com/miurahr/aqtinstall)
  (`-m qtmultimedia qtshadertools qtcanvaspainter`). Qt 6.11 est le **plancher** depuis le
  `LOT-69` : c'est la première version fournissant Qt Canvas Painter. Installer Qt ≥ 6.11 avec
  `aqtinstall` demande une version de l'outil plus récente que celle publiée sur PyPI — voir
  [`External/README.md`](External/README.md).
  Sans Qt, la cible `JustAnotherDnDGame` est **ignorée** (avertissement explicite) : seuls les tests se
  construisent.

### Depuis Visual Studio (recommandé)
1. `Fichier > Ouvrir > Dossier…` puis sélectionner la racine du dépôt.
2. VS détecte `CMakeLists.txt` et `CMakePresets.json`.
3. Choisir le preset `vs` (ou `ninja`) dans la barre d'outils, puis générer.

### En ligne de commande
```sh
cmake --preset vs        # configure (ou : ninja)
cmake --build --preset vs
ctest --preset vs        # lance les tests
```

Presets **Release** (`vs-release`, `ninja-release`) : mêmes commandes de build/test avec
`--preset vs-release` ou `--preset ninja-release` (configurer avec `ninja-release` pour ce dernier).
Vérifiés en CI (LOT-58) : certaines casses (variable lue uniquement par une assertion, `NDEBUG`)
n'apparaissent qu'en Release.

> Reproductible sur plusieurs postes : tout est versionné sauf `build/` (local).
> GoogleTest est récupéré automatiquement par CMake (FetchContent).

## Process d'implémentation

Le travail avance par **lots** (un incrément livrable par lot), décrits dans
`Documentation/Lot/` (un `epic.md` + des `tache-NN.md`).

- **Branches** : `main` est **protégée** (aucun push direct). Une **branche par lot**
  (`lot/LOT-XX-nom`) ; correctifs isolés en `fix/…`, documentation seule en `docs/…`.
- **Pull Requests obligatoires** vers `main`, avec **CI verte** requise pour merger.
  `main` reste toujours compilable et testée.
- **Commits** : [Conventional Commits](https://www.conventionalcommits.org/) en français
  (`feat`, `fix`, `docs`, `refactor`, `test`, `build`, `ci`, `chore`).
- **Conventions de code** : nommage, RAII, documentation Doxygen (`.h` **et** `.cpp`),
  gestion d'erreurs — voir
  [`Documentation/Specification/conventions.md`](Documentation/Specification/conventions.md).
  Le code compile **sans avertissement** (`/W4 /WX`).
- **Tests** : toute logique de `Core` est couverte par des tests (unitaires et, au besoin,
  d'intégration). `CHANGELOG.md` est tenu à jour.
- **Traçabilité** : les exigences `EX-…` (spécifications) sont des identifiants stables,
  vérifiés en CI (`scripts/lint_exigences.py`).

Détails dans [`CONTRIBUTING.md`](CONTRIBUTING.md).

### Vérifications locales

Les mêmes contrôles qu'en intégration continue, tous lançables **depuis la racine du dépôt** :

```sh
python scripts/lint_exigences.py           # identifiants EX-… : ni doublon, ni orphelin
python scripts/lint_exigences.py --next    # prochain numéro libre, par catégorie
python scripts/generate_cahier_test.py --check   # cahier de test à jour
python scripts/build_docs.py               # documentation Doxygen (WARN_AS_ERROR)
```

> `build_docs.py` existe parce que Doxygen résout les chemins de son fichier de configuration
> relativement au **répertoire courant**, et non à l'emplacement du `Doxyfile` : lancer
> `doxygen Documentation/Doxyfile` depuis la racine échoue (`source '…' is not a readable file`).
> Le script se place dans `Documentation/` pour vous, quel que soit le répertoire d'appel.

**Version de Doxygen : `1.16.1`**, épinglée en CI (`EX-NFR-031`) et à installer à l'identique en
local. Les versions plus anciennes appliquent des règles de résolution de liens différentes : une
documentation générée sans avertissement avec une autre version peut échouer en CI, et la
vérification locale ne prédirait plus rien. Binaires officiels :
<https://github.com/doxygen/doxygen/releases/tag/Release_1_16_1>.

## Intégration continue

Chaque job ci-dessous est un **contrôle requis pour merger** (protection de branche), à l'exception
de `docs` (`docs.yml`, informatif).

| Workflow | Job | Déclencheur | Rôle |
|----------|-----|-------------|------|
| **CI** (`ci.yml`) | `build-test-coverage` | PR vers `main` | Build + tests (CTest) **Debug** sur `windows-2022`, **couverture** agrégée `UnitTests`+`IntegrationTests`+`SystemTests` avec seuil (LOT-58). |
| **CI** (`ci.yml`) | `build-test-release` | PR vers `main` | Build + tests **Release** (LOT-58) : une casse Release-only (variable inutilisée sous `NDEBUG`…) est refusée avant le tag, pas après. |
| **CI** (`ci.yml`) | `build-ninja` | PR vers `main` | Build + tests via le générateur Ninja (détection Qt automatique). |
| **CI** (`ci.yml`) | `sanitize` | PR vers `main` | Les trois exécutables de test sous **AddressSanitizer** (LOT-58, `EX-NFR-003`). |
| **CI** (`ci.yml`) | `clang-tidy` | PR vers `main` | Analyse statique sur le diff de la PR ; `bugprone-*` bloquant, le reste consigné (LOT-58). |
| **CI** (`ci.yml`) | `format` | PR vers `main` | `clang-format --dry-run --Werror`, version épinglée (LOT-58). |
| **CI** (`ci.yml`) | `lint-exigences` | PR vers `main` | Identifiants `EX-…`, cahier de test à jour, séquence de niveaux démo. |
| **Documentation** (`docs.yml`) | `docs` | Push sur `main` | Génère la **Doxygen** (garde-fou `WARN_AS_ERROR`) et la publie sur la branche `gh-pages` (site en ligne). |
| **Release** (`release.yml`) | `rolling-debug` | Push sur `main` | Compile un exécutable **Debug autonome** et publie la préversion roulante **`debug-latest`** pour les non-développeurs. |
| **Release** (`release.yml`) | `versioned-release` | Tag `vX.Y.Z` | Publie une **release versionnée** (non préversion) avec les exécutables **Debug et Release**, chacun autonome. |

> « Autonome » signifie qu'aucune installation n'est requise côté utilisateur : `windeployqt` dépose
> les DLL Qt, le plugin de plateforme et le runtime du compilateur à côté de l'exécutable. Le
> runtime MSVC est **dynamique** (`/MD`) et non statique — les DLL Qt officielles sont construites
> ainsi, et un CRT statique provoquerait des incohérences d'allocation entre l'application et Qt.

## Licence

JustAnotherDnDGame est distribué sous **GNU General Public License v3.0 ou ultérieure**
(`GPL-3.0-or-later`). Le texte complet est dans [`LICENSE`](LICENSE).

```
Copyright (C) 2026 Valentin Eloy

Ce programme est un logiciel libre : vous pouvez le redistribuer et/ou le modifier
selon les termes de la GNU General Public License telle que publiée par la Free
Software Foundation, soit la version 3 de cette licence, soit (à votre choix)
toute version ultérieure.

Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de COMMERCIALISATION
ou d'ADAPTATION À UN USAGE PARTICULIER. Voir la GNU General Public License pour
plus de détails.
```

Ce que cela implique concrètement, et qui n'est pas toujours évident :

- **L'usage commercial n'est pas interdit** — une licence open source ne peut pas le restreindre
  (définition de l'Open Source, clause 6). Ce que la GPL garantit à la place est plus fort dans les
  faits : *toute* redistribution, commerciale ou non, d'une version modifiée **doit en publier le
  source** sous la même licence. Personne ne peut refermer le projet.
- **Le copyleft se propage.** Tout code lié à JustAnotherDnDGame doit être compatible GPL-3.0.
- **Qt reste sous LGPLv3**, en lien dynamique — les deux licences sont compatibles, la LGPLv3
  autorisant explicitement la redistribution sous GPLv3. Les obligations propres à Qt sont
  détaillées dans [`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md).
- Les **ressources** (graphismes, sons, polices) ont leurs propres licences — CC0 et SIL OFL —
  indépendantes de celle du code, et recensées dans le même fichier.
- Les en-têtes `SPDX-License-Identifier: GPL-3.0-or-later` marquent le **code** du projet, et lui
  seul. Un `.png` ou un `.wav` du dépôt n'est pas sous GPL pour autant.

Ces mentions sont aussi affichées **dans le jeu**, à l'écran *Crédits* — la LGPLv3 de Qt et la SIL
OFL des polices l'exigent, et un joueur qui n'ouvrira jamais ce dépôt doit pouvoir les lire.
