# TACHE-02 — Job CI AddressSanitizer {#lot-58-tache-02-job-asan}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `.github/workflows`, `CMakeLists.txt` ·
**Statut :** fait

## Contexte
L'option `ENABLE_ASAN` est en place depuis le `LOT-01` (`CMakeLists.txt`, cible interface
`project_options`). Le `LOT-01` demandait « une exécution ponctuelle avec `-DENABLE_ASAN=ON` sans
rapport d'erreur » — et c'est la dernière fois que le sujet apparaît dans le dépôt.

`EX-NFR-003` (« L'empreinte mémoire doit rester stable dans le temps — aucune fuite ; vérifiable via
AddressSanitizer ») est **orpheline** : déclarée dans les exigences non fonctionnelles, référencée
par aucun lot. Depuis, cinquante-cinq lots ont ajouté des caches de textures, un atlas procédural,
un historique d'annulation à régions, un pipeline de composition de quads et un décodage d'images —
tout ce qui manipule des tampons bruts. Rien n'a jamais été instrumenté.

## Travail à réaliser
- **Job `sanitize`** dans `ci.yml` : configuration `-DENABLE_ASAN=ON` (générateur Ninja, x64,
  environnement MSVC établi par `vcvars64.bat` comme le job `build-ninja` existant), build des
  trois cibles de test, puis exécution de `UnitTests`, `IntegrationTests` et `SystemTests`.
- **Ne pas construire l'application Qt** dans ce job : `-DBUILD_EDITOR_QT=OFF` évite d'instrumenter
  un binaire qu'on ne peut de toute façon pas exécuter sans affichage. Vérifier que les tests
  restent constructibles sans Qt — la garde conditionnelle existe déjà dans
  `Source/Test/CMakeLists.txt`.
- **Traiter les rapports trouvés.** Il est probable qu'il y en ait ; c'est le but. Chaque rapport
  est soit corrigé, soit — s'il vient d'une dépendance tierce ou de l'exécution — consigné
  explicitement avec sa raison.
- **Rattacher `EX-NFR-003`** : la référencer depuis cette tâche et l'epic suffit à la sortir de
  l'orphelinat, mais l'intention est qu'elle soit **exécutée**, pas seulement citée.

## Fichiers impactés
- `.github/workflows/ci.yml` — nouveau job.
- `CMakeLists.txt` — ajustements de l'option `ENABLE_ASAN` si l'édition de liens MSVC l'exige.
- `Documentation/Specification/exigences-non-fonctionnelles.md` — `EX-NFR-003` mentionne où elle est
  vérifiée.
- Sources corrigées selon les rapports.

## Tests (obligatoires)
- Les trois exécutables de test passent sous ASan, sans rapport d'erreur.
- **Test négatif** : une lecture volontairement hors bornes dans un test jetable est bien détectée
  et fait échouer le job — sans quoi rien ne prouve que l'instrumentation est active.
- Le job reste vert de manière reproductible sur trois exécutions consécutives (un rapport
  intermittent est un vrai défaut, pas un aléa à ignorer).

## Points d'attention
- **ASan sous MSVC ne se combine pas avec tout** : l'édition de liens incrémentale et certaines
  options de génération d'informations de débogage sont incompatibles. Le message d'erreur ne
  nomme pas toujours la cause ; s'attendre à un temps de mise au point sur la configuration.
- **`/fsanitize=address` a besoin de ses DLL runtime** dans le `PATH` à l'exécution sur les
  images GitHub — sinon l'exécutable ne démarre pas, avec une erreur qui ne parle pas d'ASan.
- **ASan détecte les débordements et les usages après libération, pas toutes les fuites.** Pour
  l'empreinte mémoire au sens de `EX-NFR-003`, viser d'abord la sûreté mémoire, et noter que la
  stabilité dans la durée reste à vérifier autrement (le budget de rendu mesuré du LOT-62 y
  contribue).
- Le temps d'exécution est multiplié par deux à trois : vérifier que la durée du job reste
  acceptable, quitte à n'y exécuter que les tests, jamais l'application.

## Définition de fait (DoD)
- Les trois exécutables de test s'exécutent sous AddressSanitizer en CI sans rapport d'erreur, la
  détection est démontrée active par un test négatif, les rapports trouvés sont corrigés ou
  consignés, `EX-NFR-003` n'est plus orpheline.

## Exigences
`EX-NFR-003` (empreinte mémoire, ASan) — rattachée par cette tâche ; réutilise `EX-NFR-041` (RAII),
`EX-NFR-022` (CI), `EX-NFR-010` (tests sans GPU), `EX-NFR-020` (tests unitaires).
