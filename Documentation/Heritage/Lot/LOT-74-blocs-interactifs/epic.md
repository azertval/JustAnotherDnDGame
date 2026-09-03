# LOT-74 — Blocs interactifs volatils (descendant, fragile, éphémère) {#lot-74}

> Statut : **fait** (vérification automatisée : build `/W4 /WX` sans avertissement, `ctest`
> intégralement vert — dont les 14 tests unitaires nouveaux des deux contrôleurs et les trois
> tableaux ajoutés à la séquence —, quatre linters et Doxygen verts. La vérification IHM manuelle
> — ressenti des trois blocs, couleurs de palette, clignotement d'avertissement — reste à faire
> par l'utilisateur, comme pour tout lot touchant moteur et rendu.)
> Prérequis : [LOT-72](@ref lot-72) (ground pound, `EX-GP-058` — le geste qui brise le bloc fragile),
> [LOT-63](@ref lot-63) (plateformes mobiles à position continue et portage, `EX-GP-026`),
> [LOT-67](@ref lot-67) (configurations de tuile par instance, éditables et annulables).
> Ne fait partie d'aucun programme cadré : comme les `LOT-67`/`LOT-69`/`LOT-70`, il répond à un
> manque constaté à l'usage — ici, un manque **explicitement consigné par le `LOT-72`** (voir
> Objectif).

## Objectif

Donner au level design un vocabulaire de blocs qui **ne survivent pas** au passage du personnage :
un bloc qui s'enfonce dès qu'on le touche, un bloc qui vole en éclats sous un ground pound, un bloc
qui s'efface une fois qu'on l'a quitté. Trois tuiles, trois façons pour le sol de se dérober.

Avant ce lot, le moteur comptait 33 `TileType` et quatre familles de mécanismes vivants
(`core::BlockController`, `core::MechanismController`, `core::DangerController`,
`core::PlatformController`). **Aucune** ne décrivait un bloc destructible, fragile ou disparaissant :
une recherche `destructible|fragile|crumbl|breakable` sur `Source/` n'y renvoyait rien côté
gameplay. Une fois posé, un bloc était là pour toujours — la seule matière dont l'état variait était
la porte (`EX-GP-021`), et elle ne fait que s'ouvrir. Le sol était donc un acquis : le personnage
pouvait toujours revenir sur ses pas, et aucun tableau ne pouvait demander de **décider vite**.

**Ce manque est déjà consigné, et par ce dépôt lui-même.** Le `LOT-72` a buté dessus en livrant le
ground pound, et l'a écrit deux fois — dans son `epic.md` (`### Exclus`) et dans
`tache-04-ground-pound.md` (`## Points d'attention`) :

> **Casse de blocs fragiles** au ground pound : nécessiterait un nouveau `TileType` (aucun bloc
> fragile n'existe dans `Source/Core/Levels/TileType.h`), avec support éditeur/rendu — hors périmètre
> d'un lot de mouvement.

Le ground pound est donc livré depuis le `LOT-72` avec, littéralement, **rien à casser** : son seul
effet propre est une secousse caméra héritée de tout impact lourd. Ce lot lui donne sa cible, et
règle du même geste les deux autres blocs volatils — non par opportunisme, mais parce que les trois
partagent **exactement** le même pipeline (type de tuile → chargeur/écrivain → éditeur → rendu →
contrôleur → contenu de démonstration). Les cadrer séparément paierait trois fois la même chaîne de
huit fichiers pour trois comportements de quelques dizaines de lignes chacun.

Le pari technique du lot est qu'**aucune passe de physique nouvelle** n'est nécessaire. Les deux
mécaniques dont le moteur a besoin — porter le personnage sur un corps à position continue, et
retirer de la matière de la grille de collision en cours de partie — existent déjà : la première
dans `core::PlatformController` (`EX-GP-026`), la seconde dans `core::MechanismController` (copie
mutable du `TileMap`, portes et clés). Ce lot les réutilise telles quelles plutôt que d'ouvrir un
troisième chemin de collision (voir « Décisions de cadrage »).

## Périmètre

### Inclus
- **Bloc descendant** (`EX-GP-027`) : **tout** contact du personnage l'arme définitivement ; il
  descend ensuite à vitesse constante, en **portant** ce qui repose dessus, jusqu'à s'arrêter contre
  la matière pleine ou à sortir par le bas du tableau.
- **Bloc fragile** (`EX-GP-028`) : solide comme un `Solid`, **détruit** par un ground pound
  (`EX-GP-058`) qui l'atteint par le dessus — et par lui seul.
- **Bloc éphémère** (`EX-GP-029`) : solide ; une fois que le personnage s'y est posé **puis l'a
  quitté**, il disparaît après un court délai, **définitivement** jusqu'au rechargement du tableau.
- Support **complet** des trois types dans la chaîne : chargeur/écrivain de niveau, brouillon
  éditable, palette de l'éditeur (libellés localisés FR/EN), rendu jeu et éditeur.
- **Contenu** : trois tableaux de démonstration, insérés dans la séquence livrée, avec leur parcours
  scripté (`EX-LVL-012` — une mécanique introduite à la fois).
- **Réduction d'une dette de synchronisation** : la borne « dernier énumérateur de `TileType` »,
  aujourd'hui recopiée à la main dans **trois** fichiers, devient une sentinelle unique (TACHE-02).

### Exclus
- **Réapparition** d'un bloc éphémère ou fragile après un délai : décision de cadrage explicite (voir
  plus bas), la disparition est définitive jusqu'au `reload()`. Une variante réapparaissante serait
  une **quatrième** mécanique, avec sa propre question de conception (que faire si le personnage
  occupe la case au moment de la réapparition ?) — elle n'est pas nécessaire pour combler ce qui
  manque aujourd'hui, et rien dans ce lot ne l'interdit plus tard.
