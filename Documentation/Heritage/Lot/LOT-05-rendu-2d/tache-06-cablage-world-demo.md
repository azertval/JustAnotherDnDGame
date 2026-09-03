# TACHE-06 — Câblage du World dans la boucle + scène de démo {#lot-05-tache-06-cablage-world-demo}

**Lot :** [LOT-05](epic.md) · **Emplacement :** `Source/HMI/main.cpp` · **Statut :** fait

## Contexte
Tâche d'assemblage. Aujourd'hui la boucle de `main` a un emplacement de mise à jour
**vide** (« arrive au LOT-03 ») et n'affiche rien. On y branche le `World` et le rendu,
et on construit une petite scène pour **voir** le résultat.

## Travail à réaliser
- Instancier un `core::World`, y enregistrer les systèmes de simulation (au moins le
  `MovementSystem` de démo), et appeler `world.update(fixedDelta)` pour **chaque pas fixe**
  restitué par `FixedTimestep` (remplace le `// vide`).
- Après la simulation, exécuter le **rendu découplé** : `graphics.clear(...)`, puis
  `SpriteRenderer` (TACHE-05) avec la `Camera2D`, puis `graphics.present()`.
- **Scène de démonstration** codée en dur : créer quelques entités formant une **grille de
  tuiles** (`Transform` + `Sprite` de couche fond) et **un sprite** mobile (couche entités,
  avec `Velocity` pour le voir bouger via le `MovementSystem`).
- Répercuter le redimensionnement sur la caméra et la swap chain.

## Fichiers impactés
- `Source/HMI/main.cpp`.
- Éventuel petit assembleur de scène de démo (`Source/HMI/...`), sinon inline dans `main`.

## Vérifications (obligatoires)
- L'exécutable ouvre une fenêtre affichant la grille de tuiles et le sprite ; le sprite
  **se déplace** (simulation à pas fixe active).
- Le déplacement est **déterministe** (même durée simulée → même position).
- Fermeture propre (croix / Échap) ; aucune fuite ni erreur au redimensionnement.

## Points d'attention
- **Séparation** claire : la simulation (`World::update`) précède le rendu (lecture seule) ;
  le rendu ne fait pas avancer la logique.
- Scène **codée en dur** : le chargement depuis un fichier de niveau est un lot ultérieur.
- Vérifier le rendu réel de bout en bout (lancer l'exécutable), pas seulement la compilation.

## Définition de fait (DoD)
- Chaîne complète **ECS → simulation à pas fixe → rendu** visible et déterministe ;
  tous les critères d'acceptation de l'[epic](epic.md) satisfaits ; build `/W4 /WX`, CI verte,
  Doxygen et `CHANGELOG.md` à jour.

## Exigences
`EX-REN-020`, `EX-REN-021`, `EX-REN-022`, `EX-ARCH-030`, `EX-NFR-002`.
