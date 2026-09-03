# LOT-46 — Moteur d'animation générique piloté par données {#lot-46}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40) (*TextureCache*, contrat d'asset),
> [LOT-42](@ref lot-42) (skins), [LOT-45](@ref lot-45) (assets par instance). Prérequis de
> [LOT-47](@ref lot-47) et [LOT-48](@ref lot-48).

## Objectif
Rendre le monde **vivant**. C'est le trou le plus large du cadrage initial du programme : il
prévoyait d'habiller les tuiles et d'assigner des textures par case, mais **aucun lot n'animait quoi
que ce soit**. Une porte texturée serait restée une image fixe en s'ouvrant.

L'animation existe aujourd'hui, mais uniquement pour le personnage et **codée en dur** :
`core::AnimationClip` est un `enum { Idle, Run, Jump }`, les durées d'image sont des constantes de
compilation, et `core::AnimationSystem` ne parcourt que les entités portant `core::Player`. Aucune
tuile, aucun mécanisme, aucun décor ne peut être animé.

Ce lot remplace ce dispositif par un moteur **piloté par données**, sans rien changer au rendu
visible du personnage.

## Périmètre

### Inclus
- **Un clip devient une donnée** : nom, suite d'indices d'images, durée par image, bouclé ou joué une
  fois. `core::AnimationClip` cesse d'être un `enum` d'états figés.
- **`core::Animation`** référence un clip par nom au sein d'un jeu de clips, conserve l'image
  courante et le temps écoulé. Reste une **structure de données pure**, sans méthode ni type GPU,
  conformément au patron des composants ECS.