- **Casse d'un bloc fragile par un dash vers le bas**, ou par tout autre geste : le ground pound est
  le seul déclencheur (décision de cadrage). Tant qu'une charge de dash existe, « dash + bas » reste
  un dash vertical normal (`EX-GP-058`) : lui donner un pouvoir de destruction rendrait le
  comportement du bloc dépendant du **budget de dash restant**, donc illisible pour le joueur.
- **Poussée** d'un bloc descendant, et tailles réduites (`×0.5`/`×0.25`, `EX-GP-005`) des trois
  nouveaux blocs : ce sont des blocs de décor volatil, pas des blocs poussables. Les combiner
  demanderait de faire cohabiter `core::BlockController` (case par case) et une position continue
  sur la même tuile — un chantier sans rapport avec l'objectif.
- **Réentraînement des modèles d'IA livrés** : l'encodage d'observation gagne trois canaux
  (TACHE-07), ce qui change la **forme** du tenseur ; les modèles publiés avant ce lot ne sont donc
  plus rechargeables. Le lot livre le refus explicite au chargement, pas de nouveaux modèles.
- **Nouvelle touche ou nouvelle action de contrôle** : les trois blocs réagissent à des gestes déjà
  mappés (contact, ground pound, départ). `Documentation/Specification/controles.md` est inchangé.

## Décisions de cadrage

- **Le bloc descendant est un corps à position continue, pas un bloc de grille.** Il émet des
  `core::PlatformSample` (`Core/Physics/PlatformSample.h`), exactement comme
  `core::PlatformController`. Concaténés à la liste de plateformes du pas, ces échantillons lui
  donnent **gratuitement** le portage du personnage
  (`core::CharacterPhysicsSystem::applyPlatformPortage`), la collision continue
  (`core::sweepAabbVsAabb`), le portage des blocs poussables posés dessus
  (`core::BlockController::update`) et l'interpolation d'affichage (`hmi::PreviousPosition`). Aucune
  passe de collision nouvelle n'est écrite : c'est le cœur de l'économie de code de ce lot.
