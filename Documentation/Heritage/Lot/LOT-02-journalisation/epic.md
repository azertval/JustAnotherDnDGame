# LOT-02 — Journalisation & diagnostics {#lot-02}

> Statut : **terminé**. Infrastructure **transverse** : utilisée par tous les modules (y compris l'ECS du LOT-03).

## Objectif
Fournir, dans `Core/Diagnostics`, un **système de journalisation** (log par niveaux, vers des sorties configurables) et un **mécanisme d'assertions** (`JADG_ASSERT`), utilisables partout dans le code et testables. Remplace les `std::cout` / `std::fprintf` provisoires.

À l'issue du lot : n'importe quel module peut journaliser (`trace`/`info`/`warning`/`error`) et poser des préconditions vérifiées en Debug, conformément au guide de conventions §10.

## Périmètre

### Inclus
- Niveaux de log et interface `Logger` (filtrage par niveau minimal).
- **Sinks** (destinations) enfichables : console / débogueur Windows, plus un sink mémoire pour les tests.
- **Macros de log** avec fichier/ligne et horodatage.
- **Assertions** `JADG_ASSERT(condition, message)` : actives en Debug, retirées en Release, avec **handler surchargeable** (testable sans abandon du processus).
- Intégration dans `main` (remplacement du `std::fprintf` de gestion d'erreurs).

### Exclus (plus tard)
- Sink fichier avec rotation, journalisation asynchrone/multi-thread (mono-thread au MVP).
- Catégories/canaux de log avancés, filtrage par module.

## Exigences couvertes
- Guide de conventions **§10** (assertions `JADG_ASSERT`, niveaux de log, interdiction de `std::cout` en production).
- `EX-NFR-010` — `Core` testable sans fenêtre ni GPU (le logger vit dans `Core`).
- `EX-NFR-020` — comportement couvert par des tests unitaires.
- Appuie `EX-NFR-040` (signaler les erreurs récupérables) et la politique d'erreurs des conventions (§9).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-niveaux-logger.md) | Niveaux de log & interface `Logger` | `Core/Diagnostics` | ✅ Fait |
| [TACHE-02](tache-02-sinks.md) | Sinks enfichables (console/débogueur + mémoire de test) | `Core/Diagnostics` | ✅ Fait |
| [TACHE-03](tache-03-macros-log.md) | Macros de log (fichier/ligne, horodatage) | `Core/Diagnostics` | ✅ Fait |
| [TACHE-04](tache-04-assertions.md) | Assertions `JADG_ASSERT` (handler surchargeable) | `Core/Diagnostics` | ✅ Fait |
| [TACHE-05](tache-05-integration.md) | Intégration dans `main` & documentation | `HMI` | ✅ Fait |

## Critères d'acceptation du lot
1. Un message journalisé au-dessus du niveau minimal atteint les sinks configurés ; en dessous, il est filtré.
2. Un sink mémoire permet de vérifier en test le contenu et le niveau des messages émis.
3. `JADG_ASSERT` déclenche le handler en Debug sur condition fausse, et **ne compile aucun code** en Release.
4. Le handler d'assertion est surchargeable, permettant de tester l'échec sans interrompre les tests.
5. `main` utilise le logger (plus de `std::fprintf` direct).
6. Tests unitaires verts (`ctest`), build `/W4 /WX` sans avertissement, **CI verte** sur la PR, API documentée en Doxygen, `CHANGELOG.md` mis à jour.

## Dépendances
- Aucune dépendance de code. Ce lot **précède** LOT-03 (ECS), qui l'utilisera pour ses assertions et traces.

## Navigation des tâches
- @subpage lot-02-tache-01-niveaux-logger
- @subpage lot-02-tache-02-sinks
- @subpage lot-02-tache-03-macros-log
- @subpage lot-02-tache-04-assertions
- @subpage lot-02-tache-05-integration
