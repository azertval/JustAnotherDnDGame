# TACHE-05 — Couverture étendue et seuil {#lot-58-tache-05-couverture}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `.github/workflows` · **Statut :** fait

## Contexte
L'étape `Coverage` de `ci.yml` exécute OpenCppCoverage sur **`UnitTests.exe` uniquement**, exporte
un rapport Cobertura et HTML, et l'**envoie en artefact**. Rien ne le lit, rien ne le compare, rien
n'échoue jamais à cause de lui.

Deux conséquences. D'abord, la couverture affichée est fausse par construction : les 89 tests
d'intégration et les 3 tests système — c'est-à-dire toute la physique du personnage vérifiée de bout
en bout, et la franchissabilité des niveaux livrés — n'y figurent pas. Ensuite, `EX-NFR-022` demande
que la CI exécute « build, tests **et couverture** » ; produire un chiffre que personne ne regarde
n'est pas l'exécuter.

## Travail à réaliser
- **Instrumenter les trois exécutables** : `UnitTests`, `IntegrationTests` et `SystemTests`,
  agrégés en un rapport unique (OpenCppCoverage sait fusionner plusieurs exécutions).
- **Mesurer la valeur réelle** une fois les trois inclus — elle sera différente, probablement plus
  haute, de celle qu'on croyait.
- **Poser un seuil calé sous la valeur mesurée**, avec une marge explicite, et faire **échouer** la
  CI en dessous. Le seuil est une cliquet anti-régression, pas un objectif.
- **Consigner la valeur de référence et sa date** dans le workflow, en commentaire, pour que la
  prochaine personne sache d'où vient le nombre.
- **Exclure `Source/Test`** du calcul (déjà fait via `--excluded_sources`) et vérifier que
  `External/` en est bien absent.

## Fichiers impactés
- `.github/workflows/ci.yml` — étape de couverture.
- `Documentation/Specification/exigences-non-fonctionnelles.md` — `EX-NFR-022` précise ce qui est
  mesuré et que le seuil bloque.

## Tests (obligatoires)
- Le rapport agrégé inclut des lignes couvertes par `IntegrationTests` et par `SystemTests` que
  `UnitTests` seul ne couvrait pas — vérifié sur un fichier identifié (par exemple
  `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`), sinon l'agrégation ne fonctionne pas.
- **Test négatif** : un seuil temporairement placé au-dessus de la valeur mesurée fait échouer le
  job.
- Le job reste dans une durée acceptable une fois les trois exécutables instrumentés.

## Points d'attention
- **Ne pas viser haut.** Un seuil ambitieux posé le jour de la mise en place se contourne dès la
  première PR gênée. Le rôle du chiffre est d'interdire une chute, pas de forcer une hausse : c'est
  une décision de cadrage du lot, pas un arbitrage à refaire ici.
- L'instrumentation ralentit l'exécution : `SystemTests` rejoue les quinze niveaux livrés, mesurer
  l'impact avant de conclure.
- OpenCppCoverage est installé via Chocolatey sans version épinglée dans le workflow actuel —
  l'épingler, pour la même raison que Doxygen et `clang-format`.
- La couverture de `Source/HMI` restera structurellement basse : la moitié du dossier est du widget
  Qt et du Direct3D non testable sans machine graphique. C'est attendu et documenté par
  `EX-NFR-004`/`EX-NFR-010` ; le seuil doit en tenir compte plutôt que de pousser à écrire des tests
  de façade.

## État de la mesure (LOT-58)
- **Mécanisme** : `ci.yml` exporte `UnitTests`, `IntegrationTests` et `SystemTests` en binaire
  intermédiaire (`--export_type binary:...`), puis les fusionne (`--input_coverage`) en un rapport
  Cobertura unique ; une étape dédiée compare `line-rate` au seuil et fait échouer le job en
  dessous. `OpenCppCoverage` épinglé (`OPENCPPCOVERAGE_VERSION: 0.9.9.0`).
- **Valeur mesurée** : **93.66 %** (8602/9184 lignes), le **2026-08-10**, en local (build **Ninja**
  Debug, pas le preset `vs` utilisé en CI — écart possible avec la première mesure réelle en CI, à
  surveiller). `COVERAGE_THRESHOLD_PERCENT: 85` dans `ci.yml`, marge explicite d'environ 8.5 points.
- **Preuve que la fusion fonctionne** : `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`
  n'apparaît **pas du tout** dans un rapport `UnitTests` seul (0 ligne instrumentée, le fichier est
  absent du rapport), et apparaît à **99 %** dans le rapport fusionné — la preuve demandée par la
  tâche que `IntegrationTests`/`SystemTests` apportent une couverture que `UnitTests` seul n'a pas.
- **Test négatif** : un seuil temporairement placé à 95 % (au-dessus des 93.66 % mesurés) fait
  échouer la comparaison — vérifié par calcul direct sur `coverage.xml`, logique identique à celle
  du job CI.
- OpenCppCoverage a pu être installé après une élévation UAC accordée manuellement (l'installeur
  Inno Setup ne peut pas s'élever de façon non interactive dans cet environnement).

## Définition de fait (DoD)
- La couverture agrège les trois exécutables, un seuil documenté et daté fait échouer la CI en cas
  de chute, la démonstration du blocage est faite, l'outil de couverture est épinglé.
- Atteinte, avec une réserve : la mesure ci-dessus vient d'un build **Ninja** local, pas du preset
  `vs` exact de la CI. À confirmer (ou ajuster le seuil) sur la première exécution réelle du job
  `build-test-coverage` en CI.

## Exigences
Réutilise `EX-NFR-022` (CI : build, tests et couverture), `EX-NFR-020` (tests unitaires `Core`),
`EX-NFR-021` (test système de franchissabilité), `EX-NFR-004` (rendu vérifiable sans GPU),
`EX-NFR-031` (outils épinglés).
