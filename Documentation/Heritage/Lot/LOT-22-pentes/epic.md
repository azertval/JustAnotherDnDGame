# LOT-22 — Pentes réelles {#lot-22}

> Statut : **terminé**. Le personnage **suit** la surface d'une tuile en pente en marchant dessus
> (`EX-GP-003`), plutôt que d'être simplement bloqué ou arrêté à son pied — première brique d'une
> collision **par forme de tuile**, pas seulement solide/vide.

## Objectif
Toutes les tuiles solides du moteur sont aujourd'hui des **rectangles pleins** (`isSolid`,
binaire). `EX-GP-003` demande une tuile de **pente** : solide, mais dont la surface est
**inclinée** — le personnage doit y marcher en suivant la montée/descente, sans à-coup ni saut
brusque en haut ou en bas de la pente. C'est le changement le plus risqué proposé pour la forme
des blocs (voir la discussion de cadrage tenue avec l'utilisateur) : il touche directement
`SweptCollision`/`CharacterPhysicsSystem`, cœur déjà testé et stable de la physique. Ce lot est
traité **seul**, avant `LOT-23`/`LOT-24`, pour isoler ce risque.

## Périmètre

### Inclus
- Deux nouvelles tuiles : `SlopeUpRight` (monte de gauche à droite) et `SlopeUpLeft` (monte de
  droite à gauche) — pente à **45°**, montée pleine sur toute la largeur d'une case (une case de
  différence de hauteur entre les deux bords).
- **Suivi de pente** : un personnage qui marche sur une pente voit sa position verticale **calée**
  sur la hauteur de la pente à sa position horizontale, chaque pas fixe — pas seulement bloqué par
  le bord haut ou en chute libre au-dessus du bord bas.
- Chargement/écriture JSON (`"slopeUpRight"`/`"slopeUpLeft"`), palette de l'éditeur, rendu (une
  couleur distincte, triangle plutôt que carré plein).

### Exclus (hors périmètre de ce lot)
- **Angle autre que 45°** — une pente à pente variable (ex. 30°, sur deux cases) est une évolution
  possible, non demandée pour ce lot.
- **Pente en plafond** — uniquement au sol (une pente « à l'envers » servant de plafond incliné
  n'est pas couverte).
- **Bloc poussable sur une pente** — comportement non défini (un bloc qui glisserait sur une pente
  est une mécanique à part) ; `BlockController` (LOT-21) continue de supposer un sol plat.
- **Collision arrondie** (`EX-GP-004`, `LOT-23`) et **blocs à taille fractionnaire** (`EX-GP-005`,
  `LOT-24`) — traités dans des lots séparés, qui **réutilisent** l'infrastructure posée ici.

## Décisions de cadrage
- **Une pente n'est PAS solide pour le balayage horizontal (`sweepX`).** Si elle l'était comme une
  tuile pleine, son bord « haut » agirait comme un mur invisible bloquant l'ascension — exactement
  l'inverse du suivi voulu. Le personnage doit pouvoir avancer horizontalement à travers la colonne
  d'une pente ; c'est la **résolution verticale** qui le maintient sur la surface.
- **Une fonction de hauteur pure par type de tuile**, pas une donnée par case. `SlopeUpRight`/
  `SlopeUpLeft` étant des pentes linéaires à 45° sur exactement une case, leur hauteur de surface à
  une position horizontale donnée se calcule directement depuis le type de tuile (aucun champ
  supplémentaire dans `TileMap` ni dans le format JSON) — cohérent avec le reste du moteur
  (`core::isSolid(TileType)` est déjà une fonction pure du type, sans état par case).
- **Nouvelle passe de résolution, après le balayage existant.** L'ordre devient : (1) balayage X/Y
  classique contre les tuiles **pleines** (murs, sols plats, plafonds — inchangé, aucune régression
  attendue sur les niveaux existants, aucune pente n'y étant présente) ; (2) **suivi de pente** — si
  le personnage chevauche horizontalement une case de pente et se trouve à une hauteur proche de sa
  surface (tolérance similaire à `kSkin`), sa position Y est calée sur cette surface et `grounded`
  est forcé à `true`. Cette étape est un **ajout**, pas une réécriture du balayage existant : le
  risque de régression sur la physique plate reste borné.
- **Le suivi de pente l'emporte sur la chute libre, mais pas sur un saut volontaire.** Une vitesse
  verticale positive (chute) proche de la surface est absorbée par le calage ; une vitesse
  verticale négative franche (le joueur vient de sauter) n'est **pas** rattrapée par la pente —
  sinon un saut depuis une pente serait immédiatement annulé.
- **Découverte pendant la TACHE-02, hors périmètre initial** : une pente suivie d'un bloc plein de
  même hauteur (le raccord pente → palier le plus courant) bloquait le personnage à mi-montée — le
  balayage horizontal (`sweepX`) traite comme un mur toute case pleine partageant une ligne que la
  boîte chevauche déjà, or suivre une pente fait légitimement chevaucher la boîte à l'intérieur de
  sa case (pas seulement à sa frontière, comme sur un sol plat). Corrigé en généralisant le principe
  de la « peau » (`kSkin`) déjà utilisée pour le sol plat : une ligne où l'empreinte horizontale
  courante de la boîte repose sur une pente n'est jamais un mur pour `sweepX` (voir
  `rowIsSlopeGround`, `SweptCollision.cpp`, et @ref guide-physique, section « Suivi de pente »).
  Détaillé dans la TACHE-02.

## Exigences couvertes
- `EX-GP-003` — implémentée (marqueur « non implémenté » retiré).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-tuile-pente.md) | Modèle de tuile et fonction de hauteur | `Core/Levels` | ✅ |
| [TACHE-02](tache-02-collision-suivi-pente.md) | Collision et suivi de pente | `Core/Physics`, `Core/Ecs/Systems` | ✅ |
| [TACHE-03](tache-03-editeur-rendu.md) | Éditeur et rendu | `HMI/Editor`, `HMI/Graphics` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Le personnage monte et descend une pente à 45° sans à-coup visible, sans se retrouver bloqué à
   la transition pente ↔ sol plat.
2. Tomber sur une pente depuis au-dessus pose le personnage sur sa surface, pas au travers.
3. Sauter depuis une pente fonctionne comme depuis un sol plat (impulsion normale, pas absorbée par
   le calage).
4. **Aucune régression** sur la physique existante : tous les tests de `test_physique_personnage.cpp`
   (sol plat, murs, sauts, dash, wall jump…) restent verts sans modification.
5. Logique nouvelle **couverte par des tests** dédiés (montée, descente, chute sur pente, saut
   depuis une pente, transition pente/sol plat). Build `/W4 /WX` sans avertissement, Doxygen et
   lint des exigences verts.

## Dépendances
- Étend `TileType`/`TileMap` (LOT-03/07) et `SweptCollision`/`CharacterPhysicsSystem` (LOT-01,
  LOT-08 à LOT-11, LOT-19) — seul lot de cette série à modifier la physique du personnage
  directement ; `LOT-23` et `LOT-24` en dépendent (réutilisent l'infrastructure de suivi de
  surface posée ici).

## Navigation des tâches
- @subpage lot-22-tache-01-modele-tuile-pente
- @subpage lot-22-tache-02-collision-suivi-pente
- @subpage lot-22-tache-03-editeur-rendu
- @subpage lot-22-tache-04-documentation-verification
