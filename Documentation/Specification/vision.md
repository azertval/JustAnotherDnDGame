# Vision & périmètre {#spec-vision}

> Statut : **cadré** (`LOT-01`). Cette page remplace la vision du jeu de plateforme dont ce dépôt
> est dérivé (voir §Origine). Les objectifs ci-dessous sont **à concrétiser** : aucun n'est encore
> livré, le programme de lots (@ref lots) dit lequel les porte.

## Concept

**RPG 2D en vue de dessus**, à monde de cartes connectées. Le joueur dirige un personnage qui
explore en **temps réel** (déplacement libre 8 directions, interaction avec les PNJ, les coffres
et les mécanismes) et affronte les rencontres en **combat tactique au tour par tour** sur la grille
de la carte, régi par un système **d20**.

- **Genre** : action-RPG d'exploration + combat tactique.
- **Perspective** : 2D, vue de dessus, décor en tuiles multi-couches (sol / décor / collision).
- **Session type** : progression continue dans un monde persistant, sauvegardée.
- **Public** : joueurs appréciant l'exploration et la réflexion tactique.
- **Plateforme** : Windows (bureau), rendu Qt QRhi (Direct3D 11).

### Décisions de cadrage

Quatre décisions structurantes, actées avant le `LOT-01` et non réouvertes sans arbitrage
explicite :

1. **Règles d20 maison**, structurellement compatibles avec le SRD (six caractéristiques,
   modificateur `(score-10)/2`, jet d20 contre difficulté ou classe d'armure, avantage/désavantage)
   mais **sans en dépendre** : classes et sorts propres, définis en JSON. On garde la familiarité
   sans la charge d'implémentation ni l'obligation d'attribution.
2. **Combat sur la carte d'exploration**, jamais en arène séparée. La grille de combat est
   **dérivée de la couche collision** du niveau. Conséquence de level design à tenir dès la
   première carte : *toute carte doit être un terrain tactique valide* — largeur suffisante,
   obstacles cohérents.
3. **Un héros au départ, quatre à terme.** Rien ne doit supposer l'unicité du personnage : la
   fiche est un objet autonome, l'ordre d'initiative est multi-alliés, la sauvegarde stocke une
   *liste*. Le passage au groupe est un lot d'ajout, jamais une refonte.
4. **Échelle : 1 case = 1,5 m** (5 ft), avec `PIXELS_PER_UNIT = 16` inchangé. Fixe portées,
   vitesses et gabarits d'effet.

## Boucle de gameplay

1. Le joueur explore une carte : déplacement libre, interaction avec le décor et les PNJ.
2. Il progresse dans le monde par des portails entre cartes, guidé par les dialogues et les quêtes.
3. Une rencontre se déclenche (contact, zone, dialogue) : le monde se fige, la grille tactique se
   monte sur la carte courante.
4. Le combat se joue au tour par tour, dans l'ordre d'initiative, chaque action résolue au d20.
5. Victoire : retour à l'exploration, l'ennemi retiré de la carte durablement, butin et expérience
   acquis. Défaite : reprise à la dernière sauvegarde.

## Objectifs (*vertical slice*)

- \anchor EX-VIS-001 **EX-VIS-001** — Le jeu doit proposer un personnage jouable se déplaçant
  librement en 8 directions sur une carte en tuiles vue de dessus.
- \anchor EX-VIS-002 **EX-VIS-002** — Le jeu doit relier plusieurs cartes par des portails, avec
  retour possible au point de départ.
- \anchor EX-VIS-003 **EX-VIS-003** — Le jeu doit permettre de dialoguer avec un PNJ, dialogue à
  choix et conditions.
- \anchor EX-VIS-004 **EX-VIS-004** — Le jeu doit résoudre un combat tactique complet au tour par
  tour : initiative, déplacement à portée, attaque au d20 contre une classe d'armure, fin de
  rencontre.
- \anchor EX-VIS-005 **EX-VIS-005** — Toute résolution chiffrée doit être **déterministe à graine
  fixée** : un combat rejoué produit exactement les mêmes jets (`EX-NFR-002`).

## Objectifs produit (au-delà du moteur)

- \anchor EX-VIS-006 **EX-VIS-006** — Le projet doit fournir un **éditeur de cartes** permettant à
  des membres non-développeurs de créer du contenu sans coder : couches, entités, portails. Hérité
  de l'éditeur du dépôt d'origine, retargeté par le `LOT-11`.
- \anchor EX-VIS-007 **EX-VIS-007** — Toute règle chiffrée (classes, sorts, objets, ennemis) doit
  être **définie en données** (JSON), jamais codée en dur dans le C++ : c'est ce qui rend
  l'équilibrage possible sans recompiler.

## Hors périmètre (*vertical slice*)

- Multijoueur, réseau.
- Groupe de plusieurs personnages jouables (prévu, mais **après** le slice — cf. décision 3).
- Génération procédurale de cartes ou de donjons.
- Édition collaborative en temps réel dans l'éditeur.
- Portabilité hors Windows.

## Origine

Ce dépôt est dérivé de `ProjectGaming`, jeu 2D de plateforme et puzzle en vue de côté livré en
`0.1.3` après 74 lots. Le `LOT-01` en a conservé le moteur — ECS, boucle à pas fixe, mathématiques
déterministes, balayage AABB, modèle et chargeur de niveaux, rendu, éditeur, IHM Qt — et retiré
tout le gameplay propre à la vue de côté. La vision du platformer et son programme de lots restent
consultables en lecture seule sous `Documentation/Heritage/`.

## Traçabilité

Ces objectifs sont détaillés dans [`gameplay.md`](gameplay.md), [`controles.md`](controles.md),
[`rendu-technique.md`](rendu-technique.md), [`niveaux.md`](niveaux.md),
[`exigences-non-fonctionnelles.md`](exigences-non-fonctionnelles.md),
[`editeur-niveaux.md`](editeur-niveaux.md) et [`architecture.md`](architecture.md). Les
spécifications propres au RPG (`exploration.md`, `regles-dnd.md`, `combat.md`, `rpg.md`) sont
créées par les lots qui les concrétisent. Chaque lot de `../Lot/` référence les exigences `EX-…`
qu'il couvre.
