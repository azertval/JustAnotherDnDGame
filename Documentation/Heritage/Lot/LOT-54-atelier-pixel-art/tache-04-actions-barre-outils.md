# TACHE-04 — Actions, barre d'outils, état et historique visuel {#lot-54-tache-04-actions-barre-outils}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/HMI/Editor` · **Statut :** fait

## Contexte
La tâche par laquelle l'atelier devient un citoyen de l'éditeur plutôt qu'une fenêtre à part. Elle
n'existait pas dans le cadrage initial du lot : le canevas y arrivait avec ses propres boutons, sa
propre palette d'outils et aucun affichage d'état — c'est-à-dire en reproduisant les trois défauts
que [LOT-56](@ref lot-56) et [LOT-57](@ref lot-57) viennent de corriger. Tout ce dont elle a besoin
existe désormais et n'a qu'à être réutilisé.

Trois mécanismes en particulier :

- `hmi::EditorActions` et `hmi::ThemeIcons` (TACHE-04 de LOT-56) : une commande est une **action**
  portant son libellé traduit, son icône, son raccourci et son état cochable, plaçable dans une barre
  d'outils, un menu et un menu contextuel sans duplication.
- `hmi::EditorStatus` (TACHE-01 de LOT-57) : le contenu de la barre d'état est décidé par une
  **fonction pure**, testée sans Qt, sur le patron établi par `hmi::gameHudLines` au `LOT-52`.
- `hmi::PanelFocus` (TACHE-02 de LOT-57) : la correspondance pure outil → panneau à mettre en avant,
  et la règle « suivre l'outil actif tant que l'utilisateur n'a rien imposé ».

Le point le plus délicat du lot se joue ici : l'application a désormais **deux historiques**, celui
de `core::LevelDraft` et celui du canevas (TACHE-02). Le cadrage initial traitait ce conflit comme un
simple point d'attention sur le focus. Avec les actions, il devient une question de conception, et sa
réponse est meilleure : **une seule** paire d'actions Annuler/Refaire, dont la cible est le contexte
d'édition actif, et dont le libellé nomme l'opération concernée.

## Travail à réaliser
- **Outils du canevas comme actions** : pinceau, gomme, pot de peinture, pipette — et, si la TACHE-06
  est réalisée, sélection — enregistrés dans `hmi::EditorActions`, en **groupe exclusif** distinct de
  celui des outils de niveau.
- **Icônes dessinées par code** ajoutées à `hmi::ThemeIcons`, selon le même patron : géométrie
  produite par une fonction **pure** (liste de primitives et de rôles de couleur), peinte à la taille
  et à l'échelle demandées, recolorée depuis les jetons.
- **Barre d'outils du canevas**, alimentée par ces actions. Aucun bouton radio empilé n'est ajouté par
  ce lot.
- **Annuler et Refaire à cible contextuelle** : une seule paire d'actions pour toute l'application ;
  entrer dans le canevas leur donne l'historique du canevas pour cible, en sortir rend celui du
  niveau. Le libellé affiche l'opération qui sera annulée (« Annuler le remplissage »), et l'action
  est désactivée quand la pile correspondante est vide. Les raccourcis proviennent des actions
  remappables de `hmi::EditorKeyBindings`, branchées par la TACHE-04 de LOT-57 — pas d'interception
  en dur dans un widget.
- **Extension du modèle d'état** : `hmi::EditorStatus` reçoit un **contexte d'édition d'asset** en
  plus du contexte niveau — asset ouvert, indicateur de modifications non enregistrées, outil de
  canevas actif, pixel survolé, facteur de zoom, couleur courante, et case de planche survolée
  lorsque le mode planche est actif (TACHE-08). La décision reste une fonction pure ; aucun second
  modèle n'est créé.
- **Aide contextuelle** à l'outil de canevas actif, dans la même zone que celle des outils de niveau,
  restaurée après l'expiration d'un message transitoire — le comportement établi par LOT-57.
- **Panneau d'historique visuel** : liste ordonnée des opérations nommées de la TACHE-02, avec retour
  à un point antérieur d'un seul geste, et indication de la position courante dans la liste.
- **Insertion dans le regroupement des panneaux** : le panneau d'historique et le canevas entrent dans
  la disposition en onglets de LOT-57 et dans la table `hmi::PanelFocus`, en respectant la règle de
  mise en avant — suggestion tant que l'utilisateur n'a rien imposé, silence ensuite.
- **Traduction** : libellés, infobulles et aides dans les deux catalogues ; les infobulles affichent
  le raccourci de l'action sans qu'il soit saisi une seconde fois.

