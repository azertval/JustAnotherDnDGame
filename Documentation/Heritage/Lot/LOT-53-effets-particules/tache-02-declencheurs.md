# TACHE-02 — Déclencheurs depuis les transitions d'état du personnage {#lot-53-tache-02-declencheurs}

**Lot :** [LOT-53](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems`, `Source/HMI/Game` · **Statut :** ✅ fait

## Contexte
Un émetteur sans déclencheur ne sert à rien. Les événements à marquer sont exactement ceux que le
joueur ressent sans les voir — un dash, un atterrissage, une mort — et l'état correspondant est déjà
calculé par la physique dans `core::Player` : `dashTimer`, `grounded`, et l'issue de niveau
(`core::LevelOutcome`).

Ce sont des **transitions**, pas des états : « vient d'atterrir » se détecte en comparant au pas
précédent, comme l'atterrissage du personnage (LOT-48, TACHE-02) et les transitions de mécanismes
(LOT-47, TACHE-02). Le projet a donc déjà ce patron deux fois ; il ne doit pas être réinventé une
troisième.

## Travail à réaliser
- **Détection des transitions** au pas fixe :
  - **début de dash** — `dashTimer` passe d'inactif à actif ;
  - **atterrissage** — `grounded` passe de faux à vrai, avec une intensité fonction de la vitesse
    verticale à l'impact (une chute longue produit plus de poussière qu'un petit saut) ;
  - **mort** — `core::LevelOutcome` passe à échec ;
  - **saut mural** — optionnel, si le rendu s'y prête.
- **Émission** : chaque transition déclenche l'effet correspondant, décrit par ses constantes
  nommées (TACHE-01).
- **Traînée de dash** : cas particulier, ce n'est pas une émission ponctuelle mais une émission
  **continue pendant** la durée du dash. Le traiter explicitement.
- **Aucun champ ajouté à `core::Player`** : l'état précédent nécessaire à la détection est conservé
  hors de la simulation de gameplay, comme `hmi::PreviousPosition` le fait pour le rendu.
- **Rechargement de niveau** : après un échec, les particules sont vidées — l'effet de mort ne doit
  pas persister sur le niveau rechargé.

## Fichiers impactés
- `Source/Core/Ecs/Systems/ParticleSystem.{h,cpp}` (déclencheurs).
- `Source/HMI/Game/GameSession.{h,cpp}` (câblage, vidage au rechargement).
- `Source/Test/Unit/Core/Ecs/test_particle_triggers.cpp` (nouveau).

## Tests (obligatoires)
- Chaque transition déclenche l'effet attendu **une seule fois** (pas à chaque pas où la condition
  est vraie).
- **Intensité d'atterrissage** proportionnelle à la vitesse d'impact.
- Traînée de dash : émission pendant toute la durée du dash, arrêt à sa fin.
- Rechargement de niveau → aucune particule résiduelle.
- Les tests de gameplay existants passent **sans modification**.
- Tests `Core` purs, sans GPU.

## Points d'attention
- **Une transition, pas un état.** Émettre à chaque pas où `grounded` est vrai produirait un nuage
  de poussière permanent — c'est l'erreur la plus probable de cette tâche.
- **Ne pas ajouter de champ à `core::Player`** « pour savoir si on vient d'atterrir » : ce serait
  faire entrer la présentation dans la simulation et fausser les tests de gameplay.
- Le seuil de vitesse en dessous duquel un atterrissage ne produit **aucun** effet doit exister et
  être nommé : sinon chaque petit pas émettrait de la poussière.

## Définition de fait (DoD)
- Dash, atterrissage et mort déclenchent chacun leur effet, une seule fois, avec une intensité
  cohérente ; aucun champ ajouté à `core::Player` ; les particules sont vidées au rechargement ; les
  tests de gameplay passent inchangés ; `/W4 /WX` propre.

## Exigences
`EX-REN-008` (effets visuels) ; réutilise `EX-GP-017` (dash), `EX-GP-016` (saut mural),
`EX-GP-031` (échec au contact d'un danger), `EX-ARCH-012` (rendu sans effet sur la simulation),
`EX-NFR-002` (déterminisme).
