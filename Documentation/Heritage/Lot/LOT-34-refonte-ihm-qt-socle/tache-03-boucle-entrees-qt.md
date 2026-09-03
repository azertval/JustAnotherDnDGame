# TACHE-03 — Boucle de rendu Qt (pas fixe, interpolation) + entrées Qt → `InputState` {#lot-34-tache-03-boucle-entrees-qt}

**Lot :** [LOT-34](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
La boucle historique (`main.cpp:260-308`) est un `while` manuel : `pumpMessages` → `consumeResize` →
accumulateur `FixedTimestep::advance` → N pas de simulation (avec `beginInputFrame` **par pas
consommé**, discipline nerveuse du `LOT-33`) → un rendu avec `interpolationAlpha`. Sous Qt, c'est
`QApplication::exec()` qui possède la boucle d'événements : cette tâche **reproduit exactement** la
même discipline, pilotée par un tick Qt, et **traduit les événements d'entrée Qt** vers le
`hmi::InputState` existant (que la simulation lit déjà). Le **déterminisme** (`EX-NFR-002`) et la
**latence d'entrée** (`EX-CTRL-020/021`) doivent être strictement préservés.

## Travail à réaliser

### Boucle de rendu pilotée Qt
- Piloter le rendu par **`QWindow::requestUpdate()`** (recalé sur la V-Sync/compositeur) traité dans
  `event(QEvent::UpdateRequest)` — préférable à un `QTimer` fixe pour la fluidité ; `QTimer` haute
  fréquence en repli.
- À chaque tick : mesurer le temps réel écoulé (`std::chrono::steady_clock`, comme l'existant),
  `steps = timestep.advance(elapsed)`, exécuter les `steps` pas de **simulation/mise à jour**, appeler
  `beginInputFrame()` **après chaque pas consommé**, puis **rendre une fois** avec
  `timestep.interpolationAlpha()`. Re-`requestUpdate()` pour la frame suivante.
- Réutiliser tel quel `core::FixedTimestep` (aucune modification) et le `RenderContext`
  (`interpolationAlpha`).

### Entrées Qt → `hmi::InputState`
- Installer les gestionnaires sur `GameViewport` : `keyPressEvent`/`keyReleaseEvent`,
  `mousePressEvent`/`Release`/`Move`, `wheelEvent`, en écrivant dans l'**état courant** de
  `InputState` (pas d'avance des fronts ici — c'est `beginInputFrame` qui l'assure, `LOT-33`).
- **Table de correspondance `Qt::Key` → `hmi::Key`** (et boutons souris) : fonction pure, testable
  sans Qt fenêtré, couvrant les touches utilisées (déplacement, saut, dash, `Ctrl`/`Maj`/flèches/`Tab`
  pour l'éditeur à venir, molette pour zoom).
- **Perte de focus** : `focusOutEvent` (ou `QEvent::WindowDeactivate`) → `InputState::releaseAll`
  (remise à zéro courant **et** précédent, sans front parasite — `LOT-33`).
- **Répétition clavier** : ignorer les événements `autoRepeat` de Qt (un maintien ne doit pas générer
  de fronts répétés).

### Manette (XInput) conservée
- Sonder XInput **dans le tick** (avant les pas), en réutilisant la logique et le **throttling
  manette déconnectée** du `LOT-33` (extraire de `hmi::Window::pollGamepad` vers un utilitaire
  partagé si nécessaire, pour ne pas dépendre de `hmi::Window` qui reste au legacy).

## Fichiers impactés
- `Source/Editor/GameViewport.{h,cpp}` (tick + handlers d'entrée), éventuel
  `Source/Editor/QtInputMapper.{h,cpp}` (table `Qt::Key`→`hmi::Key`, pure).
- Éventuel `Source/HMI/Input/GamepadPolling.*` (extraction du sondage XInput partagé) — sans changer
  le comportement `LOT-33`.

## Tests (obligatoires)
- **`Qt::Key` → `hmi::Key`** : test unitaire exhaustif de la table (sans Qt fenêtré ; on peut tester
  la fonction de mapping avec des valeurs `Qt::Key`), y compris touches non mappées (ignorées).
- **Cadence déterministe** : la logique d'accumulateur restant `core::FixedTimestep` (déjà testée),
  vérifier qu'un scénario simulé (suite de deltas) produit le **même nombre de pas** qu'attendu — via
  les tests existants de `FixedTimestep`, non régressés.
- Intégration entrées→jeu (clavier/manette) et fluidité : **vérification manuelle** (dépendance GPU).

## Points d'attention
- **Ne pas avancer les fronts au rendu** : l'erreur corrigée au `LOT-33` ne doit pas réapparaître —
  `beginInputFrame` seulement **par pas consommé**, jamais par frame de rendu.
- **Un seul point d'échantillonnage par frame, en amont des pas** (`EX-CTRL-021`) : les handlers Qt
  remplissent l'état courant entre deux ticks ; le tick lit cet état.
- **`requestUpdate` vs `QTimer`** : `requestUpdate` peut se caler sur la V-Sync ; s'assurer que la
  boucle continue même sans événement (redemander l'update à chaque frame).
- **Coexistence** : cette boucle Qt vit dans `Source/Editor` ; l'ancienne boucle `main.cpp` reste
  intacte (legacy). Éviter toute dépendance de `Editor` sur `hmi::Window`.

## Définition de fait (DoD)
- La boucle Qt exécute la simulation à pas fixe avec interpolation ; clavier/souris/molette et manette
  alimentent `InputState` ; focus perdu = entrées relâchées. Déterminisme intact (tests verts),
  mapping d'entrées **testé**, `/W4 /WX` propre ; ressenti vérifié manuellement.

## Exigences
`EX-CTRL-020`/`EX-CTRL-021` (latence/échantillonnage entrées, à tout framerate), `EX-NFR-002`
(déterminisme), `EX-ARCH-031` (interpolation) ; réutilise la robustesse manette `EX-NFR-040`
(`LOT-33`).
