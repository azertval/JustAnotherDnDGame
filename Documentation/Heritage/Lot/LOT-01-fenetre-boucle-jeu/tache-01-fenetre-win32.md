# TACHE-01 — Fenêtre Win32 & pompe de messages {#lot-01-tache-01-fenetre-win32}

**Lot :** [LOT-01](epic.md) · **Module :** `Source/HMI` · **Statut :** terminé

## Contexte
Le jeu a besoin d'une fenêtre native Windows comme surface de rendu et source d'événements. Cette tâche pose la fenêtre et sa boucle de messages, sans encore de rendu.

## Travail à réaliser
- Créer une classe `Window` (RAII) encapsulant la fenêtre Win32 :
  - Enregistrement de la classe de fenêtre, création via `CreateWindowEx`, titre paramétrable, **redimensionnable**, icône par défaut.
  - Destruction propre de la fenêtre dans le destructeur.
- Implémenter la **pompe de messages** (`PeekMessage`/`Translate`/`Dispatch`) exposée de façon non bloquante pour la boucle de jeu.
- Exposer les événements utiles à la boucle : **demande de fermeture**, **redimensionnement** (nouvelle taille du client), sans traiter encore les entrées de gameplay.
- Fournir un accès au **handle** de fenêtre (`HWND`) pour l'initialisation Direct3D (TACHE-02).

## Fichiers impactés
- `Source/HMI/Window.h`, `Source/HMI/Window.cpp` (nouveau).
- `Source/HMI/CMakeLists.txt` (ajout des sources ; liaison `user32`/`gdi32` si nécessaire).

## Points d'attention
- Respect des conventions : membres `_camelCase`, RAII (pas d'`init()`/`cleanup()` manuels), documentation `.h` + `.cpp`.
- La fenêtre ne doit **pas** dépendre de `Core` (sens des dépendances : `HMI → Core`).
- Isoler la `WndProc` (statique) de l'état d'instance proprement (pointeur utilisateur via `GWLP_USERDATA`).

## Définition de fait (DoD)
- La fenêtre s'ouvre, se déplace, se redimensionne et se ferme (croix) sans erreur.
- Aucune fuite de handle à la fermeture.
- Compile `/W4 /WX`, formaté clang-format, API documentée en Doxygen.

## Exigences
`EX-REN-003`, `EX-REN-001`.
