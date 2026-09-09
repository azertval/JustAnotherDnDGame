# Contribuer à JustAnotherDnDGame

## Conventions de code
Voir [`Documentation/Specification/conventions.md`](Documentation/Specification/conventions.md). Le code doit être formaté (`clang-format`) et compiler sans avertissement avant tout commit.

## Messages de commit — Conventional Commits
Format : `<type>(<portée facultative>): <description à l'impératif>`

Types :
| Type | Usage |
|------|-------|
| `feat` | Nouvelle fonctionnalité |
| `fix` | Correction de bug |
| `docs` | Documentation seule |
| `refactor` | Refonte sans changement de comportement |
| `test` | Ajout/modification de tests |
| `build` | Build, CMake, dépendances |
| `ci` | Intégration continue |
| `chore` | Tâche diverse (config, outillage) |

Exemples :
```
feat(core): ajouter la détection de collision AABB
fix(hmi): corriger le ratio d'aspect au redimensionnement
docs(conventions): préciser la politique de gestion d'erreurs
```
La portée correspond en général au module (`core`, `hmi`, `elements`, `test`, `build`…).

## Stratégie de branches
- `main` est **protégée** : **aucun push direct**. Toute évolution passe par une **Pull Request**.
- **Une branche par lot** : `lot/LOT-XX-nom-du-lot` (ex. `lot/LOT-03-fondation-ecs`).
  - Pour un correctif isolé hors lot : `fix/...` ; pour de la doc seule : `docs/...`.
- Ouvrir une **Pull Request** vers `main`. Le merge exige une **CI verte** (build + tests + couverture, zéro avertissement).
- `main` reste **toujours compilable et testée**.

## Automatisations sur `main` (après merge d'une PR)
- **CI** (`.github/workflows/ci.yml`) : s'exécute sur chaque PR ; contrôle requis pour merger.
- **Release** (`release.yml`) : à chaque push sur `main`, republie l'exécutable Debug autonome
  dans la **Release roulante `debug-latest`** (préversion, toujours à jour). À chaque tag
  `vX.Y.Z` poussé, publie une **Release versionnée** (non préversion) avec les exécutables
  **Debug et Release**, chacun autonome — destinés aux non-développeurs (télécharger,
  décompresser, lancer).

## Publier une version
1. Bumper `VERSION` dans le `project()` du `CMakeLists.txt` racine — **seul** endroit où le numéro
   est écrit : il alimente `core::Engine::version()` à la compilation, et `scripts/build_docs.py`
   l'injecte dans la documentation générée. Rien d'autre à aligner à la main.
2. Dans `CHANGELOG.md`, transformer `## [Non publié]` en `## [X.Y.Z] - AAAA-MM-JJ`, lui ajouter un
   chapeau de jalon, et rouvrir un `## [Non publié]` vide au-dessus.
3. Vérifier les notes que produira la release :
   `python scripts/extract_release_notes.py vX.Y.Z` — le workflow lit **cette** section du
   CHANGELOG (`--notes-file`) et **échoue** si elle est absente.
4. Merger, puis poser le tag sur le commit de merge : `git tag vX.Y.Z && git push origin vX.Y.Z`.
- **Documentation** (`docs.yml`) : génère la Doxygen et la publie sur la branche **`gh-pages`** (lisible en ligne via GitHub Pages).

## Avant d'ouvrir une PR
1. `cmake --build --preset vs` compile sans avertissement.
2. `ctest --preset vs` passe à 100 %.
3. `cmake --preset vs && cmake --build --preset vs-release && ctest --preset vs-release` compile et
   teste en configuration **Release** (LOT-58) : certaines casses (variable lue uniquement par une
   assertion, code conditionné à `core::kDeveloperBuild`) ne se voient qu'ici.
4. Le code est formaté (`clang-format`) et les nouveaux comportements sont couverts par des tests.
   Vérifié en CI (LOT-58) avec une version **épinglée** (`LLVM_VERSION` dans `ci.yml`) : deux
   versions majeures ne formatent pas identiquement. Reproduire localement (même version,
   installée en isolation via le paquet PyPI qui redistribue les binaires officiels LLVM, sans
   dépendre de celle fournie par l'IDE) :
   `pip install "clang-format==$LLVM_VERSION" && git ls-files 'Source/*.cpp' 'Source/*.h' | xargs clang-format --dry-run --Werror --style=file`
5. `clang-tidy` sur les fichiers `Source/*.cpp` modifiés (LOT-58) :
   `cmake -S . -B build/ninja-tidy -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DENABLE_PCH=OFF`
   puis `clang-tidy -p build/ninja-tidy <fichier.cpp>` (version LLVM épinglée : `LLVM_VERSION` dans
   `ci.yml`). Seules les violations `bugprone-*` font échouer la CI ; les autres familles
   (`cppcoreguidelines-*`, `modernize-*`, `performance-*`, `readability-*`) restent visibles mais
   non bloquantes (triage complet hors périmètre du `LOT-58`, voir
   `Documentation/Lot/LOT-58-verification-release-analyse/tache-03-clang-tidy.md`).
6. Le `CHANGELOG.md` (section *Unreleased*) est mis à jour si pertinent.
7. Si `QT_VERSION_MINIMUM` (`Source/HMI/CMakeLists.txt`) a changé, `env.QT_VERSION` de `ci.yml` et
   `release.yml` doit être bumpé à l'identique — vérifié automatiquement par
   `python scripts/check_qt_version_pin.py` (job `lint-exigences`), pas seulement par relecture.
   Depuis le `LOT-69`, la CI installe Qt avec un `aqtinstall` pris **depuis git à un commit
   épinglé** (`env.AQT_SOURCE`), la version PyPI ne sachant pas installer Qt ≥ 6.11 : dès
   qu'`aqtinstall 3.3.1` paraît, remplacer `aqtsource` par `aqtversion: '==3.3.1'` et supprimer
   `AQT_SOURCE`. Le motif complet est dans [`External/README.md`](External/README.md).
8. Les mises en page des **écrans d'identité** (`MainMenu`, `OptionsPage`, `CreditsScreen`,
   `PauseScreen`, `RpgCharacterSheetPlate`) s'éditent **dans Qt Designer** (`EX-IHM-006`, `LOT-85`),
   lancé par le script, et jamais `designer.exe` à la main :

   ```
   powershell -ExecutionPolicy Bypass -File scripts/designer.ps1 -Path <fichier.ui>
   ```

   `powershell` et non `pwsh` : PowerShell 7 n'est pas installé sur le poste de développement, et
   l'invoquer donne un « command not found » qui ne dit pas lequel des deux manque. Le script bâtit
   le plugin des widgets promus **en Release** (Designer rejette une DLL Debug sans le moindre
   message) et régénère le thème que Designer affiche. Deux règles y tiennent :
   - **aucun commentaire XML dans ces `.ui`** — Designer les efface à l'enregistrement ; la
     justification de la mise en page vit dans l'en-tête C++ de l'écran ;
   - ne reste construit en code que ce qu'une **donnée engendre en nombre variable**. Une rangée
     fixe écrite dans le constructeur est un morceau de maquette que Designer ne montre pas.

   Après toute retouche des jetons de design ou de `theme-identity.qss`, relancer
   `scripts/designer.ps1` (ou `IdentityQss` puis `python scripts/sync_ui_theme.py`) pour que le
   thème déposé dans les `.ui` décrive encore le jeu — vérifié par
   `python scripts/sync_ui_theme.py --check` et `python scripts/check_ui_designer.py`.
