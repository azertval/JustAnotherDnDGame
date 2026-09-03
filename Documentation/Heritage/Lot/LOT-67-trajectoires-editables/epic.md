# LOT-67 — Trajectoires et règles de tableau éditables {#lot-67}

> Statut : **fait** (vérification automatisée : build Debug/Release, `ctest` à 100 %, lint
> d'exigences, cahier de test régénéré ; la vérification IHM du canevas Direct3D 11 — poignées,
> aperçu du glisser, panneau — reste manuelle, comme pour tout lot touchant au rendu).
> Prérequis : [LOT-63](@ref lot-63) (plateformes mobiles et dangers avancés),
> [LOT-50](@ref lot-50) (patron de geste à poignées), [LOT-64](@ref lot-64) (patron de propriété
> de niveau éditable).

## Objectif

Rendre au level designer la maîtrise des éléments mobiles, aujourd'hui hors de sa portée.

Le constat qui déclenche ce lot est net : `core::LevelDraft` expose depuis le `LOT-63` trois
mutateurs annulables et couverts par des tests — `setPlatformConfig`, `setMoverConfig`,
`setBlinkConfig` — dont **aucun n'a jamais eu d'appelant dans `Source/HMI`**. Le modèle d'édition
existait, l'éditeur ne s'en servait pas. Concrètement, définir la route d'une plateforme mobile ou
le rythme d'un piège imposait d'ouvrir le JSON à la main, ce que l'existence même de l'éditeur
(`EX-VIS-006`) exclut. Même situation pour `jumpBudget`/`dashBudget`, sérialisés et appliqués au
spawn depuis longtemps, jamais exposés.

Le lot comble ce trou et lève deux limites de conception devenues gênantes au même endroit :

- **Une plateforme n'avait que deux points.** `MovingPlatformConfig` portait `startPosition` et
  `endPosition`, et la documentation actait explicitement « pas de chemin multi-points ». Un
  parcours en L, un circuit fermé, un aller simple par un autre chemin : rien de tout cela n'était
  exprimable.
- **Le nombre de sauts aériens était global et codé en dur** (`PhysicsConfig::airJumps`), et le
  dash n'avait qu'un booléen rechargé à l'atterrissage — aucune notion de charges. Un tableau ne
  pouvait pas moduler la mobilité du personnage, alors qu'il pouvait déjà en limiter le budget.

## Périmètre

### Inclus
- **Route multi-points** pour les plateformes mobiles (`EX-GP-054`) : N points de passage, mode
  aller-retour ou circuit fermé, à vitesse constante et de façon déterministe.
- **Format de niveau** correspondant (`EX-LVL-008`), avec repli compatible sur `endX`/`endY`.
- **Outil « Parcours »** au canevas (`EX-EDIT-032`) : poignées glissables pour déplacer, insérer et
  retirer un point ; poignée d'extrémité pour l'axe et la portée d'un danger mobile.
- **Panneau « Propriétés »** (`EX-EDIT-033`) : vitesse, déphasage et mode d'une plateforme ; axe et
  portée d'un danger mobile ; période, déphasage et durée active d'un danger temporisé ; règles du
  tableau.
- **Capacités par tableau** (`EX-GP-055`) : sauts aériens et charges de dash, rechargés à chaque
  atterrissage — distincts des budgets consommables déjà existants.
- **Correction** du `pushUndo()` manquant sur `setJumpBudget`/`setDashBudget`, seules propriétés de
  niveau non annulables de tout le brouillon.

### Exclus
- **La taille d'une plateforme reste une case** (`1×1` codé en dur dans `PlatformController`). Les
  poignées de parcours pourraient suggérer qu'on redimensionne aussi : ce n'est pas le cas, et
  aucune poignée de coin n'est introduite, précisément pour ne pas le laisser croire.
- **Le défaut connu plateforme × pente** (voir le guide des niveaux) n'est **pas** corrigé. Sa
  racine est hors du contrôleur de plateformes, et ce lot n'y touche pas ; la consigne d'éviter de
  combiner les deux dans un même fichier reste en vigueur.
- **Aucun nouveau tableau** n'est conçu. Le contenu livré est migré vers le nouveau format sans
  changement de comportement ; dessiner un niveau exploitant les circuits fermés est un acte de
  *level design*, distinct de ce lot d'outillage.
- La **vitesse d'un danger mobile** reste une constante de conception, comme avant.

## Décisions de cadrage

- **Les waypoints listent les points *après* le départ.** La tuile elle-même est le point 0,
  implicite : c'est la généralisation exacte de `endX`/`endY`, et cela rend impossible toute
  contradiction entre le premier point et la position de la tuile.
