# TACHE-05 — Intégration dans main & documentation {#lot-02-tache-05-integration}

**Lot :** [LOT-02](epic.md) · **Emplacement :** `Source/HMI` · **Statut :** fait

## Contexte
Dernière tâche : rendre le logger réellement utilisé par l'application et documenter son usage pour les prochains lots.

## Travail à réaliser
- Fournir un **point d'accès** au logger de l'application (instance configurée au démarrage avec un `ConsoleLogSink`).
- Remplacer le `std::fprintf(stderr, ...)` de gestion d'erreurs de `main` par un log de niveau `Error`.
- Journaliser les grandes étapes du démarrage (création fenêtre, initialisation Direct3D) en `Info`.
- Documenter dans `Source/Core/Diagnostics/README.md` l'usage des macros de log et d'assertion (exemples).

## Fichiers impactés
- `Source/HMI/main.cpp`.
- `Source/Core/Diagnostics/README.md` (mise à jour avec exemples).
- `CHANGELOG.md` (section *Non publié*).

## Vérification
- **Manuelle** : au lancement, les messages `Info` de démarrage apparaissent ; en cas d'échec d'initialisation, un message `Error` clair est journalisé.
- **Automatique** : `ctest` vert, build `/W4 /WX` sans avertissement, **CI verte** sur la PR.

## Définition de fait (DoD)
- `main` n'utilise plus d'I/O directe pour les erreurs (passe par le logger).
- Tous les critères d'acceptation de l'[epic](epic.md) satisfaits.
- `CHANGELOG.md` mis à jour ; documentation Doxygen générable sans erreur.

## Exigences
Conventions §10 ; `EX-NFR-040`, `EX-NFR-013`.
