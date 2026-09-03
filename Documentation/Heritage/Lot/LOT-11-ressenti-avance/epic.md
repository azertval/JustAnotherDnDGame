# LOT-11 — Ressenti avancé : personnage humanoïde, gravité asymétrique, finitions {#lot-11}

> Statut : **terminé**. Ce lot affine le **ressenti** du personnage du LOT-10 : une **silhouette
> humanoïde** (0,4 × 0,8 tuile au lieu d'un carré 1×1), une **gravité asymétrique** (chute plus
> rapide que la montée) et deux finitions verticales — **flottement à l'apex** et **fast-fall**
> (`EX-GP-018`). La physique horizontale (`EX-GP-010`) reste inchangée.

## Objectif
Rendre le personnage plus crédible et plus agréable à contrôler, sans changer les mécaniques :

- **Personnage humanoïde** : boîte de collision et sprite en **0,4 × 0,8** (largeur × hauteur),
  centré dans la tuile d'entrée au spawn.
- **Gravité asymétrique** (`EX-GP-018`) : la **chute** est plus rapide que la montée
  (`fallGravityMultiplier`) — le grand classique qui « pèse » un saut.
- **Apex hang** : gravité réduite près du sommet (vitesse verticale faible) → contrôle flottant à
  l'apex.
- **Fast-fall** : maintenir **bas** en l'air accélère la chute (réutilise `moveY`, déjà mappé).

Toute la logique reste **pure et déterministe au pas fixe** dans `Core` (`EX-NFR-002`,
`EX-ARCH-011`), testée sans GPU. Les niveaux et leurs preuves de franchissabilité sont
**rééquilibrés** à la nouvelle taille de personnage.

## Périmètre

### Inclus
- **Spec** : `EX-GP-018` (gravité asymétrique + finitions verticales), déjà ajoutée au cadrage.
- **Données (Core)** : `PhysicsConfig` enrichi (`fallGravityMultiplier`, `apexThreshold`,
  `apexGravityMultiplier`, `fastFallMultiplier`) ; **taille du personnage** partagée
  (`Core/Physics/PlayerSpawn.h` : dimensions + placement centré).
- **Physique (Core)** : gravité **effective** dépendant de la phase (montée / chute / apex / fast-
  fall), en conservant la borne de chute et la compatibilité avec saut, wall slide et dash.
- **Personnage humanoïde (HMI)** : le `GameScreen` fait apparaître le personnage en 0,4 × 0,8
  (collision **et** sprite), centré dans la tuile d'entrée.
- **Rééquilibrage + preuves** : les niveaux `demo`/`demo2`/`demo3` sont ajustés si besoin pour la
  nouvelle taille ; les tests de franchissabilité (intégration) et le parcours **système**
  utilisent la **vraie taille** du personnage.

### Exclus (lots ultérieurs)
- **Accélération/friction horizontale** (contredirait `EX-GP-010`) ; **coup de saut variable
  horizontal**.
- **Animations** (`EX-REN-012`), squash & stretch, décalage sprite ↔ hitbox.
- **Niveau puzzle** (mécanismes), gravité asymétrique appliquée au wall jump/dash (raffinements).

## Décisions de cadrage
- **Taille 0,4 × 0,8**, **centrée** horizontalement dans la tuile, pieds près du bas ; le **sprite
  suit la boîte** (pas de décalage hitbox/visuel pour l'instant). Dimensions et placement dans un
  en-tête partagé `Core/Physics/PlayerSpawn.h` (utilisé par le jeu **et** les tests).
- **Gravité asymétrique par multiplicateurs** : gravité de base pour la montée ; ×
  `fallGravityMultiplier` en chute ; × `apexGravityMultiplier` près de l'apex (`|vy| <
  apexThreshold`) ; × `fastFallMultiplier` supplémentaire si « bas » est maintenu en chute. La
  chute reste **à gravité constante à multiplicateur près** (`EX-GP-011`, `EX-GP-018`).
- **Fast-fall via `moveY`** (déjà mappé au LOT-10 pour la visée du dash) : maintenir bas en l'air.
- **Rééquilibrage guidé par les tests** : on ajuste les niveaux jusqu'à ce que les preuves de
  franchissabilité **à la vraie taille** repassent au vert (pas de magie, mesure).

## Exigences couvertes
- `EX-GP-018` (gravité asymétrique + apex hang + fast-fall), `EX-GP-011` (retombée à gravité
  constante, précisée).
- `EX-NFR-002` (pas fixe déterministe), `EX-NFR-010`/`EX-NFR-020` (testabilité, tests),
  `EX-ARCH-011` (composants = données pures), `EX-REN-011` (sprite du personnage).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-donnees.md) | Données : réglages de *feel* + taille/placement du personnage | `Core` | ✅ |
| [TACHE-02](tache-02-gravite-asymetrique.md) | Gravité asymétrique + apex hang + fast-fall | `Core/Ecs/Systems` | ✅ |
| [TACHE-03](tache-03-personnage-humanoide.md) | Personnage humanoïde (spawn 0,4×0,8, sprite) | `HMI/Interface` | ✅ |
| [TACHE-04](tache-04-reequilibrage.md) | Rééquilibrage des niveaux + preuves à la vraie taille | `Elements/Levels` | ✅ |

## Critères d'acceptation du lot
1. Le personnage est **humanoïde** (0,4 × 0,8) en collision **et** au rendu, centré au spawn.
2. La **chute est plus rapide que la montée** ; un léger **flottement** se produit à l'apex ;
   maintenir **bas** accélère la chute.
3. Les niveaux `demo`/`demo2`/`demo3` restent **franchissables** par le personnage à sa vraie
   taille (preuves d'intégration et parcours **système** verts).
4. La logique est **couverte par des tests** (`ctest` vert), déterministe au pas fixe.
5. Build `/W4 /WX` sans avertissement, Doxygen et `CHANGELOG.md` à jour, `lint` des exigences vert.

## Dépendances
- Prolonge le LOT-10 : mêmes `CharacterPhysicsSystem`, `PhysicsConfig`, `Collider`, `GameScreen`,
  `LevelSequence` ; `moveY` (mapping du LOT-10) sert au fast-fall.

## Navigation des tâches
- @subpage lot-11-tache-01-donnees
- @subpage lot-11-tache-02-gravite-asymetrique
- @subpage lot-11-tache-03-personnage-humanoide
- @subpage lot-11-tache-04-reequilibrage