- **Armement par tout contact, jamais désarmé.** Le bloc s'arme au premier recouvrement entre sa
  boîte et celle du personnage — par le dessus, par le côté, par le dessous. Cette règle est plus
  lisible pour le joueur (« je l'ai touché, il part ») qu'un test de portage, et elle évite d'avoir à
  définir un seuil de « vraiment posé dessus ». Une fois armé, il ne se réarme ni ne s'arrête : sa
  descente est un aller simple.
- **Le bloc descendant ne traverse jamais la matière.** Il s'arrête contre une case pleine et n'est
  retiré du niveau que s'il franchit le **bord bas** du tableau. Aucun autre corps de ce moteur ne
  traverse un `Solid` ; en faire une exception ici rendrait le comportement imprévisible dès qu'un
  décor passe sous le bloc.
- **Écrasement mortel.** Un personnage coincé entre un bloc descendant et le sol meurt, en
  réutilisant le front `squished` existant — même décision que pour la plateforme mobile écrasant
  contre un plafond (`EX-GP-026`, cadrage retenu au `LOT-63`), plutôt que de mettre le bloc en pause.
- **Le ground pound est le seul geste qui brise un bloc fragile** (`core::Player::groundPounding`),
  et le contact doit venir du **dessus**. Voir « Exclus » pour la raison : tout autre déclencheur
  ferait dépendre la destruction du budget de dash restant.
- **Le bloc fragile est brisé _avant_ que la physique ne résolve le pas.** Le contrôleur tourne à
  l'étape 4 du pas fixe (avec `core::BlockController`), sur la boîte du personnage **d'avant** le
  pas. C'est indispensable : résolu après, le ground pound s'arrêterait net sur un bloc qu'il est
  précisément censé traverser en le brisant.
- **Bloc fragile et bloc éphémère partagent un seul contrôleur**
  (`core::VolatileBlockController`). Ni l'un ni l'autre ne bouge : tous deux ne font que **quitter**
  la grille de collision, sur le patron de la copie mutable du `TileMap` déjà tenue par
  `core::MechanismController`. Deux règles de retrait, un seul overlay — deux contrôleurs jumeaux
  auraient dupliqué l'overlay et son ordre de composition.
- **« Le personnage a quitté le bloc » est un front, pas un état.** Le moteur ne conserve aucun
  contact persistant : `player.grounded` est recalculé de zéro à chaque pas. Le contrôleur mémorise
  donc, par bloc, « reposait-il dessus au pas précédent », et déclenche sur la transition
  `true → false` — patron de `hmi::detectPlayerEvents` (`HMI/Game/GameEvents.h`), test de portage
  `core::restsOnTopOfPlatform`.
- **Disparition définitive jusqu'au rechargement.** Un bloc éphémère effacé ou un bloc fragile brisé
  ne revient pas ; `hmi::GameSession::reload()` remet le niveau à neuf comme pour tout le reste. Le
  risque de rendre un tableau insoluble est réel et **assumé** : c'est exactement ce que le test
  système `ParcoursCompletSysteme` détecte, en relevant la trajectoire réelle plutôt que la seule
  présence des tuiles.
- **Les trois valeurs sont ajoutées _après_ `MovingPlatform` dans l'énumération**, jamais insérées au
  milieu. Insérer décalerait les canaux d'observation de l'IA (`aisolver::TileWindowEncoder`, dont
  les indices dérivent de l'ordre de l'enum) et changerait le **sens** des poids déjà entraînés ;
  ajouter en fin préserve la signification de tous les canaux existants. Le prix — trois bornes
  `0 … TileType::MovingPlatform` recopiées à la main — est payé une fois pour toutes par la
  sentinelle unique de TACHE-02, plutôt qu'une troisième fois à la main.
- **Déterminisme** (`EX-NFR-002`) : aucune accumulation flottante. Les deux contrôleurs comptent des
  **pas fixes entiers** (patron `core::DangerController::_stepCount`,
  `core::BlockController::_fallTimers`) ; la position d'un bloc descendant est fonction de
  `(pas courant − pas d'armement)`, jamais d'un `position += vitesse * dt`.

## Exigences couvertes

Nouvelles : [`EX-GP-027`](@ref EX-GP-027) (bloc descendant), [`EX-GP-028`](@ref EX-GP-028) (bloc
fragile), [`EX-GP-029`](@ref EX-GP-029) (bloc éphémère).

Réutilisées : [`EX-GP-005`](@ref EX-GP-005) (collision boîte-contre-boîte),
[`EX-GP-022`](@ref EX-GP-022) (blocs poussables portés par un bloc descendant),
[`EX-GP-026`](@ref EX-GP-026) (position continue, portage, écrasement mortel),
[`EX-GP-031`](@ref EX-GP-031) (contact mortel), [`EX-GP-058`](@ref EX-GP-058) (ground pound),
[`EX-LVL-004`](@ref EX-LVL-004) (validation d'un niveau), [`EX-LVL-012`](@ref EX-LVL-012) et
[`EX-LVL-015`](@ref EX-LVL-015) (contenu de démonstration et sa couverture),
[`EX-EDIT-005`](@ref EX-EDIT-005) (édition annulable), [`EX-ARCH-011`](@ref EX-ARCH-011),
[`EX-NFR-002`](@ref EX-NFR-002) (déterminisme), [`EX-NFR-040`](@ref EX-NFR-040) (erreur
récupérable).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-types-de-tuile.md) | Trois types de tuile, bout en bout | `Source/Core/Levels`, `Source/HMI/Editor`, `Source/Elements/Localization` | ✅ |
| [TACHE-02](tache-02-borne-enumeration.md) | Sentinelle unique de fin d'énumération | `Source/Core/Levels/TileType.h` et ses trois copies | ✅ |
| [TACHE-03](tache-03-bloc-descendant.md) | Bloc descendant : contrôleur à position continue | `Source/Core/Gameplay/SinkingBlockController.*` | ✅ |
| [TACHE-04](tache-04-bloc-fragile.md) | Bloc fragile : destruction au ground pound | `Source/Core/Gameplay/VolatileBlockController.*` | ✅ |
| [TACHE-05](tache-05-bloc-ephemere.md) | Bloc éphémère : disparition au départ du personnage | `Source/Core/Gameplay/VolatileBlockController.*` | ✅ |
| [TACHE-06](tache-06-integration-pas-fixe.md) | Intégration dans le pas fixe et rendu | `Source/HMI/Game/GameSession.cpp`, `Source/HMI/Graphics` | ✅ |
| [TACHE-07](tache-07-encodage-ia.md) | Encodage d'observation de l'IA : 33 → 36 canaux | `Source/AiSolver/Env` | ✅ |
| [TACHE-08](tache-08-contenu-demo.md) | Contenu : trois tableaux, séquence, parcours scripté | `Source/Elements/Levels`, `Source/Test/Systeme` | ✅ |
| [TACHE-09](tache-09-tests.md) | Tests croisés et déterminisme | `Source/Test/{Unit,Integration,Systeme}` | ✅ |
| [TACHE-10](tache-10-documentation.md) | Documentation, exigences, CHANGELOG | `Documentation/`, `CHANGELOG.md` | ✅ |

## Critères d'acceptation du lot

1. Sans poser aucune des trois nouvelles tuiles, le jeu se comporte **exactement** comme avant le
   lot : `ctest` intégralement vert, `demo-final` et le rejeu IA inchangés, aucun tableau existant
   modifié.
2. **Bloc descendant** : un contact quelconque l'arme ; il descend ensuite à vitesse constante en
   portant le personnage et les blocs poussables posés dessus ; il s'arrête contre une case pleine ;
   il est retiré du niveau en franchissant le bord bas ; un personnage écrasé contre le sol meurt.
3. **Bloc fragile** : un ground pound qui l'atteint par le dessus le détruit **et se poursuit** au
   travers, sans arrêt visible ; aucun autre geste ne le brise (marche, saut, dash horizontal ou
   vertical, poussée de bloc) ; la destruction dure jusqu'au rechargement du tableau.
4. **Bloc éphémère** : il reste solide tant que le personnage est dessus, quelle que soit la durée ;
   il disparaît après le délai fixé une fois quitté ; il ne réapparaît pas.
5. Les trois tuiles sont **posables dans l'éditeur** (palette, libellés FR et EN), **sauvegardées et
   rechargées** sans perte, **annulables** (`EX-EDIT-005`), et **distinctes à l'œil** en jeu comme
   en édition.
6. Un fichier de niveau écrit **avant** ce lot se charge à l'identique, et un fichier contenant un
   type inconnu échoue proprement en **erreur récupérable** (`EX-NFR-040`), sans plantage.
7. Les trois tuiles sont posées **au moins trois fois** dans la séquence livrée et **atteignables**
   par le parcours scripté (`test_couverture_mecaniques`, `ParcoursCompletSysteme`).
8. La borne de fin d'énumération n'existe plus qu'à **un seul endroit** ; le nombre de canaux
   d'observation de l'IA vaut 36 et un modèle à l'ancienne forme est refusé avec un message
   explicite, jamais par un plantage.
9. `ctest` à 100 %, les quatre linters verts, build `/W4 /WX` sans avertissement, Doxygen vert,
   déterminisme conservé (`EX-NFR-002`).

## Dépendances

S'appuie sur le ground pound du [LOT-72](@ref lot-72) (`EX-GP-058`), sur la position continue et le
portage des plateformes mobiles du [LOT-63](@ref lot-63) (`EX-GP-026`), et sur les configurations de
tuile par instance du [LOT-67](@ref lot-67). Aucun lot ne dépend de celui-ci à ce jour. La sentinelle
d'énumération de TACHE-02 bénéficie en revanche à **tout** lot futur ajoutant un type de tuile.

## Navigation des tâches

- @subpage lot-74-tache-01-types-de-tuile
- @subpage lot-74-tache-02-borne-enumeration
- @subpage lot-74-tache-03-bloc-descendant
- @subpage lot-74-tache-04-bloc-fragile
- @subpage lot-74-tache-05-bloc-ephemere
- @subpage lot-74-tache-06-integration-pas-fixe
- @subpage lot-74-tache-07-encodage-ia
- @subpage lot-74-tache-08-contenu-demo
- @subpage lot-74-tache-09-tests
- @subpage lot-74-tache-10-documentation
