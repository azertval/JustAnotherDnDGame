# TACHE-07 — Validation croisée et tests {#lot-72-tache-07-validation-croisee}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` (tests) · **Statut :**
fait

## Contexte
Les six mécaniques du lot (TACHE-01 à TACHE-06) composent entre elles et avec l'existant (dash
diagonal, wall-jump, budgets/charges LOT-67). Cette tâche referme le lot par les tests **croisés**
qui ne relèvent d'aucune tâche individuelle, et garantit la non-régression de tout ce qui existait
avant.

## Travail à réaliser
Ajouter aux suites d'intégration existantes (sur le modèle des tests du LOT-10/LOT-67) :
- **Dash diagonal boosté** : charge + dash en diagonale → direction et vitesse boostée correctes
  (normalisation préservée).
- **Dash boosté contre un mur** : pas de traversée, quel que soit le boost.
- **Dash boosté contre un bloc** : déclenche la poussée renforcée (TACHE-02) avec la vitesse boostée.
- **Dash sur pente boosté** : suivi de pente (TACHE-03) correct même avec un dash chargé.
- **Poussée renforcée puis saut** : héritage de momentum (TACHE-06) mesuré après une poussée
  renforcée déclenchée par un dash boosté.
- **Ground pound et wall slide indépendants** : aucune interférence entre eux et le dash/combo (ex. un
  wall slide — déjà livré, `EX-GP-016` — juste avant un wall-jump en sortie de dash).
- **Non-régression complète** : dash normal (sans charge), poussée simple en marche, saut simple,
  wall-jump simple, budgets/charges de dash (LOT-67) — tous identiques aux tests déjà existants,
  aucune valeur attendue retouchée.

## Fichiers impactés
- Tests d'intégration existants de `CharacterPhysicsSystem`/`BlockController` (dossier de tests du
  dépôt, même emplacement que les tests LOT-10/LOT-67).

## Tests (obligatoires)
Voir la liste ci-dessus — chaque scénario croisé est un test à part entière, en plus des tests déjà
listés dans TACHE-01 à TACHE-06.

## Points d'attention
- Ne pas dupliquer les tests unitaires déjà écrits dans chaque tâche : cette tâche couvre
  spécifiquement les **combinaisons** entre mécaniques, pas chaque mécanique isolément.
- Vérifier explicitement qu'aucun test préexistant (LOT-08 à LOT-67) n'a dû être retouché en valeur
  attendue — un tel retouché signalerait une régression, pas une évolution voulue.

## Définition de fait (DoD)
- `ctest` à 100 % sur l'ensemble de la suite (existante + nouvelle) ; build `/W4 /WX`.

## Exigences
`EX-GP-056`, `EX-GP-057`, `EX-GP-058`, `EX-GP-060`, `EX-GP-061`, `EX-GP-016`, `EX-GP-017`,
`EX-NFR-002`.
