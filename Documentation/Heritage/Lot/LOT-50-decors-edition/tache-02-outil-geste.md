# TACHE-02 — Outil de décors et geste pur de manipulation {#lot-50-tache-02-outil-geste}

**Lot :** [LOT-50](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Manipuler un décor à la souris est le geste le plus riche de l'éditeur : il faut désigner une cible
parmi des objets qui se recouvrent, distinguer un clic d'un glisser, savoir si l'on déplace ou si
l'on redimensionne selon la poignée saisie, et gérer l'abandon.

Le projet a un précédent exact de geste non trivial traité proprement : `hmi::LinkGesture` et
`hmi::resolveLinkClick` (`LOT-37`) — une machine à états **pure**, testée sans Qt, dont le viewport
n'est que le routeur d'événements. C'est ce patron qu'il faut suivre, sans quoi la logique de
manipulation finirait dispersée dans des gestionnaires d'événements Qt intestables.

## Travail à réaliser
- **Nouvelle valeur `hmi::EditorTool`** (ex. *DecorPlace*), ajoutée au panneau Outils, à ses libellés
  traduits et à la table de raccourcis de l'éditeur.
- **Geste pur** (`DecorGesture`) : machine à états sur des données, sans Qt ni GPU, couvrant
  - **désignation** : quel décor est sous le curseur, en respectant l'ordre de superposition (le plus
    au-dessus d'abord) et la visibilité des couches ;
  - **appui / glisser / relâchement** : distinguer sélection simple et déplacement, avec un seuil de
    déplacement minimal pour ne pas déplacer d'un pixel à chaque clic ;
  - **poignées** : identifier la poignée saisie (coins pour l'échelle, poignée dédiée pour la
    rotation) et calculer la transformation résultante ;
  - **abandon** : `Échap` en cours de glisser restaure l'état initial.
- **Sortie du geste** : une **description d'action** (déplacer tel décor de tant, redimensionner à
  telle échelle…), appliquée par l'appelant aux mutateurs de TACHE-01 — même séparation que
  `resolveLinkClick`.
- **Aimantation optionnelle** sur la grille, activable/désactivable, jamais imposée : un décor est
  libre par construction (`EX-DEC-001`), l'aimantation n'est qu'un confort d'alignement.

## Fichiers impactés
- `Source/HMI/Editor/EditorTool.h`, `ToolPanel.{h,cpp}`, `Source/Elements/UI/ToolPanel.ui`.
- `Source/HMI/Editor/DecorGesture.{h,cpp}` (nouveau).
- `Source/HMI/Game/GameViewport.{h,cpp}` (routage des événements).
- `Source/HMI/Input/EditorKeyBindings.{h,cpp}`.
- `Source/Test/Unit/HMI/Editor/test_decor_gesture.cpp` (nouveau).

## Tests (obligatoires)
- **Désignation** : décors superposés → le plus au-dessus est désigné ; décor sur une couche masquée
  (LOT-51) → ignoré ; clic dans le vide → aucune sélection.
- **Clic contre glisser** : un déplacement sous le seuil ne produit aucune action de déplacement.
- **Poignées** : chaque poignée produit la transformation attendue, y compris pour un redimensionnement
  depuis un coin opposé.
- **Abandon** : `Échap` en cours de geste ne laisse aucune modification.
- **Aimantation** : activée → position alignée ; désactivée → position exacte.
- Machine à états **pure**, testée sans Qt ni GPU.

## Points d'attention
- **Le geste ne mute rien.** Il décrit une action ; c'est l'appelant qui l'applique au brouillon.
  C'est ce qui le rend testable et ce qui garantit que chaque manipulation passe par l'historique.
- Un décor **très petit** ou **très grand** doit rester sélectionnable et redimensionnable : prévoir
  une taille minimale de zone cliquable pour les poignées, indépendante du zoom.
- La désignation doit tenir compte du zoom et du décalage de parallaxe (LOT-49, TACHE-03) : le décor
  est affiché à une position différente de sa position modèle. Cliquer là où on le **voit** doit
  sélectionner le bon décor — c'est un piège certain si la conversion n'est pas explicite.

## Définition de fait (DoD)
- L'outil existe, est sélectionnable au panneau et au clavier ; placer, sélectionner, déplacer,
  redimensionner et pivoter fonctionnent, avec abandon et aimantation optionnelle ; le geste est pur
  et testé sans Qt ; `/W4 /WX` propre.

## Correctif post-livraison
La mise en garde ci-dessus (« Points d'attention ») avait été identifiée mais pas suivie à la
livraison initiale : la désignation/les poignées comparaient le curseur (position de **rendu**) à
des rectangles calculés depuis la position **modèle** brute des décors, jamais convertie. Sans
effet sur la couche `Decor` (facteur `1.0`, `EX-DEC-006`), mais un décor en couche
Arrière-plan/Premier plan désignait/manipulait à côté de ce qui était visible à l'écran — rapporté
comme « le cadre de modification se désolidarise du décor ». Corrigé en convertissant
explicitement aux deux frontières concernées (`GameViewport::decorBoundsForGesture`/
`selectedDecorHandles` vers l'espace de rendu, `hmi::parallaxRenderPosition` ; le curseur vers
l'espace modèle avant d'entrer dans `hmi::DecorGesture`, `hmi::parallaxModelPosition`, son inverse
exact) — voir @ref guide-rendu pour le détail. `hmi::DecorGesture` lui-même n'a pas changé : il n'a
jamais connu la parallaxe, et n'a pas à la connaître.

## Exigences
`EX-DEC-010` (placer, déplacer, redimensionner, supprimer), `EX-EDIT-040` (édition de décors) ;
réutilise `EX-DEC-001` (position libre), `EX-EDIT-005` (annuler/refaire), `EX-EDIT-030` (éditeur
intégré), `EX-CTRL-012` (remappage), `EX-REN-033` (traduction).
