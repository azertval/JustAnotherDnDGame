# TACHE-02 — Transitions jouées une fois et repli sur clip manquant {#lot-47-tache-02-transitions}

**Lot :** [LOT-47](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/HMI/Game` · **Statut :** fait

## Contexte
Une porte qui passe instantanément de « fermée » à « ouverte » reste un changement d'image, pas une
ouverture. Ce sont les **transitions** qui donnent l'impression du mécanisme.

Le moteur les fournit déjà : les clips joués une fois avec clip suivant (LOT-46, TACHE-01 et
TACHE-02) sont exactement ce mécanisme. Cette tâche les branche sur les changements d'état détectés
en TACHE-01, et traite le cas où l'asset ne fournit pas le clip attendu.

## Travail à réaliser
- **Détection du changement d'état** : comparer l'état courant à celui du pas précédent pour chaque
  mécanisme suivi (`GameSession` conserve déjà les entités concernées dans ses listes de portes, de
  dangers commutés, de dangers clignotants et de blocs).
- **Déclenchement de la transition** : à un changement `fermée → ouverte`, jouer le clip
  d'ouverture, qui enchaîne de lui-même sur le clip ouvert ; symétriquement à la fermeture.
- **Absence de clip de transition** : basculer directement sur le clip d'état cible. Une porte sans
  animation d'ouverture doit fonctionner, simplement sans transition.
- **Repli sur clip d'état manquant** : conserver un rendu **lisible** — l'image fixe du clip
  disponible — et journaliser une fois l'état et le clip manquants. À la différence d'une texture
  absente (damier magenta, LOT-40), un clip absent ne justifie pas de masquer l'objet : la case a un
  rôle de gameplay et doit rester visible.
- **Cohérence au rechargement de niveau** : après un échec et un rechargement, les mécanismes
  repartent de leur clip d'état, sans rejouer de transition.

## Fichiers impactés
- `Source/HMI/Graphics/MechanismVisuals.{h,cpp}` : état `MechanismVisualState` par instance et
  `advanceMechanismVisual` (décision + progression, logique pure).
- `Source/HMI/Game/GameSession.{h,cpp}` : résout l'asset effectivement lié à chaque instance (via
  `hmi::resolveTileAppearance`, point unique, jamais dupliqué) et écrit le résultat sur sa tuile.
- `Source/HMI/Graphics/TileSkinTag.h` : nouveau champ `animatedFrame`, image courante **par
  instance**, prioritaire sur l'horloge partagée par asset de `LOT-46` TACHE-05 (qui ne peut pas,
  à elle seule, distinguer deux tuiles du même asset à des états différents).
- `Source/HMI/Graphics/TileAppearance.cpp` : `resolveTileAppearance` consulte `animatedFrame` en
  priorité, pour les sources `Skin` et `Override`.
- `Source/Test/Unit/HMI/Graphics/test_mechanism_transitions.cpp` (nouveau).

## Tests (obligatoires)
- Changement d'état → clip de transition demandé une fois, puis clip d'état.
- Aucun changement → aucun déclenchement (pas de redéclenchement à chaque pas).
- Clip de transition absent → bascule directe sur le clip d'état, sans erreur.
- Clip d'état absent → repli lisible + un seul message de log (pas un message par pas).
- Rechargement de niveau → état initial sans transition.
- Logique **pure**, testée sans GPU.

## Points d'attention
- **La transition est décorative, la collision ne l'attend pas.** La porte cesse d'être solide à
  l'instant où `MechanismController` le décide, pas à la fin de l'animation. C'est une divergence
  assumée et documentée : l'alternative — synchroniser la physique sur une durée lue dans un fichier
  d'asset — laisserait un fichier de données modifier le gameplay (`EX-ARCH-012`) et casserait le
  déterminisme.
- **Ne pas journaliser à chaque pas** : un clip manquant produirait soixante messages par seconde.
  Journaliser une fois par asset et par clip manquant.
- Le suivi de l'état précédent est une donnée de **présentation** : il ne doit pas être stocké dans
  `Core`, sur le modèle de `hmi::PreviousPosition`.

## Définition de fait (DoD)
- Les transitions d'ouverture et de fermeture sont jouées une fois et enchaînent sur l'état cible ;
  l'absence de clip dégrade proprement et ne pollue pas le journal ; la simulation est inchangée ;
  tests sans GPU verts ; `/W4 /WX` propre.

## Exigences
`EX-REN-006` (apparence pilotée par l'état, transitions comprises) ; réutilise `EX-REN-005`
(clips joués une fois), `EX-ARCH-012` (rendu sans effet sur la simulation), `EX-NFR-002`
(déterminisme), `EX-NFR-040` (repli).