- **Description d'animation par asset** : fichier `nom-asset.anim.json` **à côté** du PNG, décrivant la
  spritesheet (taille d'image, disposition) et ses clips. Résolu et mis en cache comme tout asset
  (`hmi::AssetPaths`, *TextureCache*), validé par le contrat d'asset (LOT-40). **Un asset sans
  fichier d'animation est affiché comme une image fixe** — c'est le cas par défaut, et il ne produit
  ni erreur ni avertissement.
- **`core::AnimationSystem` généralisé** : parcourt toute entité portant `core::Animation`, plus
  seulement le personnage. Progression au **pas fixe** (`EX-REN-021`, `EX-NFR-002`) — deux parties
  identiques doivent produire exactement les mêmes images.
- **Tuiles animées** : un skin (LOT-42) peut désigner un asset animé — eau, lave, torche. Aucune
  donnée de niveau supplémentaire : c'est une propriété de l'asset, pas de la case.
- **Clips joués une fois** (`OneShot`) avec clip de repli à la fin : le mécanisme dont LOT-47 a
  besoin pour les transitions ouverture/fermeture.
- **Migration des clips du personnage** dans le nouveau format, à comportement **strictement
  identique** : les durées actuelles (repos 0,5 s ; course 0,1 s) et l'ordre des images
  (`flatPlayerFrameIndex`) sont reconduits tels quels. L'habillage du personnage depuis un fichier
  externe relève de LOT-48.

### Exclus (hors périmètre de ce lot)
- Le **branchement** des clips sur l'état logique des mécanismes (LOT-47) et sur celui du personnage
  au-delà de l'existant (LOT-48) : ce lot fournit le moteur, pas les règles métier.
- Interpolation entre images, courbes de temps, fondu enchaîné : progression par images discrètes,
  comme aujourd'hui.
- Machine à états d'animation avec conditions de transition : le choix du clip reste une
  **projection** de l'état de la simulation, comme l'actuel `targetClip()`. Une machine à états
  serait une abstraction non justifiée pour une dizaine de clips.
- Édition des animations dans l'éditeur : le fichier `.anim.json` s'écrit à la main pour ce lot ;
  une UI dédiée relève de LOT-47 (section « Animations » du panneau) pour l'affectation, et de
  LOT-54 pour la création des images.

## Décisions de cadrage
- **Données plutôt que code** : la seule autre option — ajouter des valeurs à l'`enum` existant à
  chaque nouvel objet animé — ferait de `Core` un catalogue d'apparences, en contradiction directe
  avec `EX-ARCH-012` (la simulation ignore le rendu).
- **Description à côté de l'asset, pas dans le niveau ni dans `skins.json`** : une animation est une
  propriété **de l'image**, pas du niveau qui l'utilise ni de l'association type → fichier. Un même
  asset animé assigné à dix cases n'a qu'une description.
- **Pas de fichier d'animation = image fixe** : le cas par défaut ne doit rien coûter à l'auteur, et
  ne doit surtout pas produire d'avertissement — à la différence d'un asset **référencé mais
  introuvable**, qui reste une anomalie (`EX-NFR-040`).
- **Progression au pas fixe, pas au temps réel** : l'animation reste dans la simulation. Le
  déterminisme (`EX-NFR-002`) prime sur la fluidité à cadence variable ; l'interpolation de rendu
  existante (`hmi::PreviousPosition`) traite déjà le découplage.
- **Aucune régression visible sur le personnage** : ce lot est une refonte d'infrastructure ; ses
  critères d'acceptation l'imposent explicitement.

## Exigences couvertes
- Nouvelle : `EX-REN-005` (animations décrites par des données, applicables à toute entité,
  progression au pas fixe).
- Amendée : `EX-REN-012` (animations par séquence d'images, généralisées).
- Réutilisées : `EX-REN-021`/`EX-NFR-002` (pas fixe, déterminisme), `EX-ARCH-012` (rendu sans effet
  sur la simulation), `EX-REN-007` (contrat d'asset), `EX-NFR-040` (repli), `EX-NFR-010`/`EX-NFR-020`
  (testabilité sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-clip.md) | Modèle de clip et de jeu de clips (données pures) + `core::Animation` révisé | `Source/Core/Ecs` | ✅ |
| [TACHE-02](tache-02-systeme-generalise.md) | `AnimationSystem` généralisé à toute entité animée, clips bouclés et joués une fois | `Source/Core/Ecs/Systems` | ✅ |
| [TACHE-03](tache-03-format-anim-json.md) | Format `nom-asset.anim.json` : lecture, validation, mise en cache, repli image fixe | `Source/HMI/Graphics` | ✅ |
| [TACHE-04](tache-04-migration-personnage.md) | Migration des clips du personnage, non-régression | `Source/Core`, `Source/HMI/Graphics`, `Source/Test` | ✅ |
| [TACHE-05](tache-05-tuiles-animees.md) | Skins de tuiles animés (eau, lave, torche) | `Source/HMI/Graphics` | ✅ |

## Critères d'acceptation du lot
1. Le personnage s'anime **exactement** comme avant le lot : mêmes images, mêmes durées, même ordre —
   asserté par test, pas constaté à l'œil.
2. Un asset accompagné d'un `nom-asset.anim.json` valide s'anime, qu'il soit un skin de tuile ou une
   texture assignée à une case.
3. Un asset **sans** fichier d'animation s'affiche en image fixe, sans erreur ni avertissement.
4. Un clip joué une fois se termine sur son clip de repli, sans boucler.
5. Deux exécutions de la même séquence d'entrées produisent la même suite d'images (déterminisme).
6. Le modèle de clip, la progression et la lecture du format sont testés sans GPU ; build
   `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (*TextureCache*, contrat d'asset), [LOT-42](@ref lot-42) (skins) et
[LOT-45](@ref lot-45) (assets par instance). Remplace le dispositif d'animation figé de
[LOT-18](@ref lot-18). Prérequis de [LOT-47](@ref lot-47) (états des mécanismes),
[LOT-48](@ref lot-48) (personnage) et bénéfique à [LOT-53](@ref lot-53) (effets).

## Navigation des tâches
- @subpage lot-46-tache-01-modele-clip
- @subpage lot-46-tache-02-systeme-generalise
- @subpage lot-46-tache-03-format-anim-json
- @subpage lot-46-tache-04-migration-personnage
- @subpage lot-46-tache-05-tuiles-animees
