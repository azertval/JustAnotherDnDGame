# LOT-19 — Physique newtonienne et plaque de pression {#lot-19}

> Statut : **terminé**. Deux évolutions liées par un même concept — le **poids** du personnage :
> une chute qui suit un modèle **newtonien** (masse, traînée, vitesse terminale émergente) plutôt
> qu'un plafond arbitraire, et un nouveau mécanisme de puzzle qui **réagit à ce poids** (plaque de
> pression, à la différence de l'interrupteur à bascule existant).

## Objectif
Deux limites identifiées dans l'état actuel :
- **La chute est plafonnée artificiellement.** `CharacterPhysicsSystem` intègre déjà la gravité en
  continu (`velocity.y += gravity × dt`), mais la borne haute (`maxFallSpeed`) est un `std::min`
  brutal : la vitesse de chute grimpe linéairement puis se coupe net, sans traduire de notion de
  **masse** ni de **résistance**.
- **Les mécanismes n'ont qu'un seul comportement.** L'interrupteur à bascule (`EX-GP-020`) est le
  seul moyen d'ouvrir une porte : aucun mécanisme ne réagit à une présence **continue**, ce qui
  limite les puzzles possibles (pas de « rester dessus pour garder la porte ouverte », pas de
  course contre la montre une fois la plaque relâchée).

## Périmètre

### Inclus
- **Masse du personnage** (`core::Player::mass`) et **modèle de chute newtonien** : force nette de
  chute = poids (masse × gravité effective) **moins** une traînée proportionnelle à la vitesse ;
  la vitesse terminale **émerge** de cet équilibre (poids = traînée) au lieu d'être plafonnée —
  une masse plus grande tombe plus vite (traînée relativement plus faible). La **montée** du saut
  (impulsion, gravité simple) reste **inchangée** : seule la chute est concernée.
- **Plaque de pression** (`TileType::PressurePlate`, nouveau) : ouvre la porte liée tant qu'un
  poids suffisant y repose, la referme dès qu'il en repart — activation **continue**, à la
  différence de l'interrupteur à bascule (conservé tel quel, comportement inchangé). Réutilise
  l'infrastructure de liaison existante (identifiant/`opensWith`, éditeur Maj+clic).
- Intégration éditeur (palette, rendu) et un niveau de démonstration exploitant la plaque.

### Exclus (hors périmètre de ce lot)
- **Blocs poussables** (`EX-GP-022`) : la plaque de pression pose les bases (poids, présence
  continue) mais aucune entité poussable n'est ajoutée — avec un seul acteur possible (le
  personnage) aujourd'hui, le seuil de poids de la plaque est vérifié mais ne **distingue** encore
  rien ; il devient réellement discriminant le jour où une seconde masse existe. Lot naturel à
  venir, non entamé ici.
- **Traînée à la montée du saut** (impulsions de saut/wall jump/dash divisées par la masse) : le
  ressenti de saut a déjà été réglé et testé en LOT-11 ; le retoucher pour une demande centrée sur
  la **chute** serait un risque de régression hors sujet.
