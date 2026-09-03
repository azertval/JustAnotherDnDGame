# LOT-53 — Effets et particules {#lot-53}

> Statut : **fait**. Prérequis : [LOT-46](@ref lot-46) (moteur d'animation),
> [LOT-48](@ref lot-48) (personnage texturé).

## Objectif
Donner un retour visuel aux mouvements du personnage.

Le ressenti de déplacement a été travaillé sur quatre lots — temps de grâce au bord d'une plateforme
et mémorisation du saut (`LOT-09`), double saut, saut mural et dash (`LOT-10`), gravité asymétrique,
suspension à l'apex et chute rapide (`LOT-11`) — et **rien de tout cela n'est visible**. Un dash se
distingue d'une course uniquement par la vitesse ; un atterrissage après une longue chute est
identique à un pas.

Une fois le personnage et le décor texturés, l'absence d'effets devient la principale différence
entre le rendu du jeu et celui d'un jeu fini.

## Périmètre

### Inclus
- **Émetteur de particules dans `Core`**, simulé au **pas fixe** et **déterministe** (`EX-NFR-002`) :
  toute source d'aléa est une suite pseudo-aléatoire à graine explicite, jamais l'horloge. Deux
  parties identiques produisent les mêmes effets.
- **Effets livrés**, tous déclenchés par des transitions d'état déjà exposées par `core::Player` :
  traînée de dash, bouffée de poussière à l'atterrissage, éclatement à la mort, brève secousse
  d'écran à l'atterrissage lourd et à la mort.
- **Budget borné** : nombre maximal de particules simultanées, constante nommée ; au-delà, les plus
  anciennes sont recyclées. Cohérent avec `EX-NFR-005` (volume de primitives observable et borné).
- **Rendu** sur les calques *Object* et *Foreground* selon l'effet, en mode Texture uniquement.
- La secousse d'écran agit sur la **caméra de rendu** uniquement, jamais sur la position simulée.

### Exclus (hors périmètre de ce lot)
- Éditeur de particules, paramétrage par le level designer : effets définis en dur, réglés par
  constantes nommées.
- Effets attachés aux mécanismes ou aux décors : le personnage d'abord.
- Éclairage, lueurs, post-traitement : hors du pipeline de quads, hors sujet.
- Effets sonores.

## Décisions de cadrage
- **Dans `Core`, au pas fixe, déterministe.** Une simulation de particules au temps réel dans `HMI`
  serait plus simple à écrire, mais casserait le déterminisme que tout le projet tient depuis
  `LOT-01` (`EX-NFR-002`) et rendrait les rejeux non reproductibles. Le coût est faible : quelques
  centaines de particules par image.
- **Aucun effet sur le gameplay** (`EX-ARCH-012`) : les particules ne collisionnent pas, ne
  déclenchent rien, et la secousse d'écran ne déplace que la caméra. Le joueur ne doit jamais perdre
  une partie à cause d'un effet.
- **Secousse d'écran discrète et bornée** : un effet trop marqué nuit à la lisibilité d'un jeu de
  plateforme précis. Amplitude et durée sont des constantes conservatrices.
- **Budget explicite plutôt qu'implicite** : après six calques ajoutés, un système capable d'émettre
  sans borne est le premier candidat à faire chuter la cadence.

## Exigences couvertes
- Nouvelle : `EX-REN-008` (effets visuels de courte durée, déterministes, sans effet sur le
  gameplay, dans un budget borné).
- Réutilisées : `EX-NFR-002` (déterminisme au pas fixe), `EX-REN-021` (pas fixe), `EX-ARCH-012`
  (rendu sans effet sur la simulation), `EX-NFR-005` (budget de primitives), `EX-REN-043` (calques),
  `EX-GP-017` (dash), `EX-GP-016` (saut mural).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-emetteur-deterministe.md) | Émetteur et simulation de particules au pas fixe, déterministes, budget borné | `Source/Core/Ecs` | ✅ |
| [TACHE-02](tache-02-declencheurs.md) | Déclencheurs depuis les transitions d'état du personnage (dash, atterrissage, mort) | `Source/Core/Ecs/Systems`, `Source/HMI/Game` | ✅ |
| [TACHE-03](tache-03-rendu-secousse.md) | Rendu des particules sur les calques *Object*/*Foreground* + secousse d'écran | `Source/HMI/Graphics` | ✅ |

## Critères d'acceptation du lot
1. Un dash, un atterrissage et une mort produisent chacun un effet visuel distinct, en mode Texture.
2. Deux exécutions de la même séquence d'entrées produisent **exactement** les mêmes effets
   (déterminisme asserté par test).
3. Le nombre de particules simultanées ne dépasse jamais le budget, quelle que soit l'intensité des
   déclenchements.
4. Aucun effet sur la simulation : les tests de gameplay et de franchissabilité passent sans
   modification, et la secousse d'écran ne modifie aucune position simulée.
5. La simulation de particules est testée sans GPU ; build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-46](@ref lot-46) (animations par données) et [LOT-48](@ref lot-48) (états du
personnage exploités). Lit l'état de [LOT-10](@ref lot-10) et [LOT-11](@ref lot-11) sans le
modifier. Respecte le budget posé par [LOT-40](@ref lot-40).

## Navigation des tâches
- @subpage lot-53-tache-01-emetteur-deterministe
- @subpage lot-53-tache-02-declencheurs
- @subpage lot-53-tache-03-rendu-secousse
