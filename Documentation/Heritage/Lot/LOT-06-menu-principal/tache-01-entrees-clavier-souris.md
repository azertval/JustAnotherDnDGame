# TACHE-01 — Entrées clavier & souris {#lot-06-tache-01-entrees-clavier-souris}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/Input`, `Source/HMI/Platform` · **Statut :** à faire

## Contexte
`Window` ne remonte aujourd'hui que la fermeture et le redimensionnement (et Échap ferme
l'app). Un menu — puis le gameplay — a besoin de lire le **clavier** et la **souris**, en
distinguant *pressée / maintenue / relâchée* (`EX-CTRL-011`) et en échantillonnant une fois
par frame, en amont de la logique (`EX-CTRL-021`).

## Travail à réaliser
- `InputState` (dans `HMI/Input`) : état par frame — touches enfoncées, **front montant**
  (« vient d'être pressée »), **front descendant** (« vient d'être relâchée ») ; position
  souris (en pixels client) et boutons (enfoncé / cliqué ce frame).
- Mécanique de frame : une méthode « nouvelle frame » recopie l'état courant vers l'état
  précédent ; `Window` met à jour l'état courant à partir des messages
  (`WM_KEYDOWN/UP`, `WM_MOUSEMOVE`, `WM_LBUTTONDOWN/UP`…) ; les fronts se déduisent de la
  comparaison courant/précédent.
- `Window` : cesser de fermer sur **Échap** (Échap devient une touche normale, utilisée pour
  revenir au menu) ; exposer l'`InputState` (`input()`) et une **demande de fermeture
  programmée** (`requestClose()`) pour l'action « Quitter ». La croix continue de fermer.

## Fichiers impactés
- `Source/HMI/Input/InputState.h`, `InputState.cpp` (nouveau).
- `Source/HMI/Platform/Window.h`, `Window.cpp` (capture des entrées, `requestClose`).
- `Source/HMI/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Front montant : une touche passée d'« absente » à « présente » est « vient d'être pressée »
  exactement une frame.
- Maintien : une touche restée enfoncée n'est plus « pressée » aux frames suivantes.
- Front descendant : relâchement détecté une frame.
- Boutons souris : même logique pressé/cliqué (l'`InputState` est testable sans fenêtre en
  injectant les événements).

## Points d'attention
- Séparer l'**état logique** (`InputState`, testable en isolation) de la **capture Win32**
  (`Window`), pour tester sans fenêtre.
- Latence : l'état est prêt avant la mise à jour de la logique (`EX-CTRL-021`).
- Ne pas introduire d'entrée de *gameplay* (mapping d'actions) — hors périmètre.

## Définition de fait (DoD)
- Entrées clavier/souris exposées avec fronts, testées (`ctest` vert) ; Échap ne ferme plus ;
  build `/W4 /WX`, documenté.

## Exigences
`EX-CTRL-011`, `EX-CTRL-021`, `EX-CTRL-001`.
