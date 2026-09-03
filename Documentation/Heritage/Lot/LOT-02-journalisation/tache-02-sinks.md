# TACHE-02 — Sinks enfichables {#lot-02-tache-02-sinks}

**Lot :** [LOT-02](epic.md) · **Emplacement :** `Source/Core/Diagnostics` · **Statut :** fait

## Contexte
Un sink est une **destination** de log. On découple la production des messages (`Logger`) de leur restitution, pour tester facilement et varier les sorties.

## Travail à réaliser
- Interface `ILogSink` : `write(LogLevel, message)` (+ destructeur virtuel).
- `ConsoleLogSink` : écrit sur la sortie standard **et** vers le débogueur (`OutputDebugString` sous Windows). Ce sink relève de `Core` mais son implémentation Windows reste isolée derrière l'interface.
- `MemoryLogSink` (pour les tests) : conserve les messages reçus (niveau + texte) et permet de les inspecter.

## Fichiers impactés
- `Source/Core/Diagnostics/ILogSink.h` (nouveau).
- `Source/Core/Diagnostics/ConsoleLogSink.h`, `ConsoleLogSink.cpp` (nouveau).
- `Source/Core/Diagnostics/MemoryLogSink.h` (nouveau, pour les tests).
- `Source/Test/Unit/test_sinks.cpp` (nouveau).

## Tests (obligatoires)
- `MemoryLogSink` conserve fidèlement niveau et texte des messages reçus, dans l'ordre.
- Un `Logger` équipé d'un `MemoryLogSink` restitue bien ce qui a passé le filtre (lien avec TACHE-01).

## Points d'attention
- `ILogSink` : interface pure, destructeur virtuel, `override` sur les implémentations.
- Isoler l'appel Windows (`OutputDebugStringW`) dans `ConsoleLogSink.cpp` uniquement.
- Conventions : documentation `.h`, RAII.

## Définition de fait (DoD)
- Sinks fonctionnels et testés via le sink mémoire (`ctest` vert).
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
Conventions §10 ; `EX-NFR-010`, `EX-NFR-020`.
