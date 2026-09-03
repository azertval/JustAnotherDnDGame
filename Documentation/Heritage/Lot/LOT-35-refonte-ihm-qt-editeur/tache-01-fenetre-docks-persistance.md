# TACHE-01 — `QMainWindow` à docks + persistance `QSettings` de la disposition {#lot-35-tache-01-fenetre-docks-persistance}

**Lot :** [LOT-35](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
Le [LOT-34](@ref lot-34) fournit une `MainWindow` (`QMainWindow`) dont le widget central est le
viewport D3D11. Cette tâche transforme cette fenêtre en **poste de travail d'éditeur à panneaux
dockables** : cadre `QDockWidget` autour du viewport, et **persistance hors code** de la disposition
(le demandeur veut des « plusieurs fenêtres réglables, hors code de préférence »). Les panneaux
concrets (palette, outils, etc.) sont remplis par les tâches suivantes ; ici on pose le **cadre de
docking** et sa sauvegarde/restauration.

## Travail à réaliser
- **Zones de docking** : activer `setDockNestingEnabled(true)`, autoriser les quatre zones ; le
  viewport (`createWindowContainer`) reste le **widget central** non-dockable.
- **Panneaux vides nommés** (remplis plus tard) : créer les `QDockWidget` « Palette », « Outils »,
  « Statut » avec des `objectName` **stables** (indispensable à `saveState`/`restoreState`).
- **Persistance** via `QSettings` (portée application, organisation/appli nommées une fois) :
  - À la fermeture : `settings.setValue("mainWindow/geometry", saveGeometry())` +
    `"mainWindow/state", saveState()`.
  - Au démarrage : `restoreGeometry`/`restoreState` s'ils existent, sinon **disposition par défaut**
    codée (positions initiales raisonnables des docks).
  - Action « Réinitialiser la disposition » (menu Affichage) : efface l'état sauvegardé et réapplique
    le défaut.
- **Menu Affichage** listant les docks (toggler leur visibilité — `QDockWidget::toggleViewAction`).

## Fichiers impactés
- `Source/Editor/MainWindow.{h,cpp}` (docks, menu Affichage, persistance).
- Éventuel `Source/Editor/EditorSettings.{h,cpp}` (encapsulation `QSettings` : clés centralisées).

## Tests (obligatoires)
- **Logique de persistance testable** : si l'accès `QSettings` est encapsulé, tester la
  sérialisation/désérialisation des clés (présence/valeurs par défaut) sans fenêtre.
- **Vérification manuelle** : déplacer/détacher/masquer des docks, fermer, rouvrir → disposition
  restaurée ; « Réinitialiser » revient au défaut.

## Points d'attention
- **`objectName` obligatoires et stables** sur chaque `QDockWidget` et sur la `QMainWindow`, sinon
  `restoreState` échoue silencieusement.
- **Version de layout** : préfixer l'état d'un numéro de version (`saveState(int version)`) pour
  invalider proprement une disposition sauvegardée devenue incompatible après ajout/retrait de docks.
- **Emplacement de `QSettings`** : documenter où il est stocké (registre Windows par défaut) ; ne pas
  y mettre de données de niveau (elles restent dans les fichiers `Levels`).

## Définition de fait (DoD)
- Fenêtre d'éditeur à docks fonctionnels autour du viewport, disposition sauvegardée/restaurée et
  réinitialisable ; `/W4 /WX` propre ; vérification manuelle OK.

## Exigences
`EX-IHM-010` (éditeur à docks Qt), `EX-IHM-011` (disposition persistée hors code).
