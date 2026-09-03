# TACHE-04 — Outil « Parcours » : geometrie, geste et canevas {#lot-67-tache-04-outil-parcours}

**Lot :** [LOT-67](epic.md) · **Emplacement :** `Source/HMI/{Editor,Game,Interface}` · **Statut :** fait

## Contexte
Aucun outil ne permettait de toucher a une trajectoire. Le patron a suivre existait pourtant, pose
par le `LOT-50` pour les decors : une geometrie de poignees pure (`DecorGeometry`) et une machine a
etats de geste pure (`DecorGesture`), le viewport n'etant qu'un routeur d'evenements Qt.

## Travail a realiser
- `hmi::PathGeometry` : poignees de point et de milieu de segment, a **taille ecran constante**,
  calculees depuis `core::platformPathPoints` — donc depuis la trajectoire reellement parcourue. Une
  poignee unique d'extremite pour un danger mobile.
- Le hit-test teste les points **avant** les milieux : sur une route courte un milieu peut recouvrir
  un point, et deplacer est le geste attendu par defaut.
- `hmi::PathGesture` : designation, seuil clic/glisser, et une action unique en fin de geste
  (`MoveWaypoint`, `InsertWaypoint`, `SetMoverRange`). Le geste **ne mute rien**.
- Le seuil est reevalue **au relachement** et pas seulement pendant le glisser : un glisser assez
  rapide peut n'avoir produit aucun evenement de deplacement intermediaire, et doit neanmoins
  aboutir.
- Ajouter `EditorTool::Path`, son icone dessinee par code, son entree de catalogue et ses libelles
  traduits ; router appui, deplacement, relachement, clic droit et `Echap` dans `GameViewport`.
- Un `DangerBlink` n'ayant pas de trajectoire, il est selectionne **par sa case** : sans quoi ses
  reglages de temporisation resteraient inatteignables.

## Fichiers impactes
`Source/HMI/Editor/PathGeometry.{h,cpp}`, `PathGesture.{h,cpp}` (nouveaux), `EditorTool.h`,
`EditorStatus.cpp`, `Source/HMI/Interface/IconGeometry.{h,cpp}`, `ActionCatalog.{h,cpp}`,
`Source/HMI/Game/GameViewport.{h,cpp}`, `Source/Elements/Localization/*.lang`.

## Tests (obligatoires)
- `test_path_geometry.cpp` : le depart n'a pas de poignee ; le circuit ferme expose un milieu de
  plus ; taille ecran constante a deux zooms ; priorite du point sur le milieu ; poignee
  d'extremite d'un danger mobile ; route vide sans poignee.
- `test_path_gesture.cpp` : clic de selection sans manipulation ; clic sans mouvement sans effet ;
  glisser deplacant un point sur la case visee ; glisser d'un milieu inserant au bon rang ; **une
  seule** action finale par geste ; abandon sans effet ; axe et portee d'un danger mobile, portee
  jamais negative.

## Points d'attention
`EDITOR_ACTION_CATALOG_COUNT` est une constante en dur : l'oublier casse le test de catalogue.
Le point de depart n'a **pas** de poignee, et aucune poignee de coin n'est introduite — elle serait
indistinguable d'un point de route et laisserait croire a un redimensionnement, qui n'existe pas.

## Definition de fait (DoD)
L'outil apparait dans la barre d'outils, les gestes sont couverts par des tests purs sans Qt ni
GPU, et chaque geste complet ne coute qu'un pas d'annulation. `ctest` a 100 %.

## Exigences
`EX-EDIT-032`, `EX-EDIT-005`, `EX-GP-051`, `EX-GP-054`, `EX-NFR-010`.
