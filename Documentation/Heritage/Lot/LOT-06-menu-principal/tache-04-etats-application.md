# TACHE-04 — États d'application (écrans) {#lot-06-tache-04-etats-application}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
L'application n'a qu'un seul mode (la scène codée en dur). Un menu implique plusieurs
**écrans** (Menu, Jeu, Éditeur) et des **transitions** entre eux. On introduit une structure
d'états d'application minimale.

## Travail à réaliser
- Interface `IScreen` : `update(input, fixedDelta)` (logique/entrées de l'écran) et
  `render(...)` (dessin), avec la possibilité de **demander une transition** (rester,
  changer d'écran, ou quitter).
- Un gestionnaire d'écrans (dans `HMI/Interface`) détenant l'**écran courant** et appliquant
  les transitions demandées (remplacement de l'écran actif).
- Énumération/typage des écrans cibles (Menu, Jeu, Éditeur) et mécanisme de fabrication de
  l'écran suivant.

## Fichiers impactés
- `Source/HMI/Interface/IScreen.h` (nouveau).
- `Source/HMI/Interface/ScreenManager.h`, `ScreenManager.cpp` (nouveau) — ou équivalent.
- `Source/HMI/CMakeLists.txt`.

## Vérifications (obligatoires)
- Le gestionnaire exécute l'écran courant (update puis render) à chaque frame.
- Une transition demandée par un écran remplace bien l'écran actif à la frame suivante.
- Une demande de « quitter » se propage jusqu'à la boucle (`main`) pour fermer proprement.

## Points d'attention
- Cycle de vie **RAII** des écrans (possédés par le gestionnaire) ; pas d'état global.
- Découplage : un écran ignore les autres ; il exprime une intention de transition, le
  gestionnaire l'applique.
- La logique d'écran testable (transitions) doit pouvoir l'être sans fenêtre ni GPU.

## Définition de fait (DoD)
- Structure d'écrans et transitions fonctionnelles et testées (`ctest` vert) ;
  build `/W4 /WX`, documentée.

## Exigences
`EX-REN-030`, `EX-NFR-010`.