- **Le mode par défaut n'est jamais écrit** dans le JSON, comme tout champ à sa valeur par défaut.
- **Le calcul de distance passe en double précision.** En `float`, `stepCount + phase` perd le bit
  de poids faible au-delà d'environ 16,7 millions de pas (~77 h de jeu) : la plateforme se décale
  visiblement en fin de longue session. Le défaut préexistait au lot ; le refactor le corrige et un
  test le fige.
- **La géométrie du parcours est partagée** entre gameplay et overlay (`core::PlatformPath`) : le
  trajet dessiné dans l'éditeur est littéralement celui qui sera parcouru.
- **Un geste complet ne produit qu'une action**, donc un seul pas d'annulation ; les champs
  numériques du panneau émettent sur `editingFinished` et non `valueChanged`, sinon taper « 120 »
  empilerait trois pas d'annulation.
- **Un nouveau dock, pas un onglet du panneau Textures.** Le `LOT-57` a recentré ce panneau sur ce
  qui définit l'apparence, en le débarrassant de l'inspecteur des décors. Y loger de la vitesse de
  plateforme et des budgets de saut referait exactement l'erreur qu'il a corrigée : c'est du
  comportement, pas de l'habillage.
- **Le point de départ n'a pas de poignée.** Il est la tuile, qu'on déplace en la repeignant — lui
  donner une poignée créerait deux façons contradictoires de faire la même chose.

## Exigences couvertes

`EX-GP-054`, `EX-GP-055`, `EX-LVL-008`, `EX-EDIT-032`, `EX-EDIT-033` (nouvelles) ; `EX-GP-026`
(amendée : la limite à deux points disparaît). Réutilisées : `EX-GP-024`, `EX-GP-015`, `EX-GP-017`,
`EX-GP-051`, `EX-GP-053`, `EX-EDIT-005`, `EX-EDIT-030`, `EX-LVL-005`, `EX-NFR-002`, `EX-NFR-040`.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Route multi-points dans le modèle, le format et le brouillon | `Source/Core/Levels` | ✅ |
| TACHE-02 | Parcours polyligne déterministe (aller-retour et circuit fermé) | `Source/Core/Gameplay` | ✅ |
| TACHE-03 | Capacités par tableau et charges de dash | `Source/Core/{Physics,Ecs,Levels}`, `Source/HMI/Game` | ✅ |
| TACHE-04 | Outil « Parcours » : géométrie, geste et canevas | `Source/HMI/{Editor,Game,Interface}` | ✅ |
| TACHE-05 | Overlay d'édition des trajectoires | `Source/HMI/Graphics` | ✅ |
| TACHE-06 | Panneau « Propriétés » | `Source/HMI/Editor`, `Source/Elements/UI` | ✅ |
| TACHE-07 | Contenu, documentation et référentiel | `Source/Elements/Levels`, `Documentation` | ✅ |

## Critères d'acceptation du lot

1. Une plateforme à un seul point de route se comporte **exactement** comme avant le lot : les
   tests de la suite existante passent sans retouche de leurs valeurs attendues.
2. Un niveau écrit avec `endX`/`endY` et sans capacité déclarée se charge et se joue à l'identique.
3. Une route à trois points est parcourue segment par segment à vitesse constante, puis refaite à
   l'envers ; en mode circuit fermé, elle ne rebrousse jamais chemin et revient exactement au
   départ après un cycle.
4. La position ne dérive pas après vingt millions de pas fixes.
5. Un glisser de poignée, aussi rapide soit-il, produit **une seule** entrée d'historique.
6. Les quatre règles du tableau (deux budgets, deux capacités) sont éditables et annulables, et le
   panneau distingue visiblement les deux notions.
7. `ctest` à 100 %, lint d'exigences vert, cahier de test régénéré.

## Dépendances

Aucun lot ne dépend de celui-ci. Il s'appuie sur les mécaniques du `LOT-63` (plateformes, dangers
avancés), sur le patron de geste à poignées du `LOT-50` et sur le patron de propriété de niveau
éditable du `LOT-64`.

## Navigation des tâches

- @subpage lot-67-tache-01-route-multi-points
- @subpage lot-67-tache-02-parcours-polyligne
- @subpage lot-67-tache-03-capacites-tableau
- @subpage lot-67-tache-04-outil-parcours
- @subpage lot-67-tache-05-overlay-trajectoires
- @subpage lot-67-tache-06-panneau-proprietes
- @subpage lot-67-tache-07-contenu-documentation
