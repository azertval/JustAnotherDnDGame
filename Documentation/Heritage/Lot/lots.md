# Lots {#lots}

Le travail est découpé en **lots** (un incrément livrable par lot), chacun dans
un sous-dossier `LOT-XX-nom/` contenant un `epic.md` (objectif, périmètre,
critères d'acceptation) et des fichiers `tache-NN.md` (une unité de travail
chacun). Les lots référencent les [spécifications](@ref specifications) via les
identifiants d'exigences `EX-…`.

Contrairement aux spécifications, les lots **conservent** leur numéro (`LOT-XX`) :
c'est un identifiant stable, jamais réordonné.

## Lots

- @subpage lot-01
- @subpage lot-02
- @subpage lot-03
- @subpage lot-04
- @subpage lot-05
- @subpage lot-06
- @subpage lot-07
- @subpage lot-08
- @subpage lot-09
- @subpage lot-10
- @subpage lot-11
- @subpage lot-12
- @subpage lot-13
- @subpage lot-14
- @subpage lot-15
- @subpage lot-16
- @subpage lot-17
- @subpage lot-18
- @subpage lot-19
- @subpage lot-20
- @subpage lot-21
- @subpage lot-22
- @subpage lot-23
- @subpage lot-24
- @subpage lot-25
- @subpage lot-26
- @subpage lot-27
- @subpage lot-28
- @subpage lot-29
- @subpage lot-30
- @subpage lot-31
- @subpage lot-32
- @subpage lot-33
- @subpage lot-34
- @subpage lot-35
- @subpage lot-36
- @subpage lot-37
- @subpage lot-38
- @subpage lot-39
- @subpage lot-40
- @subpage lot-41
- @subpage lot-42
- @subpage lot-43
- @subpage lot-44
- @subpage lot-45
- @subpage lot-46
- @subpage lot-47
- @subpage lot-48
- @subpage lot-49
- @subpage lot-50
- @subpage lot-51
- @subpage lot-52
- @subpage lot-53
- @subpage lot-54
- @subpage lot-55
- @subpage lot-56
- @subpage lot-57
- @subpage lot-58
- @subpage lot-59
- @subpage lot-60
- @subpage lot-61
- @subpage lot-62
- @subpage lot-63
- @subpage lot-64
- @subpage lot-65
- @subpage lot-66
- @subpage lot-67
- @subpage lot-69
- @subpage lot-70
- @subpage lot-72
- @subpage lot-73
- @subpage lot-74

## Apres le programme `0.1.0`

Le [LOT-74](@ref lot-74) — **livré** — donne au level design des blocs qui ne
survivent pas au passage du personnage : un bloc **descendant** que tout contact arme et qui emporte
ce qui repose dessus (`EX-GP-027`), un bloc **fragile** que brise un ground pound (`EX-GP-028`), un
bloc **éphémère** qui s'efface une fois qu'on l'a quitté (`EX-GP-029`). Il ne répond pas à un
programme cadré mais à un manque que le dépôt avait **déjà consigné** : le `LOT-72` a livré le ground
pound en excluant explicitement la casse de blocs fragiles, faute d'un `TileType` à casser — le geste
existe donc depuis lors sans cible. Les trois blocs sont cadrés ensemble parce qu'ils partagent
**exactement** la même chaîne de huit fichiers (type de tuile, chargeur, éditeur, rendu, contrôleur,
contenu) pour trois comportements de quelques dizaines de lignes ; les séparer l'aurait payée trois
fois. Le pari technique est qu'**aucune passe de physique nouvelle** n'est nécessaire : le portage
d'un corps à position continue existe (`EX-GP-026`, plateformes mobiles) et le retrait de matière de
la grille de collision aussi (portes et clés) — le lot réutilise les deux plutôt que d'ouvrir un
troisième chemin de collision. Il solde au passage une dette consignée dans le code lui-même : la
borne « dernier énumérateur de `TileType` », recopiée à la main dans trois fichiers, devient une
sentinelle unique — elle s'y trouvait en réalité **quatre** fois, la quatrième n'ayant été révélée
que par l'exécution des tests. Deux conséquences sont assumées et documentées : la disparition d'un
bloc est **définitive** jusqu'au rechargement, et l'observation de l'IA passe de 33 à 36 canaux, ce
qui **invalide les modèles entraînés** avant le lot (leur chargement est refusé sans conversion
silencieuse, comportement qui existait déjà et que le lot se contente de tester). Le lot met enfin
au jour deux dettes qu'il consigne sans les corriger : l'ordre de résolution du pas fixe est réécrit
dans **quatre** orchestrations indépendantes, dont seules deux sont comparées automatiquement — et
l'une d'elles n'appliquait pas les capacités de tableau (`EX-GP-055`), écart corrigé ici parce que
`demo-bloc-fragile` est le premier tableau livré à en déclarer une.

