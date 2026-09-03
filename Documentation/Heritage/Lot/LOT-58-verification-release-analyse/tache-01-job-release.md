# TACHE-01 — Job CI de vérification en configuration Release {#lot-58-tache-01-job-release}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `.github/workflows`, `CMakePresets.json` ·
**Statut :** fait

## Contexte
`ci.yml` a deux jobs de build, et tous deux sont en **Debug** : le preset `vs` construit et teste
avec `--config Debug`, le preset `ninja` fixe `CMAKE_BUILD_TYPE=Debug`. La configuration Release
n'apparaît que dans `release.yml`, au job `versioned-release`, déclenché **par le tag**.

L'ordre des opérations est donc : on tague, puis on découvre si le code compile en Release. Un tag
est public et ne se rétracte pas proprement. Le dépôt garde la trace d'un cas réel —
`90f85254 fix(core): correction d'une variable non utilisee en Release`, sur `LevelDraft` : une
variable lue uniquement par une assertion, donc inutilisée sous `NDEBUG`, donc `/W4 /WX` en échec.
La famille est classique et régulière : assertions portant la seule lecture d'une variable,
`if constexpr (core::kDeveloperBuild)` masquant un paramètre, code de journalisation compilé hors
Release.

## Travail à réaliser
- **Presets Release** : ajouter un preset de build et un preset de test en configuration Release
  pour le générateur Visual Studio, et un preset de configuration `ninja-release`
  (`CMAKE_BUILD_TYPE=Release`) — la symétrie avec les presets Debug existants est le but.
- **Job `build-test-release`** dans `ci.yml` : installation de Qt (identique aux autres jobs),
  configuration, build et `ctest` en Release, plus le contrôle « l'exécutable est bien produit »
  déjà présent ailleurs.
- **Contrôle requis pour merger** : le job rejoint la protection de branche, au même titre que
  `build-test-coverage`.
- **Vérifier que le job attrape réellement la faute** : introduire temporairement une variable
  utilisée uniquement dans une assertion, constater l'échec, retirer.

## Fichiers impactés
- `CMakePresets.json` — presets de build et de test Release.
- `.github/workflows/ci.yml` — nouveau job.
- `CONTRIBUTING.md` — la liste « avant d'ouvrir une PR » mentionne la vérification Release.
- `README.md` — section build, si les presets y sont énumérés.

## Tests (obligatoires)
- Le job passe sur l'état actuel de `main` (sinon, le corriger fait partie de cette tâche).
- **Test négatif obligatoire** : sur une branche jetable, une variable lue seulement par un
  `CORE_ASSERT` fait échouer le job Release et **pas** le job Debug. C'est la preuve que la tâche
  sert à quelque chose ; sans elle, on ajoute un job qui ne vérifie rien.
- `ctest` en Release passe à 100 % : les tests dépendant d'assertions actives doivent être
  identifiés, et adaptés ou explicitement conditionnés.

## Points d'attention
- **Certains tests peuvent dépendre du comportement Debug** — notamment ceux de
  `Source/Test/Unit/Core/Diagnostics/test_assert.cpp`. S'ils échouent en Release, la bonne réponse
  est de les conditionner explicitement à `core::kDeveloperBuild`, pas de désactiver le job.
- La physique en virgule flottante peut différer entre Debug et Release (optimisations,
  contraction de multiplication-addition). Si un test de tolérance échoue, resserrer le **test**,
  pas la vérification : c'est exactement le genre de fragilité que ce job existe pour révéler.
- Durée de CI : le job est parallèle aux autres, mais Qt et le build s'ajoutent. Mesurer et
  consigner.
- Ne pas dupliquer la logique d'installation de Qt : les jobs existants ont déjà le bon bloc, avec
  le cache.

## Définition de fait (DoD)
- La CI construit et teste en Release sur chaque PR, le job est requis pour merger, une casse
  Release-only est démontrée refusée par un test négatif, `ctest` Release est à 100 %.

## Exigences
`EX-NFR-023` (vérification Release en CI) ; réutilise `EX-NFR-013` (compilation sans
avertissement), `EX-NFR-022` (CI verte pour merger), `EX-NFR-030` (build via CMake),
`EX-BUILD-010` (provisionnement de Qt en CI).
