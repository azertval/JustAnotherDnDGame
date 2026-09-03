# TACHE-03 — Macros de log (fichier/ligne, horodatage) {#lot-02-tache-03-macros-log}

**Lot :** [LOT-02](epic.md) · **Emplacement :** `Source/Core/Diagnostics` · **Statut :** fait

## Contexte
Confort d'utilisation : des macros courtes pour journaliser, capturant automatiquement le contexte (fichier, ligne) et l'horodatage.

## Travail à réaliser
- Macros `JADG_LOG_TRACE/INFO/WARNING/ERROR(message)` qui appellent le logger courant avec le niveau correspondant.
- Enrichissement du message : **horodatage**, **niveau**, et **fichier:ligne** (`__FILE__`, `__LINE__`).
- Format de ligne lisible et stable (ex. `[HH:MM:SS][INFO][fichier:ligne] message`).

## Fichiers impactés
- `Source/Core/Diagnostics/Log.h` (macros ; nouveau).
- `Source/Core/Diagnostics/Logger.*` (formatage de ligne si nécessaire).
- `Source/Test/Unit/test_log_format.cpp` (nouveau).

## Tests (obligatoires)
- Le message formaté contient le niveau et le texte fournis.
- Le formatage inclut la position source (fichier/ligne) transmise par la macro.
- La partie variable (horodatage) est isolée pour permettre un test déterministe du reste.

## Points d'attention
- Les macros doivent s'évaluer proprement (une seule évaluation des arguments, protection par `do { } while(0)` si besoin).
- Conventions : macros en `UPPER_SNAKE_CASE`, documentation `.h`.

## Définition de fait (DoD)
- Macros fonctionnelles, formatage testé (`ctest` vert).
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
Conventions §10 ; `EX-NFR-020`.
