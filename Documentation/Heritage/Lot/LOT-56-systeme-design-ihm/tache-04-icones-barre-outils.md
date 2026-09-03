# TACHE-04 — Icônes dessinées par code et barre d'outils à actions {#lot-56-tache-04-icones-barre-outils}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/HMI/Editor` · **Statut :** fait

## Contexte
`EX-EDIT-015` exige depuis le `LOT-15` que l'éditeur expose ses commandes « de façon découvrable à
l'écran : une **barre d'outils** pour changer d'outil ». Il n'existe aucune barre d'outils dans le
projet : les six outils sont des **boutons radio empilés verticalement** dans le panneau Outils, et
`QToolBar` comme `QActionGroup` sont absents de tout `Source/HMI`. `QIcon` n'y apparaît que comme
conteneur de vignettes d'assets — il n'existe aucun jeu d'icônes.

Cette absence a une seconde conséquence, moins visible et plus coûteuse : une commande n'existe nulle
part comme **entité unique**. Enregistrer est défini une fois comme entrée de menu et une fois comme
raccourci intercepté dans *GameViewport* ; essayer le niveau également. Une action Qt, à l'inverse,
porte son libellé, son icône, son raccourci et son état cochable, et peut être placée simultanément
dans une barre d'outils, un menu et un menu contextuel **sans duplication**. C'est le mécanisme sur
lequel [LOT-57](@ref lot-57) s'appuiera pour supprimer les doublons de commande — raison pour laquelle
cette tâche appartient au socle et non au lot d'ergonomie.

Le projet dispose enfin d'un précédent pour produire du contenu graphique sans fichier d'asset :
`hmi::buildProceduralAtlasImage` (`LOT-39`) et `hmi::ProceduralFont` (`LOT-52`) dessinent le leur par
code, avec repli déterministe.

## Travail à réaliser
- **Jeu d'icônes dessinées par code** : une icône par outil (pinceau, rectangle, sélection, lien,
  texture, décor) et par commande principale (enregistrer, essayer, annuler, refaire, grille,
  recadrer, mode de rendu). Dessin vectoriel simple au moment du rendu, à la taille et à l'échelle
  d'affichage demandées, **recoloré depuis les jetons** — donc net partout et cohérent avec le thème,
  y compris après le changement de thème de la TACHE-06.
- **Actions** : chaque outil et chaque commande principale devient une action portant son libellé
  traduit, son icône, son raccourci et, le cas échéant, son caractère cochable. Les outils forment un
  groupe **exclusif**.
- **Barre d'outils** de l'éditeur, alimentée par ces actions, remplaçant les boutons radio du panneau
  Outils.
- **Remplacement des définitions dupliquées** : les entrées de menu existantes et le raccourci
  correspondant sont fournis par la même action, plus définis séparément.
- **Traduction** : tous les libellés et infobulles passent par le catalogue (`EX-REN-033`), dans les
  deux langues ; les infobulles affichent le raccourci de l'action sans qu'il soit saisi une seconde
  fois.
- **Synchronisation conservée** : le viewport peut changer l'outil actif (touche dédiée) ; l'action
  cochée doit suivre, comme le fait aujourd'hui la resynchronisation entre *GameViewport* et
  *ToolPanel*.
- **Ouvert à un second contexte d'édition** : les actions et les icônes ne sont pas réservées à
  l'éditeur de niveaux. L'atelier pixel art de [LOT-54](@ref lot-54) y ajoutera son propre groupe
  exclusif d'outils et ses propres icônes, selon le même patron de géométrie pure. Concevoir
  `EditorActions` et `ThemeIcons` en conséquence — un jeu d'actions extensible et plusieurs groupes
  exclusifs coexistants — coûte peu ici et évite une reprise là-bas.

## Fichiers impactés
- `Source/HMI/Interface/ThemeIcons.{h,cpp}` (nouveau) — dessin des icônes depuis les jetons.
- `Source/HMI/Interface/EditorActions.{h,cpp}` (nouveau) — définition et regroupement des actions.
- `Source/HMI/Interface/MainWindow.{h,cpp}`, `Source/Elements/UI/MainWindow.ui` — barre d'outils,
  entrées de menu alimentées par les actions.
- `Source/HMI/Editor/ToolPanel.{h,cpp}`, `Source/Elements/UI/ToolPanel.ui` — retrait des boutons radio.
- `Source/Elements/Localization/fr.lang`, `en.lang` — libellés et infobulles.
- `Source/Test/Unit/HMI/Interface/test_editor_actions.cpp` (nouveau).

## Tests (obligatoires)
- **Exclusivité des outils** : activer un outil désactive le précédent ; il y a toujours exactement un
  outil actif.
- **Définition unique** : chaque commande n'a qu'une définition de raccourci — un test parcourant les
  actions et échouant si un même raccourci est attribué deux fois, ou si une commande de menu ne
  provient pas d'une action.
- **Description des icônes** : la **géométrie** de chaque icône (liste de primitives et de rôles de
  couleur) est produite par une fonction **pure**, testée sans GPU ni instance d'application — même
  découpage que `hmi::gameHudLines`, qui sépare le choix du contenu de son rendu.
- **Traduction** : chaque clé de libellé et d'infobulle utilisée existe dans les deux catalogues.

## Points d'attention
- **Ne pas vider le panneau Outils par effet de bord.** Il porte aussi le sélecteur de couche de décor,
  la case d'aimantation à la grille et la grille de vignettes de décors : seuls les boutons radio de
  sélection d'outil partent vers la barre d'outils.
- **Conserver le masquage contextuel existant** du bloc décors, aujourd'hui masqué quand l'outil Décor
  n'est pas actif : c'est le seul réflexe d'ergonomie déjà présent dans l'éditeur, et `LOT-57` le
  généralisera.
- **Les icônes doivent rester lisibles à petite taille.** Une icône dessinée par code est nette, pas
  forcément compréhensible : viser des formes distinctes en silhouette, et vérifier les six outils côte
  à côte à la plus petite taille retenue.
- Le raccourci affiché en infobulle doit provenir de l'action, jamais d'une chaîne traduite décrivant
  la touche — sans quoi le remappage rendrait l'infobulle fausse.
- L'ordre des outils dans la barre doit rester celui de la palette et du panneau Outils : un
  réordonnancement silencieux désoriente les utilisateurs existants.

## Définition de fait (DoD)
- Le changement d'outil se fait depuis une barre d'outils à icônes ; chaque commande principale est une
  action unique fournissant simultanément l'entrée de menu, le bouton et le raccourci ; les icônes sont
  dessinées par code, nettes à toute taille et recolorées par le thème ; la géométrie des icônes est
  pure et testée ; tous les libellés existent dans les deux langues ; `/W4 /WX` propre.

## Exigences
`EX-IHM-055` (commandes exposées comme actions dans une barre d'outils) ; concrétise la partie « barre
d'outils » d'`EX-EDIT-015` (découvrabilité) ; prépare `EX-IHM-062` (commande unique, `LOT-57`) ;
réutilise `EX-REN-033` (traduction), `EX-CTRL-012` (raccourcis d'éditeur remappables).
