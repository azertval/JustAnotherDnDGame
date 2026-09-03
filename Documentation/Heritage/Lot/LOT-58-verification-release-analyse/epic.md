# LOT-58 — Vérification Release, sanitizer et analyse statique {#lot-58}

> Statut : **en cours**. Prérequis : aucun. À exécuter **tôt** dans le programme `0.1.0` : c'est lui
> qui protège les lots de contenu qui suivent. TACHE-01 à TACHE-05 faites et vérifiées localement
> (couverture mesurée à 93.66 %, seuil posé à 85 %) ; seule TACHE-06 garde une réserve — la
> vérification CI réelle (`gh pr checks`) reste à faire au premier passage en ligne, faute d'avoir
> ouvert de PR pour ce lot (choix explicite : vérification locale uniquement).

## Objectif
Faire tenir à la CI les promesses que le dépôt écrit déjà.

Trois vérifications sont **déclarées** dans les spécifications et les conventions, et **exécutées
nulle part** :

1. **La configuration Release n'est jamais construite ni testée avant le tag.** Les deux jobs de
   build de `ci.yml` sont en Debug (`--preset vs` → configuration `Debug`, preset `ninja` →
   `CMAKE_BUILD_TYPE=Debug`). Le premier build Release d'un cycle a lieu dans `release.yml`, donc
   **après** que le tag est poussé — et un tag est public. Ce n'est pas une inquiétude théorique :
   `90f85254 fix(core): correction d'une variable non utilisee en Release` a déjà corrigé une casse
   que seule la configuration Release révélait.
2. **AddressSanitizer n'a pas tourné depuis le `LOT-01`.** L'option `ENABLE_ASAN` existe
   (`CMakeLists.txt`) et n'est câblée dans aucun workflow. `EX-NFR-003` — « aucune fuite ; vérifiable
   via AddressSanitizer » — est une exigence **orpheline** : déclarée, référencée par zéro lot.
   Cinquante-cinq lots de code, dont tout le pipeline Direct3D 11 et les caches de textures, n'ont
   jamais été passés au sanitizer.
3. **`clang-tidy` et `clang-format` ne sont exécutés nulle part.** `conventions.md` les présente
   comme de l'« outillage déjà en place » et `CONTRIBUTING.md` exige un code formaté avant tout
   commit ; rien ne le vérifie. Le fichier `.clang-tidy` est complet et n'a jamais rien dit.

Une couverture produite puis oubliée complète le tableau : le rapport est uploadé sans seuil, et ne
mesure que `UnitTests.exe` — les 89 tests d'intégration et les 3 tests système, c'est-à-dire toute
la physique de bout en bout, en sont absents.

## Périmètre

### Inclus
- Job CI **Release** : configuration, build et `ctest` en Release, contrôle requis pour merger.
- Job CI **AddressSanitizer** sur les trois exécutables de test.
- **`clang-tidy`** câblé et exécuté, avec une phase de triage assumée sur l'existant.
- **`clang-format`** vérifié automatiquement (contrôle, pas réécriture).
- **Couverture** étendue à `IntegrationTests` et `SystemTests`, avec un **seuil** qui fait échouer
  la CI.

### Exclus (hors périmètre de ce lot)
- Correction de fond des avertissements que clang-tidy révélera au-delà du triage : ce lot **rend
  visible** et fixe ce qui est trivial ; une refonte guidée par l'analyse statique serait un lot en
  soi.
- Portage hors Windows, autres compilateurs, autres sanitizers (UBSan, TSan).
- Tests de performance chiffrés — c'est LOT-62 (budget de rendu mesuré, programme de cadrage
  `0.1.0`).
- Signature de code, empaquetage, installeur.

## Décisions de cadrage
- **Ce lot passe avant les lots de contenu.** Il ne produit rien de visible et c'est précisément
  pour cela qu'on serait tenté de le repousser après `LOT-59`, `LOT-60` et `LOT-53`. L'ordre
  inverse est le bon : le durcissement doit exister **avant** le code qu'il doit protéger, sinon
  chaque lot de contenu ajoute sa part de dette non vérifiée et le premier passage du sanitizer
  arrive sur un tas.
