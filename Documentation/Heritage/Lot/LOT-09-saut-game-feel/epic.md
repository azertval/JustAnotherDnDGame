# LOT-09 — Saut, game feel et enchaînement de niveaux {#lot-09}

> Statut : **terminé**. Ce lot ajoute le **saut** au personnage du LOT-08 et le rend **agréable à
> jouer** (coyote time, jump buffering, hauteur variable), puis enchaîne les niveaux en une
> **progression** : écran titre → niveau 1 → niveau 2 → … → dernier niveau → retour au titre. Double
> saut, wall jump, dash, gravité asymétrique et le niveau **puzzle** (mécanismes) restent des lots
> ultérieurs.

## Objectif
Deux apports complémentaires :

1. **Un saut qui fait plaisir** — conforme à la spec (`EX-GP-011`, `EX-GP-013`) : impulsion
   verticale puis retombée sous **gravité constante**, uniquement au sol — augmenté des trois
   techniques de *game feel* décisives :
   - **hauteur de saut variable** (relâcher tôt = petit saut, maintenir = saut complet) ;
   - **coyote time** (~80 ms) : sauter encore un court instant après avoir quitté un bord ;
   - **jump buffering** (~120 ms) : un saut appuyé juste avant l'atterrissage est honoré à la pose.
2. **Un enchaînement de niveaux** (`EX-LVL-010`, `EX-LVL-011`) : les niveaux se jouent dans un
   **ordre défini** ; à la réussite d'un niveau, le **suivant** se charge automatiquement ; après
   le **dernier**, retour à l'**écran titre**.

Toute la simulation reste **pure et déterministe au pas fixe** dans `Core` (`EX-NFR-002`,
`EX-ARCH-011`), testable sans GPU ; `HMI` traduit les touches (`EX-CTRL-010`) et orchestre les
écrans. Le lot fournit au moins **deux niveaux** de démonstration (dont un **exigeant le saut**) et
des tests prouvant la franchissabilité et l'enchaînement.

## Périmètre

