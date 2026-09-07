# Exigences non fonctionnelles {#spec-exigences}

> Statut : **livré** (`0.1.0`). Outillage qualité effectivement exécuté à chaque PR depuis `LOT-H-58`
> (CI Debug **et** Release, clang-tidy, clang-format, ASan, couverture agrégée, contrôles de
> reproductibilité du `LOT-H-66`). Transverse à toutes les specs.

## 1. Performance
- \anchor EX-NFR-001 **EX-NFR-001** — Le jeu doit maintenir **60 images/seconde** sur une configuration de bureau récente pour les niveaux du MVP. Rendue **observable** par le compteur de diagnostic (`F9`, `hmi::DiagnosticsHud`, `LOT-H-62`) : la cadence dépend de la machine, elle **reste hors de portée d'un contrôle automatique** (une machine virtuelle partagée ne la mesure pas de façon reproductible) — le compteur l'affiche pour être constatée sur sa propre machine de développement, il ne l'assert jamais en CI.
- \anchor EX-NFR-002 **EX-NFR-002** — La simulation doit fonctionner à **pas de temps fixe** et rester déterministe (mêmes entrées → même résultat).
- \anchor EX-NFR-003 **EX-NFR-003** — L'empreinte mémoire doit rester stable dans le temps (aucune fuite ; vérifiable via AddressSanitizer). Vérifiée par le job `sanitize` de `ci.yml` (LOT-H-58) : les trois exécutables de test (`UnitTests`, `IntegrationTests`, `SystemTests`) s'exécutent sous AddressSanitizer à chaque PR.
- \anchor EX-NFR-043 **EX-NFR-043** — Le coût d'un niveau doit être borné et vérifié sur un
  **second axe** : la **mémoire de texture**, distincte du nombre de primitives (`EX-NFR-005`). Les
  plans picturaux (`EX-DEC-040`) en sont le motif : ils n'ajoutent presque **aucune** primitive — un
  quad chacun — mais occupent une texture à l'échelle du niveau, de sorte qu'un plafond exprimé en
  primitives laisserait passer exactement la régression qu'ils rendent possible. Le plafond est
  **testé par niveau livré**, au même endroit et selon le même patron que celui des primitives, et
  le coût est **affiché à l'auteur** pendant qu'il le dépense (`EX-EDIT-047`) plutôt que découvert
  en CI. Concrétisé en `LOT-H-69`.
- \anchor EX-NFR-005 **EX-NFR-005** — Le rendu ne doit soumettre que les primitives **effectivement
  visibles** : le contenu hors du cadrage de la caméra (`EX-REN-015`, salle courante) est écarté
  avant soumission. Le nombre de primitives émises par image doit rester **borné et observable**,
  l'habillage complet (fond, décors, ombres, tuiles, objets, effets) multipliant le volume par
  rapport au rendu d'origine. Concrétisé en `LOT-H-40`. **Vérifiée** par le test de non-régression du
  volume de primitives (`Source/Test/Unit/HMI/Graphics/test_render_budget.cpp`, `LOT-H-62`) : chaque
  niveau livré reste sous un plafond nommé, dans les deux modes de rendu, et le culling écarte une
  fraction assertée des primitives sur un grand niveau — déterministe, sans GPU (`EX-NFR-004`).
  Rendue **observable** en jeu par le même compteur de diagnostic que `EX-NFR-001` (`F9`).

