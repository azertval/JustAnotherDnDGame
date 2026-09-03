# LOT-70 — Parallaxe à trois profondeurs des tableaux qui défilent {#lot-70}

> Statut : **fait** (vérification automatisée : `ctest` à 100 % (1161 tests), build ninja, les
> quatre linters — exigences, cahier de test, séquence démo, régénération des plans — verts). La
> vérification visuelle du rendu réel (Direct3D 11/QRhi) reste manuelle, l'exécutable éditeur
> n'ayant pas pu être construit dans cet environnement (modules Qt `GuiPrivate`/`ShaderTools`
> absents, indépendant de ce lot). Prérequis : [LOT-69](@ref lot-69) (plans picturaux, parallaxe portée par
> le plan). Ne fait partie d'aucun programme cadré : répond au manque explicitement consigné par le
> `LOT-69` TACHE-10 (« un fond peint qui reporte fidèlement l'ancien habillage n'est pas une fresque
> qui exploiterait vraiment la profondeur — dessiner un tel tableau est un acte de level design, hors
> de ce lot d'outillage »). Même situation que [LOT-67](@ref lot-67)/`LOT-68` : un dossier dédié,
> faute d'un programme qui l'aurait déjà prévu.

## Objectif

Faire dire à la parallaxe portée par `core::Plane` (`EX-DEC-043`) ce qu'elle promet, dans les
tableaux où elle s'applique réellement.

Le constat qui déclenche ce lot : `hmi::planeParallaxActive` (`Source/HMI/Graphics/Parallax.h`)
n'active le décalage qu'en cadrage `Follow` ou `PerRoom` — deux tableaux du dépôt seulement,
`demo-mouvement` (suivi continu) et `demo-final` (caméra par salle). Les vingt PNG restants sont
tous à densité fixe et invisibles à l'usage puisque le cadrage `WholeLevel` neutralise le décalage
(comportement voulu, documenté dans `Parallax.h`). Sur les deux tableaux où ça compte, la migration
du `LOT-69` n'a livré que **deux** plans (un fond, un devant) : une seule paire de profondeurs qui
bouge, quand une parallaxe lisible en montre au moins trois — un fond lointain qui bouge à peine, un
fond proche qui suit la caméra à mi-vitesse, un premier plan qui la dépasse.

## Périmètre

### Inclus
- Un **troisième plan**, « lointain », densité **4** px/unité (la plus basse acceptée,
  `EX-DEC-041`), sur les **deux seuls** tableaux à parallaxe active : `demo-mouvement` et
  `demo-final`. Facteur de défilement **inférieur** à celui du plan « fond » existant, pour
  distinguer visiblement les trois profondeurs (lointain < fond < 1.0 < devant).
- Extension de `scripts/generate_demo_plans.py` : une fonction de peinture dédiée au plan lointain,
  qui **réutilise** les primitives déjà là (`wash`, la boucle de crête déchiquetée factorisée hors
  de `paint_backdrop`, `drift`/`stand_on`) plutôt que d'en inventer un second jeu.
- Un motif procédural nouveau, `star()`, pour semer le ciel du thème nocturne (`demo-mouvement`) —
  seul thème du dépôt qui en a l'usage.
- Un garde-fou système (`test_plans_livres.cpp`) : les tableaux à parallaxe active livrent **au
  moins trois** plans dont les facteurs de parallaxe sont strictement croissants — pour qu'une
  régression future (plan lointain oublié en migrant un niveau, facteur mal réglé) échoue plutôt que
  de se découvrir à l'écran.

### Exclus
- **Les vingt tableaux en cadrage `WholeLevel`** : leur parallaxe est neutralisée par construction
  (`hmi::planeParallaxActive`), un plan lointain n'y serait jamais visible — livrer une texture pour
  rien y répéterait exactement l'erreur que `LOT-69` TACHE-10 a nommée et évitée.
- **Aucune nouvelle mécanique moteur.** Le type `core::Plane`, son rendu, son bornage et son
  activation par mode existent depuis `LOT-69` ; ce lot ne fait qu'en peupler un troisième rang.
- **Repeindre les plans « fond »/« devant » existants.** Leurs facteurs et leur contenu restent
  ceux du `LOT-69` ; seul un nouveau plan s'ajoute devant eux dans la pile.
- **Rebasculer un tableau `WholeLevel` en `PerRoom`/`Follow`** pour lui donner accès à la
  parallaxe : changerait le gameplay (cadrage choisi, `EX-REN-016`) pour un motif purement visuel,
  hors de propos ici.

## Décisions de cadrage
- **Densité 4, pas 8.** Un plan lointain est vu de loin, immobile ou presque (facteur bas) et
  presque entièrement masqué par le plan « fond » opaque sous l'horizon (seule sa bande de ciel
  reste visible, au travers du voile semi-transparent de `paint_backdrop`) : la moitié de définition
  déjà perdue par le fond n'a pas de raison de valoir mieux ici (même raisonnement que
  `BACKDROP_PIXELS_PER_UNIT`, `LOT-69` TACHE-10).
