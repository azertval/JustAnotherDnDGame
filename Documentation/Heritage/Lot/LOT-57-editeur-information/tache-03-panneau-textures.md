# TACHE-03 — Recentrage du panneau Textures {#lot-57-tache-03-panneau-textures}

**Lot :** [LOT-57](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/Elements/UI` · **Statut :** fait

## Contexte
Le panneau Textures est le plus gros fichier de widget du projet et concentre environ un tiers des
contrôles de l'éditeur. Ses six onglets ont été ajoutés lot après lot — Skins au `LOT-42`, Fond au
`LOT-44`, Objets au `LOT-45`, Animations au `LOT-46`, Décors au `LOT-50`, Calques au `LOT-51` — sans
qu'on se demande jamais s'ils relèvent du même sujet. Ils n'en relèvent pas. Trois natures cohabitent :

- **Définir l'apparence du niveau** : Skins, Fond, Animations. C'est le sujet du panneau.
- **Inspecter une instance sélectionnée** : Objets et Décors listent des éléments posés et agissent
  dessus. Ce sont des inspecteurs, et leur contenu répond à la sélection faite au canevas.
- **Regarder le niveau autrement** : Calques décompose le rendu par couche. Ce n'est pas une propriété
  de texture du tout.

Le mauvais rangement du dernier onglet se lit dans le code lui-même : il a fallu y afficher un
avertissement permanent expliquant qu'il n'affecte jamais le jeu, et il y voisine une case qui double la
touche de bascule Physique/Texture — au point qu'une infobulle doit préciser qu'il s'agit de la même
bascule vue d'ailleurs. Deux symptômes du même problème.

## Travail à réaliser
- **Déplacement du mode d'inspection par calque vers le menu Affichage**, où vivent déjà les bascules de
  panneaux : une entrée par calque, dans l'ordre de dessin, plus « tout afficher ». L'avertissement
  permanent devient inutile — le menu Affichage dit de lui-même qu'on règle une vue.
- **Recentrage du panneau sur la définition d'apparence** : Skins, Fond, Animations. C'est le
  voisinage où l'atelier pixel art de [LOT-54](@ref lot-54) prendra son second point d'entrée
  (« éditer cet asset », sur l'asset couramment assigné) — une raison de plus de ne garder ici que ce
  qui définit réellement l'apparence.
- **Inspecteurs suivant la sélection** : les onglets Objets et Décors présentent l'élément sélectionné
  au canevas plutôt qu'une liste à parcourir indépendamment, la sélection croisée existante entre la
  table des décors et le canevas étant conservée dans les deux sens.
- **Clarification des deux listes déroulantes « jeu de skins »** : celle de l'en-tête porte sur la
  session d'édition, celle de l'onglet Fond sur le niveau. Deux libellés distincts et explicites, ou un
  seul contrôle si l'usage montre qu'un des deux est superflu.
- **Traduction** de tous les libellés déplacés ou reformulés, dans les deux catalogues ; retrait des
  clés devenues inutilisées.
- **Comportement inchangé** : le mode d'inspection par calque reste réservé à l'éditeur et sans aucun
  effet sur la session de jeu ni sur la bascule Physique/Texture (`EX-EDIT-044`, `LOT-51`).

## Fichiers impactés
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/Elements/UI/TexturePanel.ui`.
- `Source/HMI/Interface/MainWindow.{h,cpp}`, `Source/Elements/UI/MainWindow.ui` — entrées du menu
  Affichage.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- Tests existants de visibilité des calques — mis à jour, pas réécrits.

## Tests (obligatoires)
- **Visibilité des calques inchangée** : les tests du `LOT-51` continuent de passer à l'identique — le
  déplacement est une affaire de présentation, pas de comportement.
- **Synchronisation du menu** : cocher une entrée de calque met à jour l'état de rendu, et la
  réinitialisation « tout afficher » rétablit toutes les entrées.
- **Sélection croisée** : sélectionner un décor au canevas met à jour l'inspecteur, et réciproquement.
- Chaque clé de traduction utilisée existe dans les deux catalogues ; aucune clé orpheline n'est
  laissée derrière.

## Points d'attention
- **Ne pas confondre les deux mécanismes de vue.** La bascule Physique/Texture **compose** le rendu ; la
  visibilité par calque le **décompose**. Le `LOT-51` insiste sur cette distinction, et la déplacer dans
  le menu Affichage ne doit surtout pas les fusionner : ce sont deux entrées distinctes, dont l'une est
  traitée en TACHE-04.
- **Le panneau reste volumineux après cette tâche**, et c'est acceptable : son découpage en tant que
  refactoring est hors périmètre du lot. Ne pas dériver vers une restructuration du fichier.
- Les onglets Objets et Décors portent des actions destructrices (retirer une surcharge, supprimer un
  décor) : leur passage en inspecteurs ne doit pas les rendre déclenchables sur une sélection vide.
- Vérifier qu'aucune clé de traduction ne devient orpheline : le projet en compte déjà plusieurs, et
  cette tâche ne doit pas en ajouter.

## Définition de fait (DoD)
- Le mode d'inspection par calque est piloté depuis le menu Affichage, sans avertissement permanent, et
  son comportement est inchangé ; le panneau Textures ne porte plus que la définition d'apparence, ses
  inspecteurs suivant la sélection du canevas ; les deux sélecteurs de jeu de skins sont distinguables ;
  aucune clé de traduction orpheline ; `/W4 /WX` propre.

## Exigences
`EX-IHM-062` (un état à un seul endroit) ; réutilise `EX-EDIT-044` (visibilité des calques, `LOT-51`),
`EX-EDIT-024` (jeux de skins), `EX-EDIT-043` (surcharge de texture par case), `EX-DEC-010` (édition de
décors), `EX-REN-033` (traduction).
