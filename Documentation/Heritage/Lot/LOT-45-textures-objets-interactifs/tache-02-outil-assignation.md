# TACHE-02 — Outil et geste d'assignation de texture {#lot-45-tache-02-outil-assignation}

**Lot :** [LOT-45](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
L'éditeur dispose déjà d'outils sélectionnables (`hmi::EditorTool` : Pinceau, Rectangle, Sélection,
Lien) et d'un précédent exact pour un geste au clic non trivial : `hmi::LinkGesture` et
`hmi::resolveLinkClick` (LOT-37), une machine à états **pure**, testée sans Qt, dont le panneau
n'est que le déclencheur.

Assigner une texture à une case est un geste plus simple — un clic, une cible, un asset — mais qui
mérite le même traitement : la logique dans une fonction testable, l'UI réduite au routage
d'événements.

## Travail à réaliser
- **Nouvelle valeur `hmi::EditorTool`** (ex. *TextureAssign*), ajoutée au panneau Outils et à ses
  libellés traduits.
- **Geste pur** (`TextureAssignGesture`) : à partir d'une case cliquée, de l'état du niveau et de
  l'asset actuellement sélectionné dans la bibliothèque, décider de l'action — assigner, remplacer,
  retirer (clic sur une case déjà assignée avec le même asset, ou clic droit), ou ne rien faire
  (case vide).
- **Retour visuel** : les cases portant un override sont signalées dans le canevas quand l'outil est
  actif, sur le calque d'aides d'édition (*EditorOverlay*) — sinon l'auteur ne sait pas ce qui est
  déjà habillé.
- **Raccourci clavier** de sélection de l'outil, via la table remappable de l'éditeur
  (`EditorKeyBindings`) comme les outils existants.

## Fichiers impactés
- `Source/HMI/Editor/EditorTool.h`, `ToolPanel.{h,cpp}`, `Source/Elements/UI/ToolPanel.ui`.
- `Source/HMI/Editor/TextureAssignGesture.{h,cpp}` (nouveau).
- `Source/HMI/Game/GameViewport.{h,cpp}` (routage des clics selon l'outil actif).
- `Source/HMI/Input/EditorKeyBindings.{h,cpp}`.
- `Source/Test/Unit/HMI/Editor/test_texture_assign_gesture.cpp` (nouveau).

## Tests (obligatoires)
- Décision du geste pour chaque cas : case vide, case sans override, case avec le même override,
  case avec un override différent, aucun asset sélectionné. Fonction **pure**, sans Qt ni GPU.
- Le geste ne modifie rien par lui-même : il **décrit** l'action, c'est l'appelant qui l'applique au
  brouillon (même séparation que `resolveLinkClick`).

## Points d'attention
- **Ne pas étendre `LinkGesture`.** Sa machine à états est façonnée pour l'appariement
  déclencheur/cible en deux clics ; y greffer une assignation à un clic coupleraient deux outils
  sans rapport et rendrait les deux plus fragiles.
- L'outil doit être **sans effet** en mode Physique, ou signaler clairement que le résultat n'est
  visible qu'en mode Texture (`F8`).
- Vérifier l'interaction avec l'annulation : une assignation est une action d'édition à part
  entière, elle doit apparaître dans l'historique au même titre qu'un coup de pinceau.

## Définition de fait (DoD)
- L'outil existe, est sélectionnable au panneau et au clavier, assigne et retire une texture par
  clic, signale les cases déjà habillées, et sa décision est testée sans Qt ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-043` (texture par instance) ; réutilise `EX-EDIT-005` (annuler/refaire), `EX-EDIT-030`
(éditeur intégré), `EX-CTRL-012` (remappage), `EX-REN-033` (traduction).