### Inclus
- **Données (Core)** : `PlayerInput` enrichi d'une intention de **saut** (*pressé* + *maintenu*,
  `EX-CTRL-011`) ; `Player` enrichi des **minuteries** coyote et buffer ; `PhysicsConfig` enrichi
  des réglages (vitesse d'impulsion, fenêtres, facteur de coupe).
- **Mapping (HMI)** : `Espace` / `W` → intention de saut, dissocié des touches (`EX-CTRL-010`).
- **Saut + hauteur variable (Core)** : impulsion au sol (`EX-GP-011`, `EX-GP-013`), gravité
  **constante**, coupe de la vitesse ascendante au relâchement.
- **Coyote time + jump buffering (Core)** : minuteries au pas fixe assouplissant la condition de
  saut et mémorisant l'appui.
- **Enchaînement de niveaux (HMI)** : une **séquence ordonnée** de niveaux ; réussite → niveau
  suivant chargé automatiquement (personnage réapparu à l'entrée du nouveau niveau) ; après le
  dernier → retour au menu/titre (`EX-LVL-010`, `EX-LVL-011`).
- **Contenu + preuve** : au moins **deux niveaux** de démonstration — un premier (déplacement/chute,
  hérité du LOT-08) et un second **exigeant le saut** (section ascendante) ; tests d'intégration de
  franchissabilité (saut requis) et d'enchaînement (niveau 1 → niveau 2 → fin).

### Exclus (lots ultérieurs)
- **Double saut / saut multiple** (`EX-GP-013`), **wall jump**, **dash**, **gravité asymétrique**.
- **Niveau puzzle** (interrupteur↔porte) du MVP (`EX-LVL-012`) : nécessite le **comportement des
  mécanismes** (`EX-GP-020`…), non encore implémenté — la progression est conçue pour l'accueillir.
- **Écran de fin** dédié (`EX-REN-031`) : après le dernier niveau, on revient au **menu**.
- **Animations** (`EX-REN-012`), manette.

## Décisions de cadrage
- **Quatre techniques de *feel*** : saut au sol, hauteur variable, coyote time, jump buffering.
  Mécaniques avancées différées.
- **Gravité constante** (`EX-GP-011`) : la hauteur variable passe par la **coupe** de la vitesse
  ascendante, jamais par une modulation de la gravité.
- **État de *feel* dans les données** : minuteries portées par `Player`, décomptées par le système
  au pas fixe → déterminisme (`EX-NFR-002`).
- **Progression côté `HMI`** : le `GameScreen` (ou une petite abstraction de **séquence**) détient
  la **liste ordonnée** des niveaux et l'**indice courant** ; la réussite avance l'indice et
  **recharge la scène** (monde + personnage) pour le niveau suivant, ou revient au menu après le
  dernier. La simulation `Core` reste ignorante de la progression (un niveau à la fois).
- **Contenu progressif** : niveau 1 sans saut (prise en main), niveau 2 avec saut requis. La
  séquence est **extensible** (« niveau XX ») sans changer le code — source à trancher à
  l'implémentation (liste explicite ou balayage ordonné du dossier des niveaux).

## Exigences couvertes
- `EX-GP-011` (saut), `EX-GP-013` (au sol uniquement, pas de double saut).
- `EX-CTRL-011` (fronts), `EX-CTRL-010` (action logique).
- `EX-LVL-010` (ordre défini), `EX-LVL-011` (enchaînement automatique, retour menu après le
  dernier). `EX-LVL-012` partiellement (déplacement/saut + danger ; le niveau puzzle viendra avec
  les mécanismes).
- `EX-NFR-002` (pas fixe déterministe), `EX-NFR-010`/`EX-NFR-020` (testabilité, tests),
  `EX-ARCH-011` (composants = données pures).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-donnees-saut.md) | Données du saut : `PlayerInput`, `Player`, `PhysicsConfig` | `Core` | ✅ |
| [TACHE-02](tache-02-mapping-saut.md) | Mapping du saut (`Espace`/`W` → intention) | `HMI/Input` | ✅ |
| [TACHE-03](tache-03-saut-hauteur-variable.md) | Saut au sol + hauteur variable | `Core/Ecs/Systems` | ✅ |
| [TACHE-04](tache-04-coyote-buffering.md) | Coyote time + jump buffering | `Core/Ecs/Systems` | ✅ |
| [TACHE-05](tache-05-enchainement-niveaux.md) | Enchaînement de niveaux (séquence, auto-avance, retour titre) | `HMI/Interface` | ✅ |
| [TACHE-06](tache-06-niveaux-demo.md) | Niveaux de démo (séquence, dont saut requis) + preuve | `Elements/Levels` | ✅ |

## Critères d'acceptation du lot
1. Le personnage **saute** à l'appui, uniquement au sol (ou en coyote time), retombe sous gravité
   constante ; la **hauteur varie** selon la durée d'appui. **Coyote** et **buffering** fonctionnent.
2. À la **réussite** d'un niveau, le **suivant** se charge automatiquement (personnage à l'entrée du
   nouveau niveau) ; après le **dernier**, retour à l'**écran titre**.
3. Un niveau de démo **exige le saut** (obstacle infranchissable sans sauter) ; des tests prouvent
   la **franchissabilité avec saut** et l'**enchaînement** (niveau 1 → niveau 2 → fin).
4. La logique (saut, hauteur variable, coyote, buffer) est **couverte par des tests** (`ctest`
   vert), déterministe au pas fixe.
5. Build `/W4 /WX` sans avertissement, documentation Doxygen et `CHANGELOG.md` à jour, `lint` des
   exigences vert.

## Dépendances
- Prolonge le LOT-08 : réutilise `CharacterPhysicsSystem`, `sweepAabb`, `grounded`,
  `PlayerInput`/`Player`/`PhysicsConfig`, `toPlayerInput`, `GameScreen`, `evaluateOutcome`.
- L'`InputState` (LOT-06) fournit les fronts *pressée/relâchée* nécessaires au buffering et à la
  hauteur variable. Le `LevelLoader` (LOT-07) charge chaque niveau de la séquence.

## Navigation des tâches
- @subpage lot-09-tache-01-donnees-saut
- @subpage lot-09-tache-02-mapping-saut
- @subpage lot-09-tache-03-saut-hauteur-variable
- @subpage lot-09-tache-04-coyote-buffering
- @subpage lot-09-tache-05-enchainement-niveaux
- @subpage lot-09-tache-06-niveaux-demo