Le [LOT-73](@ref lot-73) — **livre** — reprend l'interface la ou trois symptomes rapportes a l'usage
la rendaient penible : une fenetre qui reclamait plus grand que l'ecran, un gel de plusieurs secondes
a chaque changement d'ecran, un mode IA peu lisible et incomplet. Les trois remontaient a deux
defauts de **portee**. D'abord, aucune regle ne disait *qui decide de la taille de la fenetre* : la
reponse etait, de fait, le plus dense des ecrans, dont la taille minimale etait de surcroit
multipliee par un facteur derive de la hauteur de cette meme fenetre — une boucle sans point fixe
(`EX-IHM-080`/`EX-IHM-081`). Ensuite, le facteur d'agrandissement, qui ne concerne que les ecrans du
jeu, vivait dans la feuille de style de l'**application** : en changer repolissait ses 862 widgets,
cinq secondes durant en Debug (`EX-IHM-082`). Le lot les traite en **invariants** portes par le
chemin d'ajout commun des ecrans et par deux feuilles de style disjointes, plutot qu'en correctifs
ecran par ecran — le defaut de taille s'etait deja produit deux fois, et avait ete corrige deux fois
localement. S'y ajoute la fermeture d'un ecart de perimetre du mode IA : neuf reglages y etaient lus
puis jetes avant d'atteindre le moteur, pendant que le `config.json` du run affirmait le contraire
(`EX-IHM-083`).

Le [LOT-72](@ref lot-72) — **livré** — enrichit le nuancier de mouvement du personnage en faisant
composer le dash (`LOT-10`, `EX-GP-017`) avec les autres systèmes plutôt que de le laisser isolé :
dash **chargé** (bouton de dash et direction opposée maintenus, `EX-GP-056`), poussée renforcée d'un
bloc par un dash **boosté** (`EX-GP-057`), ground pound (`EX-GP-058`), et un combo dash + saut
(jump-cancel d'un dash boosté, wall-jump en sortie, momentum hérité d'une poussée, bonus plafonné,
`EX-GP-061`). Relecture du code avant implémentation : le dash suivait déjà les pentes et le wall
slide était déjà livré (`EX-GP-016`, LOT-10) — `EX-GP-060` documente et teste ces deux points plutôt
que d'y ajouter du code. Aucune nouvelle touche : `dashHeld` (garde de la charge) dérive la touche de
dash déjà mappée, comme `jumpHeld` pour le saut. Poussée renforcée et jump-cancel sont délibérément
restreints au dash **boosté**, jamais à un dash normal — une première version sans cette restriction
avait cassé la séquence `demo-final` et les tests d'entraînement IA, corrigée avant livraison.

Le `LOT-71` — **livre** — retrace `demo-final`, qui fermait la sequence sur un enchainement de
salles sans densite : un gaufre unique de 24x24 y reunit desormais deux cles, un interrupteur a
bascule, un bloc sur plaque, un ascenseur a plateformes synchronisees, trois puits a wall jump et
une cheminee. Il porte la parallaxe a **cinq** plans, la ou le `LOT-70` en avait pose trois. Deux
dangers en ont ete **retires** : le tableau etait invincible, et ca se demontre — l'un ne laissait
qu'une fenetre d'esquive d'une seule image, l'autre balayait exactement le couloir du bas sans
qu'aucune allure ne permette de le doubler. Le parcours scripte du garde-fou systeme est reecrit en
25 phases documentees, et son plafond de pas porte de 3 000 a 9 000 — une borne de terminaison, pas
une mesure de difficulte.

Comme le `LOT-68`, et contrairement aux `LOT-69`/`LOT-70`, il n'a **pas de dossier de lot dedie** :
il ne cadre aucun systeme nouveau. Ce qu'il change vit dans les fichiers de niveau eux-memes, dans
`Source/Test/Systeme/ScriptedLevelSequence.h` et dans le `CHANGELOG.md` — un dossier de lot n'y
aurait rien ajoute que de la redite.

