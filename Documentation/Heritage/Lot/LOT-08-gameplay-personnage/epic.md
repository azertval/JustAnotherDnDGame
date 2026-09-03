# LOT-08 — Gameplay personnage : déplacement, gravité et collisions {#lot-08}

> Statut : **terminé**. Ce lot rend le personnage **jouable** dans le niveau chargé au LOT-07 :
> il se déplace horizontalement, **tombe sous gravité**, entre en **collision** avec les tuiles
> solides, **gagne** en atteignant la sortie et **recommence** en cas d'échec (danger ou chute).
> Le **saut** et le **comportement des mécanismes** restent des lots ultérieurs.

## Objectif
Faire du niveau statique du LOT-07 une **scène jouable**. Un personnage contrôlé au clavier se
déplace horizontalement à vitesse constante (`EX-GP-010`), subit une **gravité continue** tant
qu'il n'est pas au sol (`EX-GP-012`) et ne **traverse jamais** une tuile solide, même à vitesse
élevée (`EX-GP-002`, `EX-GP-014`). Atteindre la **sortie** termine le niveau en succès
(`EX-GP-030`) ; toucher un **danger** ou **sortir par le bas** provoque l'échec et le
**redémarrage** du niveau à son état initial (`EX-GP-031`, `EX-GP-032`). Le jeu étant conçu **par
tableaux** (un niveau = un écran), la **caméra reste fixe** et cadre le tableau entier, le
personnage restant toujours visible (adaptation de `EX-REN-013`, cf. décisions de cadrage).

Toute la logique de simulation (physique, collisions, règles de fin) vit dans **`Core`** comme
**donnée + système purs**, à **pas de temps fixe et déterministe** (`EX-NFR-002`, `EX-ARCH-011`),
donc testable sans fenêtre ni GPU (`EX-NFR-010`). La couche `HMI` ne fait que traduire les
entrées en **actions logiques** (`EX-CTRL-010`) et afficher le résultat.

## Périmètre

### Inclus
- **Composants (Core)** : marqueur `Player` (avec état « au sol »), `Collider` (boîte englobante
  AABB en unités monde) ; réutilise `Transform` et `Velocity` (LOT-03).
- **Balayage AABB (Core)** : primitive géométrique **pure** de collision *continue* (temps
  d'impact + normale) d'une boîte mobile contre la grille de tuiles solides, avec **glissement**
  le long des surfaces — cœur de la robustesse « pas de traversée à vitesse élevée » (`EX-GP-014`).
- **Physique du personnage (Core)** : système à pas fixe appliquant la **gravité** et le
  **déplacement horizontal** à vitesse constante, résolvant les collisions via le balayage et
  mettant à jour la vitesse, la position et l'état « au sol » (`EX-GP-010`, `EX-GP-012`,
  `EX-GP-014`).
- **Règles de fin de niveau (Core)** : évaluation **pure** de l'issue (`EnCours` / `Gagné` /
  `Perdu`) selon le recouvrement du personnage avec la **sortie**, un **danger** ou la **limite
  basse** (`EX-GP-030`, `EX-GP-031`).
- **Actions logiques (HMI)** : traduction de l'`InputState` en une **intention** de déplacement
  (`gauche` / `droite`), dissociée des touches physiques (`EX-CTRL-010`), échantillonnée une fois
  par frame en amont de la logique (`EX-CTRL-020`, `EX-CTRL-021`).
- **Intégration jouable (HMI)** : le `GameScreen` fait apparaître le personnage à l'**entrée**,
  exécute physique + règles à chaque pas fixe, garde une **caméra fixe** cadrant le tableau
  (adaptation de `EX-REN-013`), affiche
  le sprite du personnage (`EX-REN-011`), revient au **menu** au succès et **réinitialise** le
  niveau à l'échec.

### Exclus (lots ultérieurs)
- **Saut** (`EX-GP-011`, `EX-GP-013`) : décision de cadrage — ce lot pose la gravité et les
  collisions ; le saut et le *game feel* (coyote time, jump buffering) viendront dans une passe
  **physique** ultérieure.
- **Comportement des mécanismes** (interrupteur ↔ porte, blocs, clé — `EX-GP-020`…`EX-GP-023`) :
  chargés et représentés depuis le LOT-07, sans interaction ici.
- **Écrans Pause et Fin de niveau** (`EX-REN-031`, états `Pause`/`NiveauTermine` de `EX-GP-040`) :
  au succès on revient au menu, sans écran dédié.
- **Manette et remappage** (`EX-CTRL-012`, manette) ; **animations** (`EX-REN-012`).
- **Enchaînement multi-niveaux** (`EX-LVL-010`/`EX-LVL-011`) : un seul niveau, rejoué sur échec.

## Décisions de cadrage
- **Collisions par balayage continu (swept AABB)** — choisi plutôt que la résolution par axe à
  pas fixe, pour permettre des niveaux exigeant **rapidité et précision** du joueur sans
  traversée de mur à vitesse élevée. Le balayage calcule le **temps d'impact** le long du
  déplacement et fait **glisser** la boîte le long des surfaces. Satisfait `EX-GP-014` au sens
  fort.