- **La crête du plan lointain est peinte au-dessus de l'horizon du plan fond**, jamais en dessous :
  c'est la seule zone où elle a une chance d'être visible, le fond étant opaque à partir de son
  horizon. Peindre plus bas coûterait de la mémoire pour un pixel qui ne s'affiche jamais.
- **Aucun facteur de parallaxe verticale déclaré** (`parallaxY`) : les tableaux ciblés ne défilent
  qu'horizontalement (suivi latéral, salles côte à côte) ; le repli documenté par `LOT-69`
  (`parallaxY` retombe sur `parallaxX`) suffit, comme pour les plans existants.
- **La boucle de crête déchiquetée de `paint_backdrop` est factorisée** en une fonction partagée,
  plutôt que dupliquée pour le plan lointain : c'est exactement la même géométrie (bandes de largeur
  aléatoire, sommet jitteré), à un jeu de paramètres près.
- **Deux tableaux, pas vingt-deux.** Peindre un lointain sur les niveaux `WholeLevel` coûterait une
  texture et une passe de rendu (`TACHE-09` du `LOT-69`) pour un plan que la parallaxe neutralisée
  rendrait indiscernable d'un fond fixe mal aligné — le lot applique la même règle que `LOT-69` a
  déjà posée pour les plans de devant.

## Exigences couvertes
Aucune nouvelle : ce lot **consomme** `EX-DEC-040` (liste ordonnée de plans), `EX-DEC-041`
(densité par plan), `EX-DEC-043` (facteur de parallaxe par plan), `EX-DEC-044` (garde-fous de coût)
et `EX-REN-049` (composition/rendu des plans) — tous livrés et inchangés par `LOT-69`. Réutilise
aussi `EX-LVL-009` (format `planes`/`parallax`) et `EX-VIS-007` (mention de la parallaxe 2D comme
objectif produit, `decors.md`).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-plan-lointain.md) | Plan « lointain » : générateur, contenu, format des deux niveaux | `scripts`, `Source/Elements/Levels` | ✅ |
| [TACHE-02](tache-02-garde-fou-documentation.md) | Garde-fou de non-régression, documentation et référentiel | `Source/Test`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. `demo-mouvement` et `demo-final` déclarent chacun **trois** plans, aux facteurs de parallaxe
   strictement croissants (lointain < fond < devant), et se chargent/se jouent sans changement de
   géométrie de collision (`EX-ARCH-012`).
2. Chaque PNG de plan lointain mesure exactement `largeur × 4` par `hauteur × 4` pixels
   (`EX-DEC-041`), et est **reproductible** : rejouer `scripts/generate_demo_plans.py` ne produit
   aucune différence (`--check` vert, `LOT-66`).
3. Les vingt autres tableaux sont **inchangés** — aucun plan ajouté, aucun PNG régénéré à un octet
   près.
4. Un tableau à parallaxe active qui perdrait son plan lointain, ou dont les facteurs cesseraient
   d'être croissants, fait échouer un test — démontré par un cas négatif.
5. `ctest` à 100 %, build `/W4 /WX`, les quatre linters (exigences, cahier de test, séquence démo,
   Doxygen) verts, cahier de test régénéré.

## Dépendances
Consomme entièrement [LOT-69](@ref lot-69) (modèle `core::Plane`, rendu, parallaxe, générateur de
plans démo). Aucun lot ne dépend de celui-ci.

## Navigation des tâches
- @subpage lot-70-tache-01-plan-lointain
- @subpage lot-70-tache-02-garde-fou-documentation
