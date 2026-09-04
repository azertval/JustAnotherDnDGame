# Plans picturaux & pipeline pixel art {#spec-decors}

> Statut : **livré** (`LOT-H-69`). Le système de **décors-sprites** (`LOT-H-49`/`LOT-H-50`) est
> **retiré** et remplacé par des **plans picturaux** : un décor n'est plus un objet posé, c'est une
> surface peinte à l'échelle du niveau. Les exigences retirées sont conservées en fin de page
> (section [Exigences retirées](#dec-retirees)), leur texte intact, pour que les documents des lots
> déjà livrés qui s'y réfèrent restent lisibles. Le **pipeline photo → pixel art** (section 3) reste
> **post-MVP**.

## Vision
Le décor d'un niveau est **peint**, plan par plan, à l'intérieur même de l'éditeur, et se superpose
en profondeur pour donner de la **parallaxe 2D**. Concrétise l'objectif produit `EX-VIS-007`
(`vision.md`).

Ce que le `LOT-H-69` corrige : composer un habillage en posant des images ne permet pas de *dessiner*
un décor. Trois couches figées, des facteurs de défilement codés en dur et aucun outil de peinture
à l'échelle du niveau — le level designer devait sortir de l'outil pour obtenir un fond, puis le
découper en objets.

## 1. Plans

- \anchor EX-DEC-040 **EX-DEC-040** — Un **plan** est une image couvrant le **niveau entier**,
  stockée dans son **propre fichier PNG** à côté du niveau et référencée par nom. Un niveau porte
  une **liste ordonnée** de plans, de nombre **libre** ; l'**ordre est significatif** : il détermine
  la superposition. Le rendu reste **net** (`EX-DEC-003`).
- \anchor EX-DEC-041 **EX-DEC-041** — Chaque plan porte sa **densité** en pixels par unité monde
  (**16**, **8** ou **4** ; 16 = résolution native, `hmi::Camera2D::PIXELS_PER_UNIT`). Un plan
  lointain n'a pas besoin de la densité native : à 4 px/unité il coûte **seize fois moins** de
  mémoire. C'est ce réglage, et non le nombre de plans, qui décide de la viabilité mémoire d'un
  niveau (`EX-NFR-043`).
- \anchor EX-DEC-042 **EX-DEC-042** — Chaque plan est placé **derrière** les tuiles physiques ou
  **devant** le personnage — deux positions, pas trois : les trois couches historiques
  n'exprimaient déjà que ces deux intentions. Un plan de **devant** est le moyen de lecture
  immédiate qui distingue le décor traversable du décor physique (cf. `EX-REN-014`). Les plans se
  composent dans l'**ordonnancement unique** du rendu, sans en créer de concurrent (`EX-REN-049`).
- \anchor EX-DEC-043 **EX-DEC-043** — Chaque plan porte un **facteur de défilement** (parallaxe)
  **par axe**, appliqué au rendu et **purement visuel** (`EX-ARCH-012`). Le niveau décide **si** la
  parallaxe s'applique. La parallaxe est une **translation** : la taille du plan n'est jamais mise
  à l'échelle, sous peine de casser le 1:1 pixel (`EX-ARCH-022`). La translation est **bornée** de
  sorte que le plan couvre toujours le cadrage — sans quoi une bande vide apparaîtrait au bord.
- \anchor EX-DEC-044 **EX-DEC-044** — Le chargement **borne le coût** d'un niveau : nombre de plans
  plafonné, dimension de texture refusée au-delà de la limite du matériel, et **avertissement
  journalisé** quand le total dépasse le seuil raisonnable. Le garde-fou est dans le **format**, pas
  laissé à l'usage : rien n'empêcherait autrement seize plans à densité native sur un grand niveau.
- \anchor EX-DEC-003 **EX-DEC-003** — Rendu **pixel art net** (nearest-neighbor) — cf.
  `EX-ARCH-022`. Inchangée depuis `LOT-H-49` : elle vaut pour les plans comme elle valait pour les
  décors.

Les plans sont **traversables** : ils ne participent jamais aux collisions. Un plan est un décor,
jamais une géométrie de jeu.

## 2. Édition

- \anchor EX-DEC-045 **EX-DEC-045** — Les plans se peignent **dans l'éditeur**, au pixel près et à
  l'échelle du niveau, avec les tuiles physiques visibles en référence. Concrétisé par
  `EX-EDIT-046` (mode création) et `EX-EDIT-047` (panneau « Plans »).

## 3. Pipeline photo → pixel art (intégré à l'éditeur)
- \anchor EX-DEC-030 **EX-DEC-030** — L'éditeur permet de **charger une photo** et de la **convertir
  en pixel art** (pixellisation, réduction de palette) — traitement **intégré à l'outil**.
- \anchor EX-DEC-031 **EX-DEC-031** — Les **paramètres de conversion** sont ajustables (taille de
  pixel / résolution cible, palette). **Post-MVP**, non planifié.
- \anchor EX-DEC-032 **EX-DEC-032** — L'image pixel art résultante est enregistrée comme fichier et
  **référencée par un plan** (`EX-DEC-040`), ou collée dans un plan depuis l'atelier.

> Dépendance déjà levée : le chargement et l'encodage d'images existent depuis `LOT-H-39`/`LOT-H-54`
> (`hmi::decodeImageFile`, `hmi::encodeImageFile`).

## 4. Périmètre & séquencement
- Le `LOT-H-69` livre les plans, leur parallaxe et le mode création. Le pipeline photo (section 3)
  reste **post-MVP**.
- **Perte assumée du remplacement net** : il n'existe plus d'objet décoratif **ponctuel
  réutilisable**. Un motif réutilisé dans dix niveaux doit être repeint (ou collé) dans chaque plan.
  C'est le prix du choix d'un décor peint plutôt que composé, acté au cadrage du `LOT-H-69`.

## Exigences retirées {#dec-retirees}

> Retirées par le `LOT-H-69`, qui remplace les décors-sprites par des plans picturaux. Les ancres sont
> **conservées** — jamais renumérotées, jamais supprimées : les dossiers `LOT-H-49`, `LOT-H-50`,
> `LOT-H-51` et le `CHANGELOG` s'y réfèrent, et réécrire un lot livré falsifierait son histoire
> (règle de [`lots.md`](@ref lots)). Le texte ci-dessous est celui d'origine ; il décrit ce qui **a
> été** livré, pas ce qui est attendu aujourd'hui.

- \anchor EX-DEC-001 **EX-DEC-001** *(retirée en `LOT-H-69`, remplacée par `EX-DEC-040`)* — Un décor
  est un **objet libre** (non calé sur la grille de tuiles) doté d'un **transform** (position,
  échelle, rotation optionnelle) en unités monde.
- \anchor EX-DEC-002 **EX-DEC-002** *(retirée en `LOT-H-69`, remplacée par `EX-DEC-042`)* — Les décors
  se superposent par **couches** (arrière-plan, décor, premier plan).
- \anchor EX-DEC-004 **EX-DEC-004** *(retirée en `LOT-H-69`)* — Les décors sont des **entités ECS de
  la simulation `Core`**. Motif du retrait : un plan est une donnée d'habillage du niveau, pas une
  entité simulée — il n'a ni transform manipulable ni comportement.
- \anchor EX-DEC-005 **EX-DEC-005** *(retirée en `LOT-H-69`)* — Chaque décor porte une propriété
  **statique** ou **manipulable en jeu**. Motif : le drapeau n'a jamais eu d'effet, et la mécanique
  qu'il préparait (`EX-DEC-020`) est elle-même retirée.
- \anchor EX-DEC-006 **EX-DEC-006** *(retirée en `LOT-H-69`, remplacée par `EX-DEC-043`)* — Chaque
  **couche** de décor porte un **facteur de défilement** (parallaxe) appliqué au rendu. Le facteur
  était une constante par couche ; il est désormais porté par le plan et réglable.
- \anchor EX-DEC-010 **EX-DEC-010** *(retirée en `LOT-H-69`, remplacée par `EX-DEC-045`)* — L'éditeur
  permet de **placer, déplacer, redimensionner, superposer et supprimer** des décors.
- \anchor EX-DEC-020 **EX-DEC-020** *(retirée en `LOT-H-69`)* — Le joueur peut **manipuler en temps
  réel** les décors marqués manipulables. Motif : la mécanique n'a jamais été livrée et n'a plus
  d'objet — on ne déplace pas une fresque peinte.
- \anchor EX-DEC-021 **EX-DEC-021** *(retirée en `LOT-H-69`)* — La manipulation en jeu reste
  **déterministe**. Retirée avec `EX-DEC-020`, dont elle était le corollaire.

## Traçabilité
S'appuie sur [`architecture.md`](architecture.md), [`rendu-technique.md`](rendu-technique.md)
(ordonnancement des calques, `EX-REN-049`), [`niveaux.md`](niveaux.md) (format, `EX-LVL-009`),
[`editeur-niveaux.md`](editeur-niveaux.md) (mode création, `EX-EDIT-046`/`EX-EDIT-047`) et
[`vision.md`](vision.md).