- **Gravité oui, saut non** — le personnage **marche et tombe** mais ne saute pas encore ; la
  gravité est le minimum pour un niveau franchissable (l'entrée est en haut, la sortie en bas) et
  rien n'est jeté quand le saut s'ajoutera (l'état « au sol » est déjà géré).
- **Retour au menu au succès** — atteindre la sortie ramène au menu (pas d'écran de fin, hors
  périmètre) ; l'échec **redémarre** le niveau à l'état initial (`EX-GP-032`).
- **Jeu par tableaux, caméra fixe** — chaque niveau tient dans **un écran** ; la caméra ne
  **suit pas** le personnage mais **cadre le tableau entier** (le personnage reste toujours
  visible). C'est une **adaptation de `EX-REN-013`** (« caméra qui suit, bornée ») au design
  retenu : le cadrage fixe satisfait l'esprit de l'exigence (personnage à l'écran, vue bornée au
  niveau) et reprend le cadrage déjà en place au LOT-07. La spec `EX-REN-013` sera reformulée en
  conséquence lors d'une passe ultérieure. Conséquence : **pas de logique de suivi** à écrire.
- **Frontière Core/HMI** — toute la simulation (composants, physique, collisions, règles) est
  **pure** dans `Core/Ecs` / `Core/Physics`, alimentée par une **intention d'entrée** neutre
  (`core::PlayerInput`). `HMI` ne fait que **mapper les touches** vers cette intention et
  **rendre** la scène. Cela garde la simulation déterministe et testable hors GPU.
- **Réinitialisation** — le `Level` chargé est **conservé en mémoire** ; réinitialiser reconstruit
  la scène (monde ECS + personnage à l'entrée) à partir de ce modèle, sans relire le fichier.

## Exigences couvertes
- `EX-GP-010` (déplacement horizontal), `EX-GP-012` (gravité continue), `EX-GP-002`/`EX-GP-014`
  (collisions solides, pas de traversée), `EX-GP-030` (sortie = succès), `EX-GP-031` (danger /
  chute = échec), `EX-GP-032` (redémarrage à l'état initial).
- `EX-CTRL-010` (actions logiques), `EX-CTRL-020` (latence ≤ 1 frame), `EX-CTRL-021`
  (échantillonnage 1×/frame).
- `EX-REN-013` (**adapté** : caméra fixe cadrant le tableau, personnage visible), `EX-REN-011`
  (sprite du personnage).
- `EX-NFR-002` (pas fixe déterministe), `EX-NFR-010`/`EX-NFR-020` (testabilité, tests),
  `EX-ARCH-011` (composants = données pures).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-composants-personnage.md) | Composants du personnage & intention d'entrée | `Core/Ecs` | ✅ |
| [TACHE-02](tache-02-balayage-aabb.md) | Balayage AABB contre la grille (géométrie pure) | `Core/Physics` | ✅ |
| [TACHE-03](tache-03-physique-personnage.md) | Physique du personnage (gravité + déplacement + collisions) | `Core/Ecs/Systems` | ✅ |
| [TACHE-04](tache-04-regles-fin-niveau.md) | Règles de fin de niveau (succès / échec) | `Core/Levels` | ✅ |
| [TACHE-05](tache-05-actions-logiques.md) | Actions logiques d'entrée (mapping touches → intention) | `HMI/Input` | ✅ |
| [TACHE-06](tache-06-integration-jouable.md) | Intégration jouable dans `GameScreen` (cadrage fixe, succès/échec) | `HMI/Interface` | ✅ |

## Critères d'acceptation du lot
1. Le personnage **se déplace** à gauche/droite à vitesse constante et **tombe** sous gravité ;
   il **repose** sur les tuiles solides et **ne les traverse pas**, même en chute rapide (test de
   non-tunneling vert).
2. **Atteindre la sortie** termine le niveau et **revient au menu** ; **toucher un danger** ou
   **sortir par le bas** **redémarre** le niveau à son état initial.
3. La **caméra reste fixe** et cadre le **tableau entier** ; le personnage est toujours visible.
4. La physique, le balayage, les règles de fin et le mapping d'actions sont **couverts par des
   tests** (unitaires + au moins un test d'**intégration** « entrées → monde → issue ») ; `ctest`
   vert.
5. Build `/W4 /WX` sans avertissement, documentation Doxygen et `CHANGELOG.md` à jour, `lint`
   des exigences vert.

## Dépendances
- Réutilise l'**ECS** (`core::World`, `Transform`, `Velocity`, LOT-03), le **modèle de niveau**
  (`TileMap`/`Level`, LOT-07) comme source de vérité des collisions, le **rendu 2D**
  (`SpriteRenderer`, `Camera2D`, LOT-05) et l'**`InputState`** (LOT-06).
- S'intègre au `GameScreen` (LOT-07) : le niveau statique devient jouable.

## Navigation des tâches
- @subpage lot-08-tache-01-composants-personnage
- @subpage lot-08-tache-02-balayage-aabb
- @subpage lot-08-tache-03-physique-personnage
- @subpage lot-08-tache-04-regles-fin-niveau
- @subpage lot-08-tache-05-actions-logiques
- @subpage lot-08-tache-06-integration-jouable