- **Un job Release séparé, pas un remplacement du job Debug.** Les deux configurations ont des
  bogues distincts : Debug a les assertions et la couche de débogage D3D, Release a les
  optimisations, `NDEBUG` et les variables devenues inutilisées. Il faut les deux.
- **clang-tidy en avertissement d'abord, bloquant ensuite.** Lancer 61 000 lignes contre
  `bugprone-*` + `cppcoreguidelines-*` + `readability-*` produira un mur. Le rendre bloquant
  immédiatement, c'est garantir qu'on le désactivera. La tâche dédiée trie, corrige le trivial, et
  n'active en bloquant que les familles déjà propres — le reste est consigné, pas caché.
- **La couverture obtient un seuil, pas un objectif ambitieux.** Le seuil est calé **sous** la
  valeur mesurée le jour de la mise en place : son rôle est d'interdire une chute, pas de forcer une
  hausse. Un seuil inatteignable se contourne.
- **ASan sur les tests, pas sur l'application.** Le sanitizer a besoin d'un scénario reproductible
  et sans GPU ; les trois exécutables de test le fournissent, l'application Qt/D3D11 non.

## Exigences couvertes
- Nouvelles : `EX-NFR-023` (la CI vérifie la configuration Release avant tout tag), `EX-NFR-024`
  (analyse statique et formatage vérifiés automatiquement).
- **Rattachée** : `EX-NFR-003` (empreinte mémoire stable, vérifiable via AddressSanitizer) — orpheline
  depuis sa déclaration, ce lot lui donne enfin une exécution.
- Réutilisées : `EX-NFR-013` (compilation sans avertissement), `EX-NFR-022` (CI : build, tests,
  couverture), `EX-NFR-012` (conventions), `EX-NFR-020` (tests unitaires), `EX-NFR-030`
  (construction via CMake), `EX-NFR-041` (RAII).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-job-release.md) | Job CI de vérification en configuration Release | `.github/workflows`, `CMakePresets.json` | ✅ |
| [TACHE-02](tache-02-job-asan.md) | Job CI AddressSanitizer sur les exécutables de test | `.github/workflows`, `CMakeLists.txt` | ✅ |
| [TACHE-03](tache-03-clang-tidy.md) | `clang-tidy` câblé, trié, puis bloquant sur les familles propres | `CMakeLists.txt`, `.clang-tidy`, `.github/workflows` | ✅ |
| [TACHE-04](tache-04-clang-format.md) | Vérification automatique du formatage | `.github/workflows` | ✅ |
| [TACHE-05](tache-05-couverture.md) | Couverture étendue aux tests d'intégration et système, avec seuil | `.github/workflows` | ✅ |
| [TACHE-06](tache-06-documentation-verification.md) | Documentation et vérification | `Documentation` | 🔄 |

## Critères d'acceptation du lot
1. Une PR introduisant une casse **Release-only** (variable inutilisée sous `NDEBUG`, assertion
   portant un effet de bord) est **refusée par la CI**, et non découverte après le tag.
2. Les trois exécutables de test passent sous AddressSanitizer sans rapport d'erreur ; `EX-NFR-003`
   n'est plus orpheline.
3. `clang-tidy` s'exécute sur chaque PR ; les familles déclarées bloquantes sont propres, le reste
   est consigné dans un état connu et daté.
4. Une PR dont le formatage s'écarte de `.clang-format` est refusée.
5. La couverture inclut `IntegrationTests` et `SystemTests`, et une chute sous le seuil fait échouer
   la CI.
6. La durée totale de la CI reste tenable pour un cycle de PR (à mesurer et consigner ; les jobs
   sont parallèles).

## Dépendances
Aucun prérequis. Protège tous les lots suivants du programme `0.1.0` — en particulier LOT-60
(audio, nouvelle dépendance tierce) et [LOT-53](@ref lot-53) (recyclage de particules, terrain
classique de dépassement de tampon).

## Navigation des tâches
- @subpage lot-58-tache-01-job-release
- @subpage lot-58-tache-02-job-asan
- @subpage lot-58-tache-03-clang-tidy
- @subpage lot-58-tache-04-clang-format
- @subpage lot-58-tache-05-couverture
- @subpage lot-58-tache-06-documentation-verification
