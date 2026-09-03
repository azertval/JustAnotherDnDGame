# TACHE-05 — Wall slide : déjà livré, validation seulement {#lot-72-tache-05-wall-slide}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` (tests) · **Statut :**
fait

## Contexte
Le cadrage initial de ce lot proposait un « wall slide contrôlé » comme nouvelle mécanique.
Relecture du code avant implémentation
(`Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp::resolveVelocity`, section « 2b. Wall slide ») :
**la mécanique existe déjà**, livrée en `LOT-10` sous `EX-GP-016` — « Au contact d'un mur en l'air,
le personnage doit **glisser** le long de celui-ci (wall slide) ». `PhysicsConfig::wallSlideSpeed`
clampe déjà `velocity.value.y` quand `wallDirection != 0 && !grounded && velocity.value.y > 0`.

**Décision** : aucune exigence dupliquée n'est créée pour le wall slide. Cette tâche ne
modifie aucun code ; elle ajoute uniquement les tests croisés qui manquaient entre le wall slide
existant et les **nouvelles** mécaniques du lot (dash chargé, combo dash + saut).

## Travail à réaliser
- Aucune modification de `CharacterPhysicsSystem`/`PhysicsConfig` pour le wall slide lui-même.
- Ajouter les tests de non-interférence avec les nouvelles mécaniques (repris dans TACHE-07,
  validation croisée) : un wall slide en cours n'est pas perturbé par une charge de dash, et un
  jump-cancel (TACHE-06) au contact d'un mur juste après un wall slide déclenche bien un wall-jump.

## Fichiers impactés
- Aucun fichier de production. Tests croisés couverts par TACHE-07.

## Tests (obligatoires)
- Voir TACHE-07 : wall slide + jump-cancel en sortie de dash contre un mur.

## Points d'attention
- Ne pas réintroduire de code dupliquant `EX-GP-016` : toute future retouche du ressenti du wall
  slide passe par `PhysicsConfig::wallSlideSpeed`, déjà existant.

## Définition de fait (DoD)
- Aucune régression sur le wall slide existant (tests LOT-10 inchangés).

## Exigences
`EX-GP-016` (réutilisée, non modifiée).
