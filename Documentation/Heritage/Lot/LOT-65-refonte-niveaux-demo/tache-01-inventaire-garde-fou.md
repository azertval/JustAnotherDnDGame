# TACHE-01 — Inventaire des mécaniques et garde-fou de couverture {#lot-65-tache-01-inventaire-garde-fou}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `scripts`, `Source/Test` · **Statut :** fait

## Contexte
Le projet sait déjà se protéger d'une divergence de **séquence** : `scripts/check_demo_sequence.py`
échoue si le jeu et le test système n'énumèrent pas les mêmes tableaux. Il ne sait rien dire de la
**couverture** : rien n'empêche qu'un type de tuile livré, testé unitairement, n'apparaisse dans
aucun niveau jouable.

C'est le cas aujourd'hui pour plusieurs mécaniques ajoutées après le `LOT-25`, et cette tâche
commence par l'établir plutôt que le supposer.

Ce contrôle est livré **en premier** et **rouge** : la liste de ce qu'il refuse devient la
spécification de travail des `TACHE-02` et `TACHE-03`.

## Travail à réaliser
- **Inventaire** de ce qui doit être couvert : chaque `core::TileType` (hors `Empty`), chaque mode
  de cadrage (`EX-LVL-006`), et les variantes significatives portées par des champs — danger
  temporisé **déphasé**, danger mobile **vertical**, budget de mouvements, texture par instance,
  décor de premier plan.
- **Garde-fou automatique** : parcourir les niveaux de la séquence livrée, relever les mécaniques
  présentes, et **échouer** en nommant celles qui n'apparaissent nulle part. Sous forme de test
  (`SystemTests`, qui charge déjà les niveaux livrés) plutôt que de script Python, afin que
  l'inventaire dérive des **énumérations du code** et ne puisse pas se périmer en silence.
- **Dérivation depuis l'énumération** : parcourir `core::TileType` plutôt qu'une liste recopiée.
  Ajouter un type de tuile sans tableau qui l'emploie doit faire échouer le contrôle **sans qu'on
  ait à penser à mettre à jour l'inventaire** — c'est toute la valeur de la tâche.
- **Exclusions explicites et justifiées** : si une mécanique ne peut légitimement pas être couverte,
  elle figure dans une liste d'exclusions **nommées et commentées**, jamais omise silencieusement.
- **État initial consigné** : la liste des mécaniques non couvertes au début du lot, qui pilote la
  suite.

## Fichiers impactés
- `Source/Test/Systeme/test_couverture_mecaniques.cpp` (nouveau).
- `Source/Test/CMakeLists.txt`.
- `scripts/check_demo_sequence.py` — si la vérification de couverture y trouve mieux sa place que
  dans un test, trancher et documenter.

## Tests (obligatoires)
- Le contrôle **échoue** au départ, en nommant précisément les mécaniques non couvertes — c'est le
  résultat attendu de cette tâche, pas un problème.
- **Test négatif** : retirer d'un tableau la seule occurrence d'une mécanique fait échouer le
  contrôle.
- L'inventaire dérive de l'énumération : ajouter un `core::TileType` fictif dans un test le fait
  apparaître comme non couvert, sans modification de l'inventaire.
- Les exclusions sont **toutes** commentées ; une exclusion sans justification fait échouer une
  relecture, à défaut d'un contrôle.
- Test `Core` pur, sans GPU, s'appuyant sur les niveaux réellement livrés.

## Points d'attention
- **Ne pas recopier la liste des types.** Une liste maintenue à la main se périme au premier lot
  suivant, et le contrôle devient un ornement.
- **Couvert ≠ franchi.** Une mécanique posée dans un coin inaccessible du tableau serait « couverte »
  sans jamais être jouée. Le contrôle vérifie la présence ; c'est le test système
  (`EX-NFR-021`) qui vérifie la franchissabilité. Les deux sont nécessaires, et il faut le dire
  plutôt que de croire le premier suffisant.
- La liste d'exclusions est le point de fuite habituel de ce genre de contrôle : la garder très
  courte, et exiger une justification écrite pour chaque entrée.
- Ce test s'ajoute à `SystemTests`, qui compile `Core` seul : ne pas y introduire de dépendance
  `HMI`. En pratique, cette limite ne mord pas sur les modes de cadrage : `core::CameraFramingMode`
  vit dans `Core/Levels/CameraFraming.h` (`LOT-64`), lu depuis `core::Level::cameraFraming()` sans
  aucune dépendance `HMI` — l'inquiétude initiale de ce point d'attention ne s'est pas confirmée.

## État initial constaté
Séquence vide au moment d'écrire ce garde-fou (`TACHE-00` venait de la vider) : les **31** types de
`core::TileType` (hors `Empty`), les **3** modes de cadrage, et les **5** variantes significatives
(danger temporisé déphasé, danger mobile vertical, budget de mouvements borné, texture par
instance, décor de premier plan) apparaissent tous comme non couverts — état rouge total, attendu,
qui pilote entièrement `TACHE-02`/`TACHE-03`.

## Définition de fait (DoD)
- Un contrôle automatique dérivé des énumérations du code échoue tant qu'une mécanique livrée
  n'apparaît dans aucun niveau de la séquence, avec des exclusions nommées et justifiées ; sa
  sensibilité est démontrée par un test négatif ; l'état initial est consigné ; `/W4 /WX` propre.

## Exigences
`EX-LVL-015` (couverture exhaustive vérifiée automatiquement) ; réutilise `EX-NFR-021` (test
système), `EX-LVL-013` (séquence en donnée), `EX-NFR-020` (tests), `EX-NFR-010` (`Core` sans GPU).
