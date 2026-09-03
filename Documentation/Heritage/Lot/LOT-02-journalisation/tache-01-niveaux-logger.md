# TACHE-01 — Niveaux de log & interface Logger {#lot-02-tache-01-niveaux-logger}

**Lot :** [LOT-02](epic.md) · **Emplacement :** `Source/Core/Diagnostics` · **Statut :** fait

## Contexte
Base du système de journalisation : définir les niveaux et un `Logger` qui filtre puis diffuse les messages vers des sinks (TACHE-02).

## Travail à réaliser
- `LogLevel` (`enum class`) : `Trace`, `Info`, `Warning`, `Error` (ordre croissant de gravité).
- `Logger` :
  - Niveau minimal configurable (les messages en dessous sont ignorés).
  - `log(LogLevel, message)` qui, si le niveau passe le filtre, transmet aux sinks enregistrés.
  - Enregistrement d'un ou plusieurs sinks (interface définie en TACHE-02 ; ici, dépendance via une interface `ILogSink`).
- Le `Logger` **possède** ses sinks (RAII) et n'utilise aucun état global caché (une instance explicite ; un accesspoint global optionnel viendra à l'intégration).

## Fichiers impactés
- `Source/Core/Diagnostics/LogLevel.h` (nouveau).
- `Source/Core/Diagnostics/Logger.h`, `Logger.cpp` (nouveau).
- `Source/Core/CMakeLists.txt`.
- `Source/Test/Unit/test_logger.cpp` (nouveau).

## Tests (obligatoires)
- Un message de niveau ≥ niveau minimal est transmis aux sinks.
- Un message de niveau < niveau minimal est filtré (aucun sink appelé).
- Plusieurs sinks reçoivent le même message.

## Points d'attention
- Conventions : `enum class`, `_camelCase`, `[[nodiscard]]` sur les accès purs, documentation `.h` + `.cpp`.
- Pas de dépendance système dans `Logger` (les I/O concrètes sont dans les sinks).

## Définition de fait (DoD)
- Filtrage et diffusion corrects, testés (`ctest` vert).
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
Conventions §10 ; `EX-NFR-010`, `EX-NFR-020`.