## 2. Architecture & maintenabilité
- \anchor EX-NFR-010 **EX-NFR-010** — La logique (`Core`) doit être **indépendante** de la présentation (`HMI`) et testable sans fenêtre ni GPU.
- \anchor EX-NFR-011 **EX-NFR-011** — Aucune dépendance cyclique entre modules (`HMI → Core`, jamais l'inverse).
- \anchor EX-NFR-012 **EX-NFR-012** — Le code doit respecter le [guide de conventions](conventions.md) (nommage, RAII, documentation, gestion d'erreurs).
- \anchor EX-NFR-013 **EX-NFR-013** — Le code livré doit compiler **sans avertissement** (`/W4 /WX`).

## 3. Qualité & vérification
- \anchor EX-NFR-020 **EX-NFR-020** — Toute logique de gameplay livrée dans `Core` doit être couverte par des **tests unitaires** (GoogleTest).
- \anchor EX-NFR-022 **EX-NFR-022** — La **CI** doit exécuter build, tests et couverture à chaque push/PR et rester verte pour merger. La couverture agrège `UnitTests`, `IntegrationTests` et `SystemTests` (job `build-test-coverage` de `ci.yml`, LOT-H-58) ; une chute sous le seuil consigné (`COVERAGE_THRESHOLD_PERCENT`) fait échouer la CI.
- \anchor EX-NFR-023 **EX-NFR-023** — La configuration **Release** doit être construite et testée en
  CI, sur **chaque PR** — avant tout tag, jamais découverte après. Vérifiée par le job
  `build-test-release` de `ci.yml` (LOT-H-58), contrôle requis pour merger au même titre que
  `build-test-coverage`.
- \anchor EX-NFR-024 **EX-NFR-024** — L'analyse statique et le formatage doivent être **vérifiés
  automatiquement**, pas seulement configurés. Vérifiés par les jobs `clang-tidy` (`bugprone-*`
  bloquant, le reste consigné) et `format` (`clang-format --dry-run --Werror`, version épinglée) de
  `ci.yml` (LOT-H-58).
- \anchor EX-NFR-004 **EX-NFR-004** — La chaîne de rendu doit être **vérifiable sans GPU** : les
  primitives de dessin produites pour une scène donnée doivent pouvoir être **capturées et
  inspectées** par un test (ordre des calques, priorité de résolution des textures, choix des
  raccords automatiques, isolement d'un calque, effets de bord du culling). Un critère d'acceptation
  du type « rendu identique » ou « ordre de calque correct » ne doit pas reposer sur une
  vérification à l'œil quand il peut être asserté. Concrétisé en `LOT-H-40`.

## 4. Portabilité & reproductibilité
- \anchor EX-NFR-030 **EX-NFR-030** — Le projet doit se construire **exclusivement via CMake**, reproductible sur plusieurs postes (cf. `README`).
- \anchor EX-NFR-031 **EX-NFR-031** — Les dépendances tierces doivent être **épinglées** à une version (ex. GoogleTest).
- \anchor EX-NFR-032 **EX-NFR-032** — Cible actuelle : **Windows/DirectX**. Le `Core` doit toutefois éviter toute dépendance système inutile pour préserver sa testabilité (et une portabilité future).

## 5. Robustesse
- \anchor EX-NFR-040 **EX-NFR-040** — Une erreur récupérable (fichier de niveau invalide, ressource manquante) ne doit pas faire planter le jeu : elle est signalée et gérée (cf. politique d'erreurs des conventions).
- \anchor EX-NFR-041 **EX-NFR-041** — Les ressources (mémoire, handles DirectX) doivent être gérées en **RAII** (libération garantie).
- \anchor EX-NFR-042 **EX-NFR-042** — Une **version publiée** doit produire une **trace exploitable**
  d'exécution : journal écrit dans un fichier à côté de l'exécutable, contenant au minimum la
  version, la configuration de build et le contexte matériel (adaptateur graphique, manette). Le
  volume est **borné** (taille maximale et rotation), la trace reste **locale** — aucun envoi réseau,
  aucune donnée personnelle — et un dossier inaccessible dégrade la journalisation, jamais le jeu
  (`EX-NFR-040`). Sans cela, un défaut signalé par un joueur n'est accompagné d'aucun élément.
  Prévu en `LOT-H-61`.

## 6. Build & dépendances
- \anchor EX-BUILD-010 **EX-BUILD-010** — Une dépendance tierce **non gérable par `FetchContent`**
  (SDK volumineux tel que **Qt**) doit être **provisionnée et documentée de façon reproductible** sur
  les trois environnements : poste local (installeur officiel ou `aqtinstall`), **CI** (étape
  d'installation dans le workflow, sur le runner épinglé) et **release** (déploiement des bibliothèques
  dynamiques requises à côté de l'exécutable, ex. `windeployqt`). La version est **épinglée**
  (`EX-NFR-031`) et la licence documentée. Introduit en `LOT-H-34`. Le poste local déclare une
  version minimale (`QT_VERSION_MINIMUM`, `Source/HMI/CMakeLists.txt`) alignée sur celle de la CI
  et vérifiée automatiquement contre elle (`scripts/check_qt_version_pin.py`) ; un écart local
  produit un avertissement explicite plutôt qu'une divergence silencieuse (`LOT-H-66`). L'exigence de
  reproductibilité porte aussi sur l'**outil de provisionnement lui-même** : lorsqu'il doit être
  pris ailleurs que sur son dépôt de paquets habituel — cas d'`aqtinstall` en `LOT-H-69`, dont la
  version publiée ne sait pas installer Qt ≥ 6.11 — la source est épinglée à une **révision
  précise** (jamais une branche mobile), et le motif du détour ainsi que sa condition de sortie
  sont écrits à l'endroit où il est déclaré.

## Exigences retirées {#nfr-retirees}

> Retirée par le `LOT-67`, qui retire du programme la notion de **niveau discret**. L'ancre est
> **conservée** — jamais renumérotée, jamais supprimée : les lots hérités s'y réfèrent, et réécrire
> un lot livré falsifierait son histoire (règle de [`lots.md`](@ref lots)).

- \anchor EX-NFR-021 **EX-NFR-021** *(retirée en `LOT-67`)* — Les niveaux du MVP doivent être
  couverts par un **test système** vérifiant leur franchissabilité. Motif : « franchir » un niveau
  suppose une sortie qui le termine (`EX-GP-030`, retirée). Ce qu'un bac à sable doit vérifier est
  autre chose — qu'aucune zone jouable n'est inatteignable — et cela se contrôle sur la **carte**,
  pas sur une trajectoire de bout en bout.

## Traçabilité
Ces exigences transverses conditionnent l'acceptation de chaque lot. Depuis le `LOT-H-58`, elles
s'appuient sur de l'outillage **effectivement exécuté** à chaque PR (CMake, CI Debug **et**
Release, clang-tidy, clang-format, ASan, couverture agrégée) et non plus seulement configuré :
voir le tableau « Outillage qualité » de [`conventions.md`](conventions.md) pour le job qui vérifie
chaque outil.

`EX-NFR-032` (cible Windows/DirectX) est, comme les exigences ci-dessus, un **invariant
transverse** : aucun lot n'a besoin de le citer pour le respecter, ce silence n'est pas une
exigence orpheline (`LOT-H-66`).