## Fichiers impactés
- `Source/HMI/Interface/EditorActions.{h,cpp}` (créé en `LOT-56`) — actions du canevas, groupe
  exclusif, Annuler/Refaire à cible contextuelle.
- `Source/HMI/Interface/ThemeIcons.{h,cpp}` (créé en `LOT-56`) — icônes des outils de canevas.
- `Source/HMI/Editor/EditorStatus.{h,cpp}` (créé en `LOT-57`) — contexte d'édition d'asset.
- `Source/HMI/Editor/PanelFocus.{h,cpp}` (créé en `LOT-57`) — panneaux de l'atelier.
- `Source/HMI/Editor/PixelHistoryPanel.{h,cpp}` (nouveau) — historique visuel.
- `Source/HMI/Editor/PixelCanvas.{h,cpp}` — raccordement des actions et exposition de l'état.
- `Source/HMI/Interface/MainWindow.{h,cpp}`, `Source/Elements/UI/MainWindow.ui` — barre d'outils et
  onglet du canevas.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Interface/test_editor_actions.cpp` (complété),
  `Source/Test/Unit/HMI/Editor/test_editor_status.cpp` (complété),
  `Source/Test/Unit/HMI/Editor/test_panel_focus.cpp` (complété).

## Tests (obligatoires)
- **Cible d'Annuler** : avec le contexte canevas actif, annuler retire la dernière opération de
  pixels et **ne touche pas** au brouillon de niveau ; avec le contexte niveau actif, l'inverse. Le
  test qui décrit le défaut évité.
- **Libellé d'Annuler** : il nomme la dernière opération de la pile active, et l'action est
  désactivée quand cette pile est vide.
- **Exclusivité des outils de canevas** : activer un outil désactive le précédent ; il y a toujours
  exactement un outil de canevas actif quand le canevas a la main, et le groupe des outils de niveau
  n'est pas perturbé.
- **Définition unique du raccourci** : aucun raccourci de commande de canevas n'est déclaré ailleurs
  que dans son action — le test de LOT-56 est étendu aux nouvelles actions.
- **Contexte d'état de l'atelier** : à partir d'un état d'édition d'asset donné, `hmi::EditorStatus`
  produit les libellés attendus ; les champs sans objet (curseur hors image, aucun asset ouvert)
  restent vides plutôt que de porter un libellé de remplacement.
- **Restauration de l'aide** : après un message transitoire, l'aide de l'outil de canevas actif
  revient à l'identique.
- **Géométrie des icônes** : chaque icône ajoutée produit sa description géométrique par une fonction
  pure, testée sans GPU ni instance d'application.
- **Mise en avant** : la table `hmi::PanelFocus` couvre les outils de canevas ; après une sélection
  manuelle d'onglet, un changement d'outil ne déplace plus la mise en avant.
- Chaque clé de traduction ajoutée existe dans les deux catalogues.

## Points d'attention
- **Deux paires d'actions Annuler/Refaire seraient un doublon**, exactement ce qu'`EX-IHM-062`
  interdit. La bonne réponse n'est pas d'arbitrer par le focus au dernier moment, mais de n'avoir
  qu'une action et de lui désigner sa cible quand le contexte d'édition change.
- **Ne pas créer un second modèle d'état.** La tentation est réelle : le contexte de l'atelier n'a
  presque aucun champ en commun avec celui du niveau. C'est pourtant la même barre, la même règle de
  restauration après message et la même exigence de pureté ; deux modèles divergeraient au premier
  ajustement.
- Le viewport peut changer d'outil par une touche ; l'action cochée doit suivre, comme la
  resynchronisation déjà en place entre *GameViewport* et *ToolPanel*.
- Les icônes des outils de canevas doivent rester lisibles à la plus petite taille d'icône des
  jetons : une pipette dessinée trop finement disparaît à 16 pixels logiques.

## Définition de fait (DoD)
- Les outils du canevas sont des actions exclusives dans une barre d'outils à icônes dessinées par
  code ; une seule paire Annuler/Refaire existe, à cible contextuelle et à libellé nommé ; la barre
  d'état affiche l'état de l'atelier via le modèle pur existant, étendu et non dupliqué ; le panneau
  d'historique permet le retour à un point antérieur ; les panneaux entrent dans le regroupement et
  la mise en avant ; chaînes traduites dans les deux catalogues ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-IHM-055` (commandes exposées comme
actions), `EX-IHM-060` (état de travail affiché en permanence), `EX-IHM-061` (panneaux groupés
suivant l'outil actif), `EX-IHM-062` (un état ou une commande à un seul endroit), `EX-EDIT-015`
(découvrabilité des commandes), `EX-CTRL-012` (raccourcis d'éditeur remappables), `EX-REN-033`
(traduction), `EX-NFR-010` (testable sans GPU).