- **Poids par tuile configurable** (ex. une plaque qui exige plus qu'un personnage seul) :
  `TileMap` ne porte qu'un `TileType` par case, aucune métadonnée numérique — un seuil **global**
  (constante) est utilisé à la place ; le rendre configurable par plaque est un changement de
  modèle de données plus large, hors périmètre.

## Décisions de cadrage
- **La traînée ne s'applique qu'en chute (`velocity.y ≥ 0`), jamais à la montée.** Séparer les deux
  intégrations (chute newtonienne / montée gravité simple, inchangée) garantit que le saut garde
  exactement le ressenti déjà validé en LOT-11 — aucune re-calibration de la hauteur/durée de saut.
- **Les multiplicateurs de ressenti existants (chute renforcée, fast-fall, flottement à l'apex,
  `EX-GP-018`) restent la première étape du calcul** : ils pondèrent la gravité **effective**, qui
  sert ensuite d'entrée au modèle newtonien (poids effectif − traînée). Aucun de ces réglages n'est
  supprimé ; le modèle newtonien remplace uniquement le **plafond** qui les suivait.
- **Le coefficient de traînée est calibré pour retomber sur l'ancienne vitesse terminale (25
  unités/s)** à masse par défaut (1,0) : la courbe change (asymptotique au lieu d'un coude net),
  la vitesse terminale **atteinte**, elle, ne change pas — pas de régression de ressenti perçue sur
  les niveaux existants, seulement une trajectoire plus naturelle en cours de chute.
- **Seul le personnage compare son poids à la plaque pour l'instant** (constante `MIN_TRIGGER_MASS`
  dans `Core/Gameplay`, indépendante de `PhysicsConfig` pour ne pas coupler les deux modules) :
  calée par défaut sur la masse par défaut du personnage, pour qu'une plaque fonctionne « prête à
  l'emploi » comme un interrupteur classique tant qu'aucune autre masse n'existe dans le jeu.
- **L'interrupteur à bascule n'est pas concerné par le poids** : sémantiquement, c'est un
  bouton/levier (contact suffit), pas un capteur de poids — seule la **nouvelle** plaque introduit
  cette vérification, pour ne rien changer au comportement ni aux niveaux existants utilisant des
  interrupteurs.

## Exigences couvertes
- Nouvelle : `EX-GP-019` (masse et chute newtonienne).
- Nouvelle : `EX-GP-025` (plaque de pression, activation continue).
- Réutilisées (inchangées) : `EX-GP-011`/`EX-GP-012` (saut, gravité continue), `EX-GP-018`
  (ressenti vertical), `EX-GP-020`/`EX-GP-021` (interrupteur/porte).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-masse-chute-newtonienne.md) | Masse et chute newtonienne | `Core/Physics`, `Core/Ecs` | ✅ |
| [TACHE-02](tache-02-plaque-de-pression.md) | Plaque de pression | `Core/Levels`, `Core/Gameplay` | ✅ |
| [TACHE-03](tache-03-editeur-niveau-demo.md) | Intégration éditeur et niveau de démonstration | `HMI/Editor`, `HMI/Graphics`, `Source/Elements` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. La vitesse de chute part de 0 (ou de la vitesse courante) et s'approche **progressivement**
   d'une vitesse terminale, sans coude net perceptible dans la courbe (vérifié par test :
   accélération décroissante à mesure que la vitesse approche le régime permanent).
2. Une masse plus grande produit une vitesse terminale plus élevée, à traînée égale (vérifié par
   test).
3. Le saut (hauteur, durée, apex) est **inchangé** par rapport à avant ce lot (non-régression,
   tests existants toujours verts).
4. Une plaque de pression maintient la porte ouverte **tant que** le personnage y reste, et la
   referme **dès qu'il en part** — sans front de bascule (contrairement à l'interrupteur).
5. L'éditeur permet de peindre une plaque de pression et de la lier à une porte, avec le même
   geste (Maj+clic) que pour un interrupteur.
6. Un niveau de démonstration illustre la plaque de pression, intégré à la séquence de jeu.
7. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- Étend `CharacterPhysicsSystem`/`PhysicsConfig` (LOT-09/11), `MechanismController` (LOT-12),
  le pipeline de niveaux (`TileType`, `LevelLoader`/`LevelWriter`/`LevelDraft`, LOT-06/07/14) et
  l'éditeur (`TilePalette`, `EditorScreen`, LOT-14/15).

## Navigation des tâches
- @subpage lot-19-tache-01-masse-chute-newtonienne
- @subpage lot-19-tache-02-plaque-de-pression
- @subpage lot-19-tache-03-editeur-niveau-demo
- @subpage lot-19-tache-04-documentation-verification
