# Créer et partager un niveau (sans ligne de commande) {#manuel-partager-niveau}

Ce guide s'adresse aux **créateurs de niveaux** (level design) sans connaissance en
programmation. Il explique comment créer un niveau dans l'éditeur intégré, puis le partager avec
le reste de l'équipe via une interface Git graphique — **jamais de ligne de commande**.

## 1. Récupérer le projet

1. Installer [GitHub Desktop](https://desktop.github.com/) (gratuit).
2. Ouvrir GitHub Desktop, se connecter avec un compte GitHub (en créer un si besoin — gratuit).
3. **File → Clone repository**, choisir `azertval/JustAnotherDnDGame`, puis un dossier sur votre
   ordinateur. Le bouton **Clone** télécharge tout le projet.

## 2. Lancer l'éditeur

Récupérez l'exécutable du jeu (`JustAnotherDnDGame.exe`) — voir
[Télécharger et lancer le jeu](@ref manuel-telecharger) si vous n'avez pas encore compilé le
projet vous-même. Depuis le menu principal, choisissez **Mode Édition**.

## 3. Créer un niveau

À l'ouverture de l'éditeur, une liste apparaît : **Nouveau niveau**, ou l'un des niveaux déjà
enregistrés. Naviguez avec les flèches **↑**/**↓**, validez avec **Entrée**.

En choisissant **Nouveau niveau**, un champ vous demande un **nom** : tapez-le puis validez avec
**Entrée** (**Échap** annule et revient à la liste). Un nom ne doit pas être vide, ni contenir
d'antislash, de barre oblique, de deux-points, d'astérisque, de point d'interrogation, de guillemet
droit, de chevron ou de barre verticale (caractères interdits par Windows dans un nom de fichier) —
le champ vous le signale si besoin.

Une fois dans la grille, la **palette** de tuiles et la **barre d'outils** occupent le panneau à
gauche de l'écran ; appuyez sur **F1** à tout moment pour afficher la liste complète des raccourcis
à l'écran (**F1** de nouveau pour la refermer).

| Action | Comment |
|--------|---------|
| Peindre une tuile | Cliquer une case de la **palette** (panneau de gauche) pour choisir son type, puis cliquer (ou cliquer-glisser) sur la grille. |
| Placer l'entrée / la sortie | Choisir *Entrée* ou *Sortie* dans la palette, puis cliquer la case voulue — l'ancienne position se déplace automatiquement (un niveau n'a qu'une seule entrée et une seule sortie). |
| Relier un interrupteur (ou une plaque de pression) à une porte | Peindre d'abord un *Interrupteur* ou une *Plaque* et une *Porte*, puis maintenir **Maj** et cliquer le déclencheur, puis (Maj toujours enfoncé) cliquer la porte. Recommencer la même paire retire la liaison. Cette liaison est possible avec n'importe quel outil actif. Un interrupteur **bascule** l'état de la porte (reste ouverte une fois activée) ; une plaque de pression garde la porte ouverte **tant qu'on reste dessus** et la referme dès qu'on en part. |
| Placer un bloc poussable | Peindre un *Bloc* comme n'importe quelle tuile — aucune liaison à faire. En jeu, le personnage peut le pousser horizontalement (une case à la fois) s'il n'y a rien derrière, et il tombe s'il n'est plus soutenu par le dessous. |
| Placer une pente | Peindre *Pente D* (montante de gauche à droite) ou *Pente G* (montante de droite à gauche) comme n'importe quelle tuile. En jeu, le personnage suit sa surface inclinée en marchant, sans ressaut ; la faire déboucher sur une case *Pleine* de même hauteur (là où la pente atteint son bord haut) raccorde proprement à un palier plat. |
| Placer un arrondi | Peindre *Arrondi D* ou *Arrondi G* comme n'importe quelle tuile — même orientation que les pentes, mais surface **courbe** (quart de cercle) plutôt que rectiligne. Le personnage y ressent une accélération verticale progressive côté tangente verticale, à la différence d'une pente qui monte à vitesse constante. |
| Placer un bloc réduit | Peindre *Bloc 1/2* ou *Bloc 1/4* comme n'importe quelle tuile — même comportement qu'un *Bloc* (poussable, tombe s'il n'est plus soutenu), mais sa boîte réelle est plus **petite** que la case et **centrée** dedans : l'espace vide qui l'entoure reste franchissable, seule sa boîte réduite arrête le personnage. |
| Changer d'outil | **Tab** fait défiler **Pinceau** (case par case) → **Rectangle** (remplit toute une zone glissée) → **Sélection** (voir ci-dessous) → Pinceau ; ou cliquer directement l'outil voulu dans le panneau. |
| Remplir une zone | Outil **Rectangle** : cliquer-glisser d'un coin à l'autre de la zone, relâcher pour la remplir du type choisi dans la palette. |
| Copier / coller une zone | Outil **Sélection** : cliquer-glisser pour définir la zone, puis **Ctrl+C** pour la copier ; **Ctrl+V** colle la copie à l'endroit survolé par la souris. |
| Déplacer la vue / zoomer | Cliquer-glisser avec le **bouton droit** de la souris pour déplacer la vue ; **molette** pour zoomer/dézoomer ; touche **0** pour revenir au cadrage automatique. |
| Afficher un quadrillage | **F10** — des lignes fines apparaissent sur chaque case, utile pour bien viser avant de peindre. |
| Voir le niveau habillé ou brut | **F8** — bascule entre le niveau **habillé** (les cases physiques projettent une ombre légère sur ce qu'il y a derrière elles, pour distinguer d'un coup d'œil ce qui porte de ce qui n'est que décor) et le niveau **tel qu'il est construit** (chaque case dans une couleur unie selon son rôle). Indispensable pour vérifier qu'un décor ne fait pas croire à un sol qui n'existe pas. Fonctionne aussi pendant l'essai (**P**) et en jeu. Le choix est conservé pour les fois suivantes. |
| Vérifier ce qui manque encore d'habillage | Onglet **Calques** du panneau **Textures** : une case à cocher par plan (Fond, Décors, Ombres, Skin des tuiles, Objets, Personnage…). Ne cocher que **Skin des tuiles** montre uniquement les tuiles déjà habillées — les cases restées vides sont celles qui n'ont pas encore de texture. Le bouton **Tout afficher** revient à la vue normale. Ceci est un outil de **vérification pour l'éditeur uniquement** : il ne change jamais ce que le joueur voit. |
| Agrandir / réduire la grille (case par case) | Flèches **←**/**→** pour la largeur, **↑**/**↓** pour la hauteur. Si la réduction supprimerait l'entrée, la sortie ou une liaison, une confirmation est demandée (**Entrée** = confirmer, **Échap** = annuler) avant d'agir. |
| Choisir une taille précise | **Ctrl+R**, taper la nouvelle taille au format `largeurxhauteur` (ex. `60x40`, ou `60*40`), **Entrée** pour valider (**Échap** annule sans rien changer). Même plafond et même confirmation destructrice qu'aux flèches. |
| Renommer le niveau | **F2**, taper le nouveau nom, **Entrée** pour valider (**Échap** pour annuler sans rien changer). |
| Annuler / refaire | **Ctrl+Z** / **Ctrl+Y**. |
| Tester le niveau | Touche **P** — le niveau se joue directement dans l'éditeur ; **Échap** pour revenir à l'édition, rien n'est perdu. |
| Enregistrer | **Ctrl+S** — un message en bas de l'écran confirme l'enregistrement, ou explique le problème si le niveau n'est pas encore jouable (par exemple : il manque une sortie). Si le nom correspond à un **autre** niveau déjà enregistré, une confirmation est demandée avant d'écraser ce fichier. |
| Quitter l'éditeur | **Échap** (hors essai immédiat) — retour au menu. Si des modifications ne sont pas encore enregistrées, une confirmation est demandée avant de les perdre. |

### Choisir comment la caméra suit le joueur

Onglet **Cadrage** du panneau **Textures** : choisissez comment la caméra du jeu cadre votre
tableau — ce réglage ne change rien à l'affichage dans l'éditeur (qui montre toujours tout le
niveau), seulement à ce que verra le joueur en jeu ou pendant un essai (**P**). Trois choix, avec un
aperçu immédiat dans le canevas (un cadre en pointillés de couleur) :

- **Niveau entier** — le joueur voit tout le tableau d'un coup d'œil, sans jamais bouger. Convient
  à un **puzzle** : le joueur doit voir l'ensemble du mécanisme pour comprendre comment le résoudre.
- **Par salle** — pour un tableau plus grand qu'un écran : la caméra reste fixe tant que le joueur
  reste dans la même « salle », puis bascule net dès qu'il en franchit la frontière (une grille de
  repère apparaît dans l'aperçu). Convient à un grand niveau construit comme une **suite de pièces**
  distinctes.
- **Suivi du personnage** — la caméra accompagne le joueur en continu, comme dans la plupart des
  jeux de plateforme. Convient à un tableau **long et linéaire** (une course, un couloir), où
  montrer tout d'un coup n'aurait pas de sens. Un champ **Taille de la caméra** (largeur/hauteur en
  cases) permet de régler ce qu'elle montre autour du joueur ; laissé vide, la taille par défaut
  s'applique.

Un niveau créé avant l'existence de ce réglage se comporte **exactement comme avant** (niveau
entier ou par salle selon sa taille) : rien à faire pour les niveaux déjà publiés.

#### Mélanger plusieurs tailles de caméra dans un même niveau (mode « Par salle »)

Le quadrillage automatique du mode **Par salle** impose une seule taille de « salle » pour tout le
tableau. Pour mélanger des tailles différentes — par exemple une grande pièce d'ensemble et un
couloir étroit dans le même niveau — dessinez vos propres zones de caméra :

1. Choisissez le mode **Par salle**.
2. Dans la barre d'outils, sélectionnez l'outil **Zone de caméra**.
3. Cliquez-glissez sur le canevas pour dessiner un rectangle : il devient une zone de caméra dès
   que vous relâchez le bouton de la souris. Répétez pour chaque zone souhaitée.
4. Le tableau **Zones de caméra**, dans l'onglet **Cadrage**, liste toutes les zones dessinées
   (position et taille). Pour en retirer une, sélectionnez-la dans ce tableau puis cliquez sur
   **Retirer** — le dessin sur le canevas ne se retire pas au clic, seulement depuis ce tableau.

Chaque geste (dessiner une zone, en retirer une) s'annule normalement avec **Ctrl+Z**. Si deux
zones se chevauchent, c'est celle dessinée **en premier** qui l'emporte à cet endroit. Tant
qu'aucune zone n'est dessinée, le quadrillage automatique décrit ci-dessus reste inchangé — dessiner
des zones est une option, pas une obligation.

Le niveau est enregistré à côté de l'exécutable, dans le dossier `Levels`. Dans l'**éditeur**,
quelle que soit sa taille, un niveau reste **entièrement visible** à l'ouverture (la caméra dézoome
automatiquement si besoin) — c'est le cadrage choisi ci-dessus qui s'applique **en jeu**.

## 4. Publier votre niveau

1. Copiez le fichier de votre niveau (`Levels\<nom>.json`, à côté de `JustAnotherDnDGame.exe`) dans le
   dossier `Source/Elements/Levels/` de votre copie du projet (celle clonée à l'étape 1).
2. Ouvrez **GitHub Desktop** : votre nouveau fichier apparaît dans la liste des changements.
3. En bas à gauche, donnez un court résumé (ex. « Ajout du niveau Foret-1 ») et cliquez
   **Commit to main**.
4. Cliquez **Push origin** (en haut) : votre niveau est envoyé sur GitHub, visible par toute
   l'équipe.

## 5. Récupérer les niveaux des autres

Dans GitHub Desktop, cliquez **Fetch origin** puis **Pull origin** : les niveaux ajoutés par
d'autres membres de l'équipe apparaissent dans votre dossier `Source/Elements/Levels/`, prêts à
être ouverts dans l'éditeur.

## En cas de problème

- **L'enregistrement affiche un message d'erreur** : le niveau n'a pas encore d'entrée et/ou de
  sortie, ou une porte n'est pas correctement reliée — le message précise le problème. Corrigez et
  recommencez **Ctrl+S**.
- **GitHub Desktop signale un conflit** : deux personnes ont modifié le *même* fichier de niveau.
  Donnez des **noms de fichiers différents** à vos niveaux pour éviter ce cas.
