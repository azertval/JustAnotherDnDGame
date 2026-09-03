# TACHE-03 — Bloc descendant : contrôleur à position continue {#lot-74-tache-03-bloc-descendant}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Source/Core/Gameplay/SinkingBlockController.{h,cpp}`
· **Statut :** fait

## Contexte
`EX-GP-027` : le bloc descendant est armé par **tout** contact du personnage, puis descend à vitesse
constante en **portant** ce qui repose dessus, jusqu'à s'arrêter contre la matière pleine ou à sortir
par le bas du tableau.

Le choix structurant est fait au cadrage : ce bloc est un corps à **position continue**, jamais un
bloc de grille. Il est donc calqué sur `core::PlatformController` (`EX-GP-026`), pas sur
`core::BlockController` — et surtout, **il émet des `core::PlatformSample`**
(`Core/Physics/PlatformSample.h`, couple `previousBox`/`currentBox`).

C'est là que se joue tout le lot : en concaténant ces échantillons à la liste de plateformes du pas
(TACHE-06), le bloc descendant hérite **sans une ligne de code de collision** du portage du
personnage (`core::CharacterPhysicsSystem::applyPlatformPortage`), de la collision continue
boîte-contre-boîte (`core::sweepAabbVsAabb`, `EX-GP-005`), du portage des blocs poussables posés
dessus (`core::BlockController::update`, `EX-GP-022`) et de l'interpolation d'affichage. Une passe
de collision dédiée serait une troisième façon de résoudre le même problème dans ce moteur ; elle
n'est pas écrite.

## Travail à réaliser
- `core::SinkingBlockController`, logique **pure** (aucun rendu, aucune fenêtre), sur le modèle
  d'en-tête de `PlatformController.h` : construction depuis le `Level` (repérage des tuiles
  `SinkingBlock` en balayant la grille, comme `BlockController` le fait pour les siennes),
  `update(...)` par pas fixe, `count()`, `boxAt()`, `samples()`.
- **Armement** : à chaque pas, tester le recouvrement entre la boîte du personnage et celle de
  chaque bloc **non encore armé** — recouvrement quelconque, pas un test de portage : par le dessus,
  par le côté, par le dessous. Un bloc armé le reste ; mémoriser le **numéro de pas** de l'armement.
- **Descente** : position = `startPosition.y + speed × (pas courant − pas d'armement) / pas par
  seconde`, calculée depuis un delta de pas **entier**, jamais accumulée
  (`position += vitesse * dt` est proscrit, `EX-NFR-002`).
- **Arrêt contre la matière** : borner la position par la première case pleine sous le bloc, lue
  dans la grille de collision **déjà résolue** passée en paramètre (mécanismes puis blocs
  poussables), comme `BlockController` le fait pour ses chutes. Un bloc arrêté cesse de descendre
  définitivement — il ne repart pas si la case se libère plus tard : ce serait un quatrième
  comportement, non cadré.
- **Sortie du tableau** : un bloc dont le haut de la boîte franchit le bord bas est retiré de la
  liste des échantillons (plus aucun portage, plus aucune collision, plus aucun dessin).
- **Écrasement** : le personnage coincé entre un bloc descendant et le sol meurt, via le front
  `squished` déjà existant — même règle que la plateforme mobile écrasant contre un plafond
  (`EX-GP-026`, cadrage `LOT-63`). Vérifier à l'implémentation que ce front est bien produit par le
  chemin de portage/collision réutilisé, plutôt que d'en ajouter un second.
- `Source/Core/Gameplay/README.md` : une entrée pour le nouveau contrôleur.

## Fichiers impactés
- `Source/Core/Gameplay/SinkingBlockController.{h,cpp}` (nouveaux),
  `Source/Core/Gameplay/README.md`, `Source/Core/CMakeLists.txt`.
- `Source/Test/Unit/Core/Gameplay/test_sinking_block_controller.cpp` (6 cas).
- Câblage dans `hmi::GameSession` : voir TACHE-06.

## Tests (obligatoires)
- **Immobile avant contact** : sans personnage, le bloc ne bouge jamais, quel que soit le nombre de
  pas.
- **Armement par les trois faces** : contact par le dessus, par le côté, par le dessous — les trois
  arment ; l'armement est irréversible (s'éloigner ne l'annule pas).
- **Vitesse constante et déterminisme** : deux exécutions identiques donnent les mêmes positions au
  pas près ; la position est fonction du seul numéro de pas depuis l'armement.
- **Arrêt sur la matière** : un bloc au-dessus d'un `Solid` s'arrête pile à son contact et n'y entre
  jamais ; il ne repart pas si la case se libère ensuite.
- **Sortie du tableau** : un bloc sans rien dessous finit par disparaître des échantillons.
- **Portage** : un personnage posé dessus descend avec lui, sans décollement ni glissement cumulé ;
  un bloc poussable posé dessus est porté de même (`EX-GP-022`).
- **Écrasement mortel** : personnage entre le bloc et le sol → échec du tableau (`EX-GP-031`).

## Points d'attention
- Ne **jamais** rendre `SinkingBlock` solide dans `core::isSolid` : sa case doit rester franchissable
  autour de lui, exactement comme `MovingPlatform` et les blocs réduits. Toute sa solidité passe par
  la collision continue.
- Ne pas réimplémenter le portage : s'il faut écrire du code de portage dans ce contrôleur, c'est
  que le chemin `PlatformSample` n'a pas été emprunté et que la décision de cadrage a été perdue.
- Attention à l'ordre dans le pas : les échantillons doivent exister **avant** que la physique du
  personnage et `BlockController` ne les consomment (TACHE-06).
- La vitesse est une **constante du moteur** (`SINK_SPEED_CELLS_PER_SECOND`) et non un champ de
  niveau : décision révisée en TACHE-01, pour ne pas ajouter un vingtième paramètre à un
  constructeur dont la surface est déjà actée comme maximale. Il n'y a donc aucune valeur de
  fichier à valider ici.
- **Un bloc non armé doit émettre son échantillon** malgré son immobilité : `SinkingBlock` n'étant
  jamais solide pour la grille, sans échantillon le personnage lui passerait au travers avant même
  d'avoir pu l'armer. Le test `ImmobileTantQuIlNEstPasTouche` verrouille ce point.

## Définition de fait (DoD)
- Contrôleur livré et **testé** unitairement, portage et écrasement vérifiés en intégration,
  déterminisme démontré ; build `/W4 /WX`.

## Exigences
`EX-GP-027`, `EX-GP-005`, `EX-GP-022`, `EX-GP-026`, `EX-GP-031`, `EX-NFR-002`, `EX-ARCH-011`.