Le [LOT-70](@ref lot-70) — **livre** — repond au manque explicitement consigne par le `LOT-69`
TACHE-10 : la migration des plans picturaux n'avait livre qu'un report fidele de l'ancien habillage,
jamais une fresque exploitant reellement la profondeur. Il ajoute un troisieme plan, lointain, aux
deux seuls tableaux ou la parallaxe est active (`demo-mouvement`, `demo-final`) — les vingt autres,
neutralises par leur cadrage `WholeLevel`, restent inchanges. Comme le `LOT-67`, un dossier de lot
dedie malgre sa taille modeste, faute d'un programme qui l'aurait deja prevu.

Le [LOT-69](@ref lot-69) — **livre** — est le premier lot d'ampleur d'apres-programme, et le premier
a **retirer** un systeme livre plutot qu'a en ajouter un : les decors-sprites du `LOT-49`/`LOT-50`
cedent la place a des **plans picturaux** peints dans l'editeur, avec parallaxe reglable.
Contrairement au `LOT-68`, il a un **dossier de lot dedie** : sa surface (dix taches, dont
le portage du rendu sur QRhi et le retrait d'un sous-systeme entier) le justifie amplement.

Le `LOT-68` poursuit dans la meme veine : refonte de l'interface, en deux volets — degraissage
des surfaces de commande de l'editeur et identite pixel art des ecrans du jeu (`EX-IHM-070` a
`EX-IHM-074`). Comme le `LOT-67`, il repond a un manque constate a l'usage plutot qu'a un programme
cadre ; il est documente par ses exigences et les guides, sans dossier de lot dedie.

Le [LOT-67](@ref lot-67) ouvre la suite : il ne fait partie d'aucun programme cadre, et repond a un
manque constate a l'usage de l'editeur — les trajectoires des elements mobiles et les regles de
mobilite d'un tableau n'etaient editables qu'en modifiant le JSON a la main.

## Programme `0.1.0`

Les lots `LOT-58` à `LOT-66`, avec le `LOT-53` (cadré de longue date et resté non commencé),
forment le programme de la version **`0.1.0`** : le passage d'un moteur complet à un **jeu**
distribuable. Deux familles s'y répondent — la **complétude produit** (boucle de jeu, son, effets,
mécanismes manquants, cadrage de caméra, refonte des niveaux) et le **durcissement d'ingénierie**
(vérification en Release, sanitizer, analyse statique, diagnostics d'une version publiée, budget de
rendu mesuré).

Ces lots sont numérotés **dans leur ordre d'exécution**, ce que la règle générale ci-dessus permet
puisqu'aucun n'était encore livré au moment de leur cadrage. Seul le `LOT-53`, cadré de longue date
et déjà publié sous ce numéro, conserve le sien et s'exécute entre le `LOT-60` et le `LOT-61` :
c'est précisément le cas que la règle protège.

| Rang | Lot | Pourquoi à cette place |
|:----:|-----|------------------------|
| 1 | [LOT-58](@ref lot-58) | Le durcissement précède le contenu qu'il doit protéger. |
| 2 | [LOT-59](@ref lot-59) | Tous les autres lots produit se voient à travers ses écrans. |
| 3 | [LOT-60](@ref lot-60) | Le son a besoin d'un écran de fin de niveau où exister. |
| 4 | [LOT-53](@ref lot-53) | Réutilise les déclencheurs d'événements posés par le `LOT-60`. |
| 5 | [LOT-61](@ref lot-61) | Indépendant ; requis avant qu'un tiers n'exécute le jeu. |
| 6 | [LOT-62](@ref lot-62) | Mesure le budget une fois tous les émetteurs livrés. |
| 7 | [LOT-63](@ref lot-63) | **Découpable** : le lot qu'on rogne si le calendrier se tend. |
| 8 | [LOT-64](@ref lot-64) | Le cadrage doit exister avant qu'on refasse les niveaux. |
| 9 | [LOT-65](@ref lot-65) | Dernier lot de contenu : exploite tout ce qui précède. |
| 10 | [LOT-66](@ref lot-66) | Clôt le programme ; les statuts ne se figent qu'à la fin. |

## Programme annexe

Un second découpage, indépendant de celui-ci, porte l'IA de résolution autonome : @ref lots-annexe.
Sa numérotation (`LOT-ANNEXE-NN`) ne croise jamais celle des lots principaux.
