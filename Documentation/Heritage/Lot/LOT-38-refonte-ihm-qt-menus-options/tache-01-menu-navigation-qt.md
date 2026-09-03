# TACHE-01 — Menu principal Qt + navigation (remplace `IScreen`/`ScreenManager`) {#lot-38-tache-01-menu-navigation-qt}

**Lot :** [LOT-38](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
La navigation historique repose sur `IScreen` / `ScreenManager` / `ScreenTransition` (un écran renvoie
une **intention** de bascule, appliquée par le manager via une factory ; `main.cpp`). Sous Qt, la
navigation menu ↔ jeu ↔ éditeur est portée par les **vues/fenêtres Qt**. Cette tâche crée le **menu
principal** et le **routage** entre modes, préparant le retrait de l'ancienne pile d'écrans
(TACHE-03).

## Travail à réaliser
- **Menu principal Qt** : écran d'accueil (widget dédié ou page d'un `QStackedWidget`) avec les
  entrées existantes : Jouer, Éditeur, Options, Touches (jeu/éditeur/manette), Enregistrer les logs,
  Quitter. Libellés localisés (`Localization`).
- **Routage / navigation** : un composant (`Source/Editor/AppNavigator` ou pilotage par `MainWindow`)
  gère la vue active (menu / jeu / éditeur), remplaçant `ScreenTransition`. Le viewport D3D11 est
  partagé/réutilisé selon le mode (jeu vs éditeur), ou instancié par mode — arbitrer.
- **Enregistrement des logs de session** : réutiliser `hmi::saveSessionLog` + `MemoryLogSink`
  (initialisé dans le `main` Qt, `LOT-34`), déclenché par l'entrée de menu.
- **Quitter** : fermeture propre (garde-fou `dirty` de l'éditeur, `LOT-36`, respecté).

## Fichiers impactés
- `Source/Editor/MainMenu.{h,cpp}`, `Source/Editor/AppNavigator.{h,cpp}` (ou navigation dans
  `MainWindow`).
- `Source/Editor/main_qt.cpp` (câblage logs de session / navigateur).

## Tests (obligatoires)
- **Logique de navigation testable** : la machine à états de navigation (mode courant, transitions
  autorisées, garde-fou avant de quitter l'éditeur modifié) est vérifiable sans Qt.
- **Vérification manuelle** : naviguer menu → jeu → menu → éditeur → menu ; enregistrer les logs ;
  quitter avec brouillon modifié (confirmation).

## Points d'attention
- **Ne pas recréer un `ScreenManager`** : profiter des mécanismes Qt (`QStackedWidget`, signaux) ; la
  logique de transition reste minimale et testable.
- **Partage du viewport** entre jeu et éditeur : décider (réutilisation vs instances distinctes) en
  tenant compte du coût de recréation du `GraphicsDevice`.
- **Coexistence** : l'exe legacy existe encore à cette tâche ; le retrait est TACHE-03.

## Définition de fait (DoD)
- Menu principal Qt et navigation entre modes opérationnels ; logs de session et quitter (avec
  garde-fou) fonctionnels ; navigation **testée** ; `/W4 /WX` propre ; vérification manuelle OK.

## Exigences
`EX-IHM-040` (menus en Qt) ; réutilise l'enregistrement des logs (`LOT-02`), `Localization` (`LOT-06`),
le garde-fou `dirty` (`LOT-36`).
